# FastKnapsack algorithm benchmark

Generated: 2026-10-06T22:35:53-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 3 trial(s), timeout 600 seconds per subprocess, target `t = 8u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 32 | 32 | 256 | simple-dp | 7 µs | — | — | 1.00× | ok |
| 32 | 32 | 256 | deterministic | 1.35 ms | 1.32 ms | 10 µs | 0.01× | ok |
| 32 | 32 | 256 | randomized-k | 1.19 ms | 1.15 ms | 11 µs | 0.01× | ok |
| 32 | 32 | 256 | optimized-peeling | 1.24 ms | 1.20 ms | 10 µs | 0.01× | ok |
| 32 | 32 | 256 | paper-random | 25.03 ms | 25.00 ms | 9 µs | 0.00× | ok |
| 32 | 32 | 256 | simplified | 1.80 ms | 1.77 ms | 11 µs | 0.00× | ok |
| 64 | 64 | 512 | simple-dp | 23 µs | — | — | 1.00× | ok |
| 64 | 64 | 512 | deterministic | 3.59 ms | 3.56 ms | 14 µs | 0.01× | ok |
| 64 | 64 | 512 | randomized-k | 3.36 ms | 3.30 ms | 14 µs | 0.01× | ok |
| 64 | 64 | 512 | optimized-peeling | 3.66 ms | 3.63 ms | 15 µs | 0.01× | ok |
| 64 | 64 | 512 | paper-random | 123.96 ms | 123.91 ms | 18 µs | 0.00× | ok |
| 64 | 64 | 512 | simplified | 5.47 ms | 5.43 ms | 14 µs | 0.00× | ok |
| 128 | 128 | 1024 | simple-dp | 92 µs | — | — | 1.00× | ok |
| 128 | 128 | 1024 | deterministic | 19.16 ms | 19.10 ms | 29 µs | 0.00× | ok |
| 128 | 128 | 1024 | randomized-k | 16.51 ms | 16.44 ms | 40 µs | 0.01× | ok |
| 128 | 128 | 1024 | optimized-peeling | 18.38 ms | 18.32 ms | 28 µs | 0.00× | ok |
| 128 | 128 | 1024 | paper-random | 1.484 s | 1.484 s | 37 µs | 0.00× | ok |
| 128 | 128 | 1024 | simplified | 39.34 ms | 39.27 ms | 30 µs | 0.00× | ok |
| 256 | 256 | 2048 | simple-dp | 361 µs | — | — | 1.00× | ok |
| 256 | 256 | 2048 | deterministic | 74.42 ms | 74.35 ms | 64 µs | 0.00× | ok |
| 256 | 256 | 2048 | randomized-k | 54.17 ms | 54.10 ms | 65 µs | 0.01× | ok |
| 256 | 256 | 2048 | optimized-peeling | 68.29 ms | 68.19 ms | 64 µs | 0.01× | ok |
| 256 | 256 | 2048 | paper-random | 6.275 s | 6.275 s | 110 µs | 0.00× | ok |
| 256 | 256 | 2048 | simplified | 134.93 ms | 134.86 ms | 66 µs | 0.00× | ok |
| 512 | 512 | 4096 | simple-dp | 1.39 ms | — | — | 1.00× | ok |
| 512 | 512 | 4096 | deterministic | 232.79 ms | 232.62 ms | 119 µs | 0.01× | ok |
| 512 | 512 | 4096 | randomized-k | 164.71 ms | 164.55 ms | 128 µs | 0.01× | ok |
| 512 | 512 | 4096 | optimized-peeling | 233.01 ms | 232.85 ms | 120 µs | 0.01× | ok |
| 512 | 512 | 4096 | paper-random | 15.350 s | 15.349 s | 201 µs | 0.00× | ok |
| 512 | 512 | 4096 | simplified | 487.44 ms | 487.27 ms | 146 µs | 0.00× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 32 | 32 | 256 | simple-dp | 7 µs | — | — | 1.00× | ok |
| 32 | 32 | 256 | paper-kernel | 755 µs | 657 µs | 27 µs | 0.01× | ok |
| 64 | 64 | 512 | simple-dp | 22 µs | — | — | 1.00× | ok |
| 64 | 64 | 512 | paper-kernel | 2.24 ms | 2.01 ms | 61 µs | 0.01× | ok |
| 128 | 128 | 1024 | simple-dp | 72 µs | — | — | 1.00× | ok |
| 128 | 128 | 1024 | paper-kernel | 7.46 ms | 7.14 ms | 128 µs | 0.01× | ok |
| 256 | 256 | 2048 | simple-dp | 277 µs | — | — | 1.00× | ok |
| 256 | 256 | 2048 | paper-kernel | 25.47 ms | 24.76 ms | 270 µs | 0.01× | ok |
| 512 | 512 | 4096 | simple-dp | 1.08 ms | — | — | 1.00× | ok |
| 512 | 512 | 4096 | paper-kernel | 97.10 ms | 95.47 ms | 669 µs | 0.01× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
