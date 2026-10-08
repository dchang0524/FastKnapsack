# FastKnapsack algorithm benchmark

Generated: 2026-10-06T21:37:51-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 3 trial(s), timeout 600 seconds per subprocess, target `t = 8u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 32 | 32 | 256 | simple-dp | 7 µs | — | — | 1.00× | ok |
| 32 | 32 | 256 | deterministic | 1.21 ms | 1.18 ms | 11 µs | 0.01× | ok |
| 32 | 32 | 256 | randomized-k | 1.19 ms | 1.16 ms | 11 µs | 0.01× | ok |
| 32 | 32 | 256 | optimized-peeling | 1.24 ms | 1.21 ms | 11 µs | 0.01× | ok |
| 32 | 32 | 256 | paper-random | 52.84 ms | 52.81 ms | 10 µs | 0.00× | ok |
| 32 | 32 | 256 | simplified | 1.84 ms | 1.80 ms | 10 µs | 0.00× | ok |
| 64 | 64 | 512 | simple-dp | 24 µs | — | — | 1.00× | ok |
| 64 | 64 | 512 | deterministic | 3.72 ms | 3.69 ms | 14 µs | 0.01× | ok |
| 64 | 64 | 512 | randomized-k | 3.82 ms | 3.77 ms | 18 µs | 0.01× | ok |
| 64 | 64 | 512 | optimized-peeling | 3.77 ms | 3.74 ms | 19 µs | 0.01× | ok |
| 64 | 64 | 512 | paper-random | 213.13 ms | 213.06 ms | 22 µs | 0.00× | ok |
| 64 | 64 | 512 | simplified | 5.70 ms | 5.66 ms | 14 µs | 0.00× | ok |
| 128 | 128 | 1024 | simple-dp | 84 µs | — | — | 1.00× | ok |
| 128 | 128 | 1024 | deterministic | 17.87 ms | 17.82 ms | 28 µs | 0.00× | ok |
| 128 | 128 | 1024 | randomized-k | 17.95 ms | 17.90 ms | 28 µs | 0.00× | ok |
| 128 | 128 | 1024 | optimized-peeling | 18.31 ms | 18.24 ms | 34 µs | 0.00× | ok |
| 128 | 128 | 1024 | paper-random | 807.37 ms | 807.29 ms | 39 µs | 0.00× | ok |
| 128 | 128 | 1024 | simplified | 39.27 ms | 39.21 ms | 29 µs | 0.00× | ok |
| 256 | 256 | 2048 | simple-dp | 330 µs | — | — | 1.00× | ok |
| 256 | 256 | 2048 | deterministic | 66.88 ms | 66.81 ms | 65 µs | 0.00× | ok |
| 256 | 256 | 2048 | randomized-k | 68.24 ms | 68.16 ms | 64 µs | 0.00× | ok |
| 256 | 256 | 2048 | optimized-peeling | 66.04 ms | 65.94 ms | 64 µs | 0.00× | ok |
| 256 | 256 | 2048 | paper-random | 2.453 s | 2.453 s | 67 µs | 0.00× | ok |
| 256 | 256 | 2048 | simplified | 134.16 ms | 134.09 ms | 65 µs | 0.00× | ok |
| 512 | 512 | 4096 | simple-dp | 1.35 ms | — | — | 1.00× | ok |
| 512 | 512 | 4096 | deterministic | 228.84 ms | 228.67 ms | 128 µs | 0.01× | ok |
| 512 | 512 | 4096 | randomized-k | 232.03 ms | 231.87 ms | 126 µs | 0.01× | ok |
| 512 | 512 | 4096 | optimized-peeling | 226.96 ms | 226.81 ms | 119 µs | 0.01× | ok |
| 512 | 512 | 4096 | paper-random | 10.347 s | 10.347 s | 274 µs | 0.00× | ok |
| 512 | 512 | 4096 | simplified | 472.75 ms | 472.59 ms | 129 µs | 0.00× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 32 | 32 | 256 | simple-dp | 6 µs | — | — | 1.00× | ok |
| 32 | 32 | 256 | paper-kernel | 745 µs | 661 µs | 28 µs | 0.01× | ok |
| 64 | 64 | 512 | simple-dp | 21 µs | — | — | 1.00× | ok |
| 64 | 64 | 512 | paper-kernel | 2.80 ms | 2.40 ms | 69 µs | 0.01× | ok |
| 128 | 128 | 1024 | simple-dp | 71 µs | — | — | 1.00× | ok |
| 128 | 128 | 1024 | paper-kernel | 7.04 ms | 6.72 ms | 128 µs | 0.01× | ok |
| 256 | 256 | 2048 | simple-dp | 280 µs | — | — | 1.00× | ok |
| 256 | 256 | 2048 | paper-kernel | 24.82 ms | 24.12 ms | 271 µs | 0.01× | ok |
| 512 | 512 | 4096 | simple-dp | 1.05 ms | — | — | 1.00× | ok |
| 512 | 512 | 4096 | paper-kernel | 95.45 ms | 93.74 ms | 753 µs | 0.01× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
