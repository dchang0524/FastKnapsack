# Coin-change and unbounded-knapsack benchmark summary

Benchmarked on 2026-10-06 on an Apple M2 running Darwin 24.1.0. All binaries
were Release builds produced by AppleClang 16. Each successful solver output
was compared byte for byte with simple dynamic programming. Times below are
median internal compute times, excluding process startup and output. Each
subprocess had an independent 600-second timeout.

Updated on 2026-10-07 with the completed randomized-k fix rerun from
2026-10-06. The `t=8u` tables below use three trials for `u=32..512` and
one trial for `u=1024`. The larger-target tables retain earlier measurements
and are explicitly labeled as results from before the fix. A new broader-input
sweep, summarized below, includes fresh long-target measurements after the fix.

The coin-change variants are:

- `deterministic`: Algorithm 4 with the explicit almost-independent space.
- `randomized-k`: Algorithm 4 with randomized k-witness recovery.
- `optimized-peeling`: Algorithm 4 with bounded peeling and the separator.
- `paper-random`: the main paper's randomized minimum-witness Algorithm 3.
- `simplified`: ordered grouped convolutions from the extreme-witness method.
- `simple-dp`: classic unbounded coin-change DP.

Unbounded knapsack has one paper-kernel implementation and simple DP. The
paper-kernel result here uses the repository's built-in quadratic max-plus
convolution backend.

## Scaling when t = 8u and n = u

### Coin change

| u | simple DP | deterministic | randomized-k | optimized peeling | paper random | simplified |
|---:|---:|---:|---:|---:|---:|---:|
| 32 | 0.007 ms | 1.35 ms | 1.19 ms | 1.24 ms | 25.03 ms | 1.80 ms |
| 64 | 0.023 ms | 3.59 ms | 3.36 ms | 3.66 ms | 123.96 ms | 5.47 ms |
| 128 | 0.092 ms | 19.16 ms | 16.51 ms | 18.38 ms | 1.484 s | 39.34 ms |
| 256 | 0.361 ms | 74.42 ms | 54.17 ms | 68.29 ms | 6.275 s | 134.93 ms |
| 512 | 1.39 ms | 232.79 ms | 164.71 ms | 233.01 ms | 15.350 s | 487.44 ms |
| 1024 | 13.99 ms | timeout | 517.94 ms | timeout | 45.626 s | 1.483 s |

Deterministic and optimized peeling each reached the 600-second cap at
`u=1024`. Corrected randomized-k completed in 517.94 ms, replacing its
previous 600-second timeout. It was 88.1 times faster than paper-random and
2.86 times faster than simplified on this single-trial case.

Randomized-k now repeats dilution chains at a scale determined by the current
unknown witness count, with no deterministic fallback. It also uses exact
enumeration when the work for requested outputs is at most 8,000,000
candidate positions. These dense coin-change inputs use that practical
shortcut, so the timings do not measure the asymptotic randomized primitive
alone. The deterministic and optimized-peeling routines retain their dense
input-product cutoff and expensive large branches.

At `u=512`, randomized-k was 93.2 times faster than paper-random in total
compute time (164.71 ms versus 15.350 s). The specific README performance
claim at `u=65536` remains untested. Paper-random varies across random runs;
the `u=1024` comparison has only one trial.

### Unbounded knapsack

| u | simple DP | paper kernel | DP / paper-kernel |
|---:|---:|---:|---:|
| 32 | 0.007 ms | 0.755 ms | 0.009x |
| 64 | 0.022 ms | 2.24 ms | 0.010x |
| 128 | 0.072 ms | 7.46 ms | 0.010x |
| 256 | 0.277 ms | 25.47 ms | 0.011x |
| 512 | 1.08 ms | 97.10 ms | 0.011x |
| 1024 | 4.35 ms | 359.43 ms | 0.012x |

With a short target interval, simple DP wins decisively for both problems.

## Amortizing the kernel over larger target ranges

The table in this section retains measurements from before the randomized-k
fix. Fresh `t=4096u` comparisons using the corrected implementation appear in
the broader-input section below; the `t=512u` and `t=2048u` cases remain historical.

The fastest Algorithm 4 time is shown for coin change. The full tables retain
all variants.

| regime | u | coin DP | best Algorithm 4 | Algorithm 4 / DP speedup | knapsack DP | paper kernel | paper-kernel / DP speedup |
|:---|---:|---:|---:|---:|---:|---:|---:|
| `t=512u` | 64 | 1.45 ms | 7.24 ms | 0.20x | 2.32 ms | 5.40 ms | 0.43x |
| `t=512u` | 128 | 5.65 ms | 26.79 ms | 0.21x | 7.58 ms | 13.12 ms | 0.58x |
| `t=512u` | 256 | 22.72 ms | 80.52 ms | 0.28x | 27.62 ms | 48.46 ms | 0.57x |
| `t=512u` | 512 | 93.33 ms | 263.13 ms | 0.35x | 95.87 ms | 119.58 ms | 0.80x |
| `t=2048u` | 128 | 32.54 ms | 54.79 ms | 0.59x | 19.72 ms | 15.35 ms | 1.28x |
| `t=2048u` | 256 | 97.68 ms | 125.68 ms | 0.78x | 75.13 ms | 50.99 ms | 1.47x |
| `t=2048u` | 512 | 355.66 ms | 381.33 ms | 0.93x | 318.30 ms | 127.00 ms | 2.51x |
| `t=4096u` | 512 | 731.12 ms | 542.32 ms | 1.35x | 687.40 ms | 184.08 ms | 3.73x |

