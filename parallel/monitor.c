#include <pthread.h>

#include "monitor.h"

#define MONITOR_THRESHOLD 8

void monitor_init(Monitor* m, int t, int d) {
    m->t = t;
    m->d = d;
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

        rearrange_n(&m->s, dst, m->work_amount);

        if (!empty(&m->s)) {
            m->signalling = true;
        } else if (m->waiting_to_give_work > 0) {
            m->signalling = true;
            ASSERT_ZERO(pthread_cond_broadcast(&m->give_work));
        }

        m->empty = true;

    } else { // The stack is not empty, just take the nodes
        // Current strategy:
        // If m->s.size > MONITOR_THRESHOLD, take 2 * m->s.size / 3 elements, else take m->s.size
        if (m->s.size > MONITOR_THRESHOLD)
            rearrange_n(&m->s, dst, (2 * m->s.size) / 3);
        else {
            rearrange_n(&m->s, dst, m->s.size);
            m->empty = true;
        }
    }

    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
}

void share_work(Monitor* m, Stack* src, int id) {
    ASSERT_ZERO(pthread_mutex_lock(&m->mutex));
    m->waiting_to_give_work++;
    while (m->signalling) {
        ASSERT_ZERO(pthread_cond_wait(&m->give_work, &m->mutex));
        m->signalling = false;
    }
    m->waiting_to_give_work--;
    // Here lies the logic of sharing work between threads.
    // Leave on the source stack (s->size / (MAX(waiting_for_work, 1) + 1)) elements.
    // Distribute the rest over waiting workers.

    int count = (src->size + 1) / 2;

    if (m->waiting_for_work == 0) {
        rearrange_n(src, &m->s, count);
        m->empty = false;
    } else {
        int workers = m->waiting_for_work + 1; // Count yourself, so +1
        int leave = (src->size + workers - 1) / workers; // Ceiling
        count = src->size - leave;

        rearrange_n(src, &m->s, count);
        m->empty = empty(&m->s);
    
        assert(leave == src->size);
    }

    m->signalling = true;
    if (m->waiting_for_work > 0) {
        update_work_amount(m);
        ASSERT_ZERO(pthread_cond_broadcast(&m->wait_for_work));
    } else if (m->waiting_to_give_work > 0) {
        ASSERT_ZERO(pthread_cond_broadcast(&m->give_work));
    } else
        m->signalling = false;

    ASSERT_ZERO(pthread_mutex_unlock(&m->mutex));
}
