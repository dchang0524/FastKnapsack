# FastKnapsack algorithm benchmark

Generated: 2026-10-06T21:49:11-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 1 trial(s), timeout 600 seconds per subprocess, target `t = 8u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 1024 | 1024 | 8192 | simple-dp | 5.17 ms | — | — | 1.00× | ok |
| 1024 | 1024 | 8192 | optimized-peeling | — | — | — | — | timeout |
| 1024 | 1024 | 8192 | paper-random | 59.611 s | 59.611 s | 473 µs | 0.00× | ok |
| 1024 | 1024 | 8192 | simplified | 1.492 s | 1.491 s | 336 µs | 0.00× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 1024 | 1024 | 8192 | simple-dp | 4.21 ms | — | — | 1.00× | ok |
| 1024 | 1024 | 8192 | paper-kernel | 397.02 ms | 393.48 ms | 1.82 ms | 0.01× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
