# FastKnapsack algorithm benchmark

Generated: 2026-10-06T21:52:11-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 2 trial(s), timeout 600 seconds per subprocess, target `t = 4096u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 512 | 512 | 2097152 | simple-dp | 731.12 ms | — | — | 1.00× | ok |
| 512 | 512 | 2097152 | deterministic | 570.74 ms | 249.84 ms | 307.12 ms | 1.28× | ok |
| 512 | 512 | 2097152 | randomized-k | 542.32 ms | 239.37 ms | 299.95 ms | 1.35× | ok |
| 512 | 512 | 2097152 | optimized-peeling | 543.57 ms | 237.16 ms | 301.20 ms | 1.35× | ok |
| 512 | 512 | 2097152 | paper-random | 18.040 s | 17.683 s | 318.64 ms | 0.04× | ok |
| 512 | 512 | 2097152 | simplified | 795.34 ms | 485.05 ms | 305.24 ms | 0.92× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 512 | 512 | 2097152 | simple-dp | 687.40 ms | — | — | 1.00× | ok |
| 512 | 512 | 2097152 | paper-kernel | 184.08 ms | 114.98 ms | 60.47 ms | 3.73× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
