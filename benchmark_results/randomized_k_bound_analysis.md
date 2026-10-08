# Bounds for the corrected adaptive randomized-k routine

Analysis date: 2026-10-07. This note analyzes the randomized branch in
`src/witness.cpp`, rather than its direct enumeration shortcuts. It assumes
independent unbiased random bits, unit-cost arithmetic on suitable machine
words, and an exact full convolution costing Theta(N log N). The current
fixed NTT modulus limits supported input sizes; asymptotic statements refer
to the algorithm with a scalable exact convolution primitive. The C++ PRNG
is an implementation of the randomness assumption, not a proof of it.

## Parameters and a more precise upper bound

Let A be the first array length, N the sum of both input lengths, M the
number of requested outputs with nonempty witness sets, and d_s the original
witness count for output s. Let k >= 1 (it may be capped at A without changing
the requested result). Logs below are at least one where needed.

Define the set of possible dilution levels

    H = { ceil(log2(d_s - r)) : s requested and d_s > 0,
          0 <= r < min(k, d_s) },
    D = |H|.

Here r is the number of distinct witnesses already found for that output.
Every occupied level in every chain belongs to H. For M > 0,
1 <= D <= 1 + ceil(log2 A).

The existing routine has the input-dependent expected bound

    E[T] = O(N log N + (D N log N + M k)(k + log(M + 1))).

For k = O(log N), this is

    E[T] = O(D N log^2 N).

In particular, if D is bounded by a constant, the routine takes expected
O(N log^2 N) time. For example, if all requested positive counts lie in
[d, 2d) with d >= 2k, then the unknown counts always lie in [d/2, 2d).
Consequently, they occupy at most three dyadic levels, so D <= 3.

This is a stronger bound on such inputs, rather than a uniformly better
worst-case bound.

## Upper-bound proof

At the start of a chain, an unfinished output has q = d_s - r unknown
witnesses. It is assigned level ell = ceil(log2 q). The nested independent
dilutions retain each position independently with probability p = 2^-ell.
For q >= 2, 1/(2q) <= p <= 1/q, and

    P(exactly one unknown survives)
      = q p (1-p)^(q-1) >= 1/(2e).

For q = 1, isolation occurs at level zero with probability one. If the
selected array becomes empty before ell, isolation at ell could not occur,
so the early break does not change this unconditional success probability.

Subtracting the contributions of recovered positions decodes a single
selected unknown. Every accepted candidate is independently checked for
membership in the witness set and distinctness. Occasionally a sum of more
than one unknown also decodes to a valid new witness; this can only help.
No uniform distribution over accepted witnesses is needed by Algorithm 4.

Each output is processed at only one level in a chain, and its level is
chosen before that chain's random masks are drawn. Thus the constant progress
probability holds conditional on all previous chains. Binomial domination,
a Chernoff bound, and a union bound imply that after

    R = O(k + log(M + 1) + log(1/delta))

chains every output is complete with probability at least 1-delta.
Integrating the exponential tail gives E[R] = O(k + log(M + 1)).
Different outputs may share correlated outcomes; the union bound does not
require independence between outputs.

Each chain computes at most D full weighted convolutions. Forming nested
masks and assigning outputs to levels costs O(N log(2A)); known-witness
subtraction and duplicate checking cost O(Mk). Since D >= 1 and A <= N,
the chain cost is O(D N log N + Mk). Initialization requires O(N log N).
Multiplying by the expected chain count proves the stated upper bound.

## Matching worst-case bound for the current implementation

The O(N log^3 N) bound when k = Theta(log N) cannot be improved uniformly
for the current full-convolution loop merely by sharpening this analysis.

Take A = 2^h, and let both input arrays have length A and consist entirely
of ones. Request all outputs, so the sparse-request shortcut is unavailable;
for sufficiently large A, the dense enumeration cutoff is also exceeded.
For s < A, the original witness count is d_s = s+1.

For every integer level j with ceil(log2(4k)) <= j <= h, consider the output
whose original count is

    d_j = 3 * 2^(j-2).

Such an output exists. An output can gain at most one witness per chain,
because it is placed in exactly one level bucket. Therefore each of these
outputs remains unfinished at the start of each of the first k chains.
During those chains, its unknown count satisfies

    2^(j-1) < d_j - r <= 2^j,  where 0 <= r < k,

so its assigned level remains j throughout those first k chains.

In each fresh chain, the probability that the sampled first array is
nonempty at level j is

    1 - (1 - 2^-j)^A >= 1 - exp(-A / 2^j) >= 1 - 1/e.

On that event, every earlier nested mask is nonempty, so the loop reaches
level j and performs a full weighted convolution there. Linearity of
expectation yields

    E[number of convolutions]
      >= (1 - 1/e) k (h - ceil(log2(4k)) + 1)
      = Omega(k log(A/k))

in the regime k = Theta(log A). The expression with the explicit level
count is the useful bound when the range is nonempty; this note does not
claim the simplified expression for every k close to A.

When k = Theta(log A), this forces Omega(log^2 A) full convolutions.
Because N = 2A and each full transform costs Theta(N log N), the expected
runtime is Omega(N log^3 N). Together with the upper bound, this gives

    E[T] = Theta(N log^3 N)

on this input family. This lower bound concerns the current implementation,
not every possible randomized witness algorithm. A different algorithm
could batch recovery, reuse transform work, or avoid full convolutions.

## Comparison and practical interpretation

Theorem 12 of Finding Witnesses by Peeling gives an expected bound for the
general reconstruction problem of

    O(F [k(log A + log k log log A) + log M log A] + Mk log M),

where F is the intersection-query cost. Setting F = O(N log N) and
k = Theta(log N) gives O(N log^3 N), matching our worst-case bound.
[Finding Witnesses by Peeling, Theorem 12](https://www.cs.bgu.ac.il/~dekelts/publications/peeling.pdf).

The constant-D result above may describe favorable inputs more accurately,
but it does not establish a new bound against the strongest possible
input-dependent analysis of prior algorithms. No novelty claim is made.

The recorded randomized-k coin-change benchmarks use direct enumeration
when the candidate work for requested outputs is small. Their speedup is
not experimental evidence for the asymptotic randomized-branch bound.

Algorithm 4 additionally invokes the witness primitive over several
ordering rounds and convolution rows. A favorable density spectrum must
hold in those calls to transfer the constant-D improvement to its total
runtime; this has not been established for all coin-change inputs.
[Main paper, Section 5.3](https://arxiv.org/pdf/2202.13484).
