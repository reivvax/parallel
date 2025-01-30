# Computational Verification of a Combinatorial Hypothesis

For a given multiset of natural numbers $A$, we define $\sum A = \sum_{a\in A}a$. For example, if $A={1, 2, 2, 2, 10, 10}$, then $\sum A=27$. For two multisets, we write $A \supseteq B$ if each element appears in $A$ at least as many times as in $B$. For the purpose of this problem, we adopt the following definitions.

Definition. A multiset $A$ is called $d$-bounded, for a natural number $d$, if it is finite and all its elements belong to ${1, \ldots, d}$ (with any repetitions).

Definition. A pair of $d$-bounded multisets $(A,B)$ is called unambiguous if for all $A'\subseteq A$ and $B'\subseteq B$, the following holds:

$$\sum A'=\sum B'\Longleftrightarrow A'=B'=\emptyset\vee(A'=A\wedge B'=B)$$

In other words, $\sum A=\sum B$, but apart from that, the sums of any non-empty subsets of $A$ and $B$ must differ.

Problem. For a fixed $d \geq 3$ (we do not consider smaller $d$) and multisets $A_0, B_0$, we want to find unambiguous $d$-bounded multisets $A \supseteq A_0$ and $B \supseteq B_0$ that maximize the value of $\sum A$ (equivalently, $\sum B$). Let us denote this value by $\alpha(d, A_0, B_0)$. We assume $\alpha(d, A_0, B_0)=0$ if $A_0$ and $B_0$ are not $d$-bounded or do not have $d$-bounded unambiguous supersets.

Example. $\alpha(d, \emptyset, \emptyset) \geq d(d-1)$.

Proof sketch. The sets $A=\{\underbrace{d, \ldots, d}_{d-1\text{ times}}\}$ and $B=\{\underbrace{d-1,\ldots,d-1}_{d\text{ times}}\}$ satisfy the conditions with $\sum A=d(d-1)=\sum B$.

Example. $\alpha(d, \emptyset, {1}) \geq (d-1)^2$

Proof sketch. The sets $A=\{1, \underbrace{d, \ldots, d}_{d-2\text{ times}}\}$ and $B=\{\underbrace{d-1,\ldots,d-1}_{d-1\text{ times}}\}$ satisfy the conditions with $\sum A=1+d(d-2)=(d-1)^2=\sum B$.

It can be proven that the above examples are optimal, i.e., $\alpha(d, \emptyset, \emptyset) = d(d-1)$ and $\alpha(d, \emptyset, \{1\}) = (d-1)^2$.

Nevertheless, in this task, we aim to verify this computationally for the largest possible values of $d$, as well as to compute $\alpha$ values for other constrained multisets $A_0, B_0$.

# Backtracking Recursion
The values of $\alpha(d, A_0, B_0)$ can be computed recursively by incrementally constructing multisets $A \supseteq A_0$ and $B \supseteq B_0$. We define $A^{\Sigma}=\{\sum A' : A'\subseteq A\}$ which is the set of all possible sums that can be obtained from elements of $A$ (not a multiset, i.e., we do not care about the number of ways a sum can be achieved). We use the following recursion:
```
Solve(d, A, B):
    if $\sum A>\sum B$ then swap(A, B)
    $S \gets A^{\Sigma}\cap B^{\Sigma}$
    if $\sum A=\sum B$ then
        if $S=\{0, \sum A\}$ then return $\sum A$
        else return 0
    else if $S=\{0\}$ then
        return $\max_{x \in \{\text{last} A, \dots, d\} \setminus B ^ {\Sigma} } \text{Solve}(d, A\cup\{x\}, B)$
    else return 0
```
where last A denotes the last element added to $A$; if $A = A_0$, we assume last A = 1 (i.e., recursion adds elements to $A_0$ in non-decreasing order).

The project implements this computation in two versions: a non-recursive (but still single-threaded) version and a parallel (multi-threaded) version.