#include "monitor.h"
#include <assert.h>

#define MONITOR_THRESHOLD 32
#define CONST_GIVEAWAY 16

void monitor_init(Monitor* m, int t, int d) {
    m->t = t;
    m->d = d;
    stack_init(&m->s);
    m->empty = true;
    ASSERT_ZERO(pthread_mutex_init(&m->mutex, NULL));
    ASSERT_ZERO(pthread_cond_init(&m->give_work, NULL));
    ASSERT_ZERO(pthread_cond_init(&m->wait_for_work, NULL));
    m->waiting_to_give_work = m->waiting_for_work = 0;
    m->work_amount = 0;
    m->signalling = false;
    m->done = false;
}

void update_work_amount(Monitor* m) {
    m->work_amount = (m->s.size + MAX(m->waiting_for_work - 1, 0)) / MAX(m->waiting_for_work, 1); // Ceiling
}

void take_work(Monitor* m, Stack* dst, int id) {
    ASSERT_ZERO(pthread_mutex_lock(&m->mutex));
    if (empty(&m->s) && m->waiting_for_work == m->t - 1) { // Everyone is waiting, so the whole work is done
        m->signalling = true;
        m->done = true;
        LOG("%d: Monitor DONE", id);
        update_work_amount(m); // Probably unnecessary
        ASSERT_ZERO(pthread_cond_broadcast(&m->wait_for_work)); // Wake up everybody
        ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
        return;
    }

    if (empty(&m->s) || m->signalling || m->waiting_for_work > 0) { // No work to take
        m->waiting_for_work++;
        while ((empty(&m->s) || m->signalling) && !m->done) {
            LOG("%d: I WAIT", id);
            ASSERT_ZERO(pthread_cond_wait(&m->wait_for_work, &m->mutex));
            m->signalling = false;
        }
        m->waiting_for_work--;

        LOG("%d: MONITOR STACK SIZE (from sleep): %ld", id, m->s.size);
        LOG("%d: WORK AMOUNT: %d", id, m->work_amount);
        rearrange_n(&m->s, dst, m->work_amount);

        LOG("%d: DST SIZE AFTER REARRANGE: %ld", id, dst->size);
        if (!empty(&m->s)) {
            LOG("%d: HERE COMES ANOTHER SLEEPER", id);
            m->signalling = true;
        } else if (m->waiting_to_give_work > 0) {
            LOG("%d: WAKING ANOTHER GIVER FROM TAKING", id);
            m->signalling = true;
            ASSERT_ZERO(pthread_cond_broadcast(&m->give_work));
        }

        m->empty = true;

    } else { // The stack is not empty, just take the nodes
        // Current strategy:
        // If m->s.size > MONITOR_THRESHOLD, take m->s.size / 2 elements, else take m->s.size
        LOG("%d: MONITOR STACK SIZE BEFORE ( no sleep ): %ld", id, m->s.size);
        if (m->s.size > MONITOR_THRESHOLD)
            rearrange_n(&m->s, dst, 3 * m->s.size / 4);
        else {
            rearrange_n(&m->s, dst, m->s.size);
            m->empty = true;
        }

        LOG("%d: MONITOR STACK SIZE  AFTER ( no sleep ): %ld", id, m->s.size);
    }

    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
}

void share_work(Monitor* m, Stack* src, int id) {
    ASSERT_ZERO(pthread_mutex_lock(&m->mutex));
    m->waiting_to_give_work++;
    while (m->signalling) {
        LOG("%d: I CANNOT ENTER SO SOON, FIRST TAKERS", id);
        ASSERT_ZERO(pthread_cond_wait(&m->give_work, &m->mutex));
        m->signalling = false;
    }
    m->waiting_to_give_work--;
    // Here lies the logic of sharing work between threads
    // For now, 
    // if waiting_for_work = 0, then give away CONST_GIVEAWAY nodes,
    // else take the number of currently waiting threads 'waiting_for_work'
    // and leave on the stack (s->size / (MAX(waiting_for_work, 1) + 1)) elements if waiting_for_work > 0 else assume waiting_for_work = 1.
    // Distribute the rest over waiting workers
    // Other possible approaches:
    // - Always give away half of the stack
    // - Always give away constant number of nodes

    int count = CONST_GIVEAWAY;

    if (m->waiting_for_work == 0) {
        rearrange_n(src, &m->s, count);
        m->empty = false;
    } else {
        int workers = MAX(m->waiting_for_work, 1) + 1; // Count yourself, so +1
        int leave = (src->size + workers - 1) / workers; // Ceiling
        count = src->size - leave;

        rearrange_n(src, &m->s, count);
        if (!empty(&m->s))
            m->empty = false;
    
        assert(leave == src->size);
    }

    m->signalling = true;
    if (m->waiting_for_work > 0) {
        update_work_amount(m);
        LOG("%d: SHARED WORK WAKING ANOTHER TAKER, COUNT = %d, SRC_SIZE = %ld, MONITOR_STACK_SIZE = %ld, WAITING = %d, WORK_AMOUNT = %d", id, count, src->size, m->s.size, m->waiting_for_work, m->work_amount);
        ASSERT_ZERO(pthread_cond_broadcast(&m->wait_for_work));
    } else if (m->waiting_to_give_work > 0) {
        LOG("%d: WAKING ANOTHER GIVER FROM SHARE", id);
        ASSERT_ZERO(pthread_cond_broadcast(&m->give_work));
    } else
        m->signalling = false;

    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
}