The crossover depends strongly on `t`. On this machine and data distribution,
coin-change Algorithm 4 first beat DP between `t=2048u` and `t=4096u` at
`u=512`. Unbounded knapsack crossed earlier, between `t=512u` and `t=2048u`.

At `u=512, t=4096u`, the full coin-change ranking was:

| method | time | speed relative to DP |
|:---|---:|---:|
| randomized-k | 542.32 ms | 1.35x faster |
| optimized peeling | 543.57 ms | 1.35x faster |
| deterministic | 570.74 ms | 1.28x faster |
| simple DP | 731.12 ms | baseline |
| simplified | 795.34 ms | 0.92x |
| paper random | 18.040 s | 0.04x |

## Broader inputs: corrected implementation

The new sweep includes 308 end-to-end runs and 36 witness runs, two trials per
case, with the same 600-second timeout. All completed without timeouts or
correctness mismatches. Seven input families cover dense, random-quarter,
sparse-32, upper-half, geometric, gcd-four, and random-no-one denominations.
Short-target cases use `u=256,512,1024` and `t=8u`; all five coin-change variants
are included at `u=256`. DP won all 84 short-target median comparisons.

Fresh long-target results use `u=512, t=4096u=2,097,152`. The speedup below is
DP time divided by the kernel method's time; values above one favor the kernel.

| family | n | coin DP | randomized-k | coin speedup | knapsack DP | knapsack kernel | knapsack speedup |
|:---|---:|---:|---:|---:|---:|---:|---:|
| dense | 512 | 781.24 ms | 501.79 ms | 1.56x | 702.16 ms | 159.22 ms | 4.41x |
| random-quarter | 128 | 199.68 ms | 485.49 ms | 0.41x | 186.92 ms | 131.10 ms | 1.43x |
| sparse-32 | 32 | 49.93 ms | 432.14 ms | 0.12x | 41.56 ms | 135.78 ms | 0.31x |
| upper-half | 256 | 385.42 ms | 490.29 ms | 0.79x | 453.46 ms | 436.86 ms | 1.04x |
| geometric | 10 | 18.98 ms | 512.09 ms | 0.04x | 19.27 ms | 129.94 ms | 0.15x |
| gcd-four | 128 | 123.25 ms | 151.73 ms | 0.81x | 123.93 ms | 58.48 ms | 2.12x |
| random-no-one | 128 | 187.93 ms | 507.76 ms | 0.37x | 168.59 ms | 333.03 ms | 0.51x |

The dense-family crossover does not generalize to all these inputs. Coin-change
randomized-k beats DP only for the dense long-target family here. Knapsack's
1.04x upper-half median advantage is too small to treat as decisive from two
trials.

Branch counters recorded zero sampling calls and 10,696 requested-output
enumeration calls across the end-to-end runs. Separate witness benchmarks
therefore force actual sampling above the cutoff, with arrays of lengths
`A` and `2A`, `A=4096,8192,16384`, and `k=ceil(log2(3A-1))`. All 36 runs used
sampling and passed exact cardinality, validity, and distinctness checks.
Direct scanning was faster in every witness trial. At `A=16384`, the sampling
routine took 3.840 s for broad dense outputs versus 143.36 ms for the central
interval, a roughly 27x difference; weighted convolutions fell from a median
805 to 26. The central interval also shares identical witness sets, so this
does not isolate density scales from shared-set correlation.

See [broader_inputs_summary.md](broader_inputs_summary.md) for the concise
findings and [broader_inputs.md](broader_inputs.md) for complete tables.
These finite measurements do not establish an asymptotic improvement.

## Conclusions

- Simple DP is the best choice for short target ranges in all measured cases.
- Kernel methods become competitive only when their fixed kernel cost is
  amortized over a long target range.
- The paper-kernel knapsack implementation beats DP for long targets even
  with the built-in quadratic max-plus backend.
- Corrected randomized-k is the fastest measured kernel method for coin
  change at every dense `t=8u` size in the original rerun tables. At `u=1024` it completes in 517.94 ms;
  deterministic and optimized peeling still time out after ten minutes.
- Fresh long-target measurements confirm the corrected randomized-k dense
  crossover (1.56x at `u=512, t=4096u`), while DP wins on the other six families.
- These results establish the practical improvement through `u=1024` on the
  measured dense inputs. They do not validate the README's `u=65536` claim
  or the performance of the asymptotic randomized primitive in isolation.

## Detailed results

- [`broader_inputs_summary.md`](broader_inputs_summary.md): seven denomination
  families, fresh long-target comparisons, and actual sampling-branch findings.
- [`broader_inputs.md`](broader_inputs.md): all 344 new runs, with matching
  JSON/CSV input seeds, hashes, verification outcomes, and branch counters.
- [`rerun_scaling_t8u.md`](rerun_scaling_t8u.md): corrected implementation,
  three trials for `u=32..512`.
- [`rerun_large_branch_u1024.md`](rerun_large_branch_u1024.md): corrected
  implementation, one trial at `u=1024` with all variants.
- [`scaling_t8u.md`](scaling_t8u.md) and
  [`large_branch_u1024.md`](large_branch_u1024.md): original results before
  the fix, retained for comparison.
- [`target_heavy_t512u.md`](target_heavy_t512u.md): two target-heavy trials,
  before the fix.
- [`propagation_heavy_t2048u.md`](propagation_heavy_t2048u.md): two trials,
  before the fix.
- [`crossover_t4096u.md`](crossover_t4096u.md): two crossover trials,
  before the fix.

Each report has matching `.csv` and `.json` files containing the raw run data.
