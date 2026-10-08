# FastKnapsack algorithm benchmark

Generated: 2026-10-06T21:50:05-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 2 trial(s), timeout 600 seconds per subprocess, target `t = 512u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 64 | 64 | 32768 | simple-dp | 1.45 ms | — | — | 1.00× | ok |
| 64 | 64 | 32768 | deterministic | 7.56 ms | 4.30 ms | 3.20 ms | 0.19× | ok |
| 64 | 64 | 32768 | randomized-k | 7.24 ms | 3.94 ms | 3.24 ms | 0.20× | ok |
| 64 | 64 | 32768 | optimized-peeling | 7.37 ms | 4.09 ms | 3.23 ms | 0.20× | ok |
| 64 | 64 | 32768 | paper-random | 149.67 ms | 146.07 ms | 3.54 ms | 0.01× | ok |
| 64 | 64 | 32768 | simplified | 9.82 ms | 6.43 ms | 3.32 ms | 0.15× | ok |
| 128 | 128 | 65536 | simple-dp | 5.65 ms | — | — | 1.00× | ok |
| 128 | 128 | 65536 | deterministic | 27.55 ms | 18.99 ms | 8.31 ms | 0.21× | ok |
| 128 | 128 | 65536 | randomized-k | 26.79 ms | 18.44 ms | 8.16 ms | 0.21× | ok |
| 128 | 128 | 65536 | optimized-peeling | 27.64 ms | 18.96 ms | 8.45 ms | 0.20× | ok |
| 128 | 128 | 65536 | paper-random | 774.76 ms | 768.39 ms | 6.10 ms | 0.01× | ok |
| 128 | 128 | 65536 | simplified | 48.92 ms | 40.53 ms | 8.12 ms | 0.12× | ok |
| 256 | 256 | 131072 | simple-dp | 22.72 ms | — | — | 1.00× | ok |
| 256 | 256 | 131072 | deterministic | 131.54 ms | 118.45 ms | 12.58 ms | 0.17× | ok |
| 256 | 256 | 131072 | randomized-k | 80.52 ms | 67.28 ms | 12.79 ms | 0.28× | ok |
| 256 | 256 | 131072 | optimized-peeling | 82.74 ms | 69.65 ms | 12.68 ms | 0.27× | ok |
| 256 | 256 | 131072 | paper-random | 4.407 s | 4.390 s | 15.92 ms | 0.01× | ok |
| 256 | 256 | 131072 | simplified | 158.06 ms | 145.05 ms | 12.66 ms | 0.14× | ok |
| 512 | 512 | 262144 | simple-dp | 93.33 ms | — | — | 1.00× | ok |
| 512 | 512 | 262144 | deterministic | 268.32 ms | 230.75 ms | 36.59 ms | 0.35× | ok |
| 512 | 512 | 262144 | randomized-k | 263.13 ms | 226.25 ms | 36.14 ms | 0.35× | ok |
| 512 | 512 | 262144 | optimized-peeling | 265.33 ms | 227.14 ms | 37.33 ms | 0.35× | ok |
| 512 | 512 | 262144 | paper-random | 14.700 s | 14.647 s | 46.13 ms | 0.01× | ok |
| 512 | 512 | 262144 | simplified | 588.79 ms | 541.67 ms | 46.11 ms | 0.16× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 64 | 64 | 32768 | simple-dp | 2.32 ms | — | — | 1.00× | ok |
| 64 | 64 | 32768 | paper-kernel | 5.40 ms | 3.54 ms | 1.65 ms | 0.43× | ok |
| 128 | 128 | 65536 | simple-dp | 7.58 ms | — | — | 1.00× | ok |
| 128 | 128 | 65536 | paper-kernel | 13.12 ms | 9.30 ms | 3.30 ms | 0.58× | ok |
| 256 | 256 | 131072 | simple-dp | 27.62 ms | — | — | 1.00× | ok |
| 256 | 256 | 131072 | paper-kernel | 48.46 ms | 38.24 ms | 9.32 ms | 0.57× | ok |
| 512 | 512 | 262144 | simple-dp | 95.87 ms | — | — | 1.00× | ok |
| 512 | 512 | 262144 | paper-kernel | 119.58 ms | 108.55 ms | 9.24 ms | 0.80× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
