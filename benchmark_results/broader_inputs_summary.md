# Broader benchmark findings

308 end-to-end runs and 36 witness runs; statuses: {'ok': 344}.

Release builds on arm, 2 trials, 600-second limit per algorithm. Full results, exact-check outcomes, input seeds, input hashes, and branch counters: [broader_inputs.md](broader_inputs.md), [JSON](broader_inputs.json), [CSV](broader_inputs.csv).

## Short targets: coin change

u=1024, t=8u. Times are medians of internal compute times; DP / randomized-k above 1 means randomized-k wins.

| denomination family | n | simple DP | randomized-k | DP / randomized-k |
|:--|--:|--:|--:|--:|
| dense | 1024 | 5.07 ms | 570.68 ms | 0.00889× |
| random-quarter | 256 | 1.50 ms | 636.00 ms | 0.00236× |
| sparse-32 | 32 | 180 µs | 376.74 ms | 0.000479× |
| upper-half | 512 | 2.53 ms | 585.04 ms | 0.00432× |
| geometric | 11 | 67 µs | 319.67 ms | 0.000211× |
| gcd-four | 256 | 912 µs | 231.41 ms | 0.00394× |
| random-no-one | 256 | 1.23 ms | 568.04 ms | 0.00216× |

## Long targets: does the DP crossover generalize?

u=512, t=4096u=2,097,152. The speedup columns are DP time divided by kernel-method time.

| family | n | coin DP | randomized-k | coin speedup | knapsack DP | knapsack kernel | knapsack speedup |
|:--|--:|--:|--:|--:|--:|--:|--:|
| dense | 512 | 781.24 ms | 501.79 ms | 1.56× | 702.16 ms | 159.22 ms | 4.41× |
| random-quarter | 128 | 199.68 ms | 485.49 ms | 0.41× | 186.92 ms | 131.10 ms | 1.43× |
| sparse-32 | 32 | 49.93 ms | 432.14 ms | 0.12× | 41.56 ms | 135.78 ms | 0.31× |
| upper-half | 256 | 385.42 ms | 490.29 ms | 0.79× | 453.46 ms | 436.86 ms | 1.04× |
| geometric | 10 | 18.98 ms | 512.09 ms | 0.04× | 19.27 ms | 129.94 ms | 0.15× |
| gcd-four | 128 | 123.25 ms | 151.73 ms | 0.81× | 123.93 ms | 58.48 ms | 2.12× |
| random-no-one | 128 | 187.93 ms | 507.76 ms | 0.37× | 168.59 ms | 333.03 ms | 0.51× |

## Randomized sampling without enumeration shortcuts

A=16,384; arrays have lengths A and 2A; k=16. Direct-k independently scans candidate positions for each output until k valid witnesses have been found. Every randomized run asserts that it used sampling.

| witness family | randomized-k | direct-k | weighted convolutions |
|:--|--:|--:|--:|
| dense-broad | 3.840 s | 9.26 ms | 805.0 |
| dense-narrow | 143.36 ms | 2.82 ms | 26.0 |
| random-half | 4.823 s | 25.18 ms | 1007.0 |
| random-sparse | 1.315 s | 496.68 ms | 280.0 |
| periodic | 2.929 s | 49.98 ms | 609.0 |
| separated-blocks | 3.523 s | 283.85 ms | 705.5 |

## What these results establish

- DP wins every tested short-target comparison (t=8u), including all five coin-change variants at u=256.
- At u=512 and t=4096u, coin-change randomized-k beats DP only for the dense family in this sweep. Knapsack's kernel wins for dense, random-quarter, and gcd-four inputs; its 1.04× median upper-half advantage is too small to treat as decisive from two trials.
- The actual randomized witness branch is slower than direct-k scanning in every measured witness family and size. Its restricted dense case is about 27× faster than its broad dense case at A=16,384.
- End-to-end solver runs recorded 0 randomized sampling calls and 10696 requested-output enumeration calls. The separate witness benchmark establishes the performance of the actual sampling branch.
- Density, the availability of small denominations, unreachable targets, the number of denominations, and target length all affect the comparison. Long-target speedups on the dense family cannot be assumed for sparse families.
- Dense-narrow requests the central interval, where every output shares all A witnesses. Dense-broad includes edge outputs with many different witness counts. This is a controlled example of input sensitivity; it does not isolate witness-count diversity from shared-set correlation.
- These finite measurements neither prove a better asymptotic bound nor establish worst-case behavior. Two trials do not characterize random-runtime tails.
- All coin-change variants were compared at u=256. At larger sizes, the focus is randomized-k/simplified/DP and knapsack; deterministic and optimized-peeling large branches were omitted because of their earlier timeouts. The README's u=65,536 claim remains untested.
