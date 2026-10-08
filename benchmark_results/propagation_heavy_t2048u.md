# FastKnapsack algorithm benchmark

Generated: 2026-10-06T21:51:12-04:00

Machine: Apple M2, Darwin 24.1.0

Configuration: Release build, median of 2 trial(s), timeout 600 seconds per subprocess, target `t = 2048u`, and `n = u`.

Every successful optimized result was compared byte for byte with simple DP.
Times are the algorithms' internal compute times and exclude process startup and output.

## Coin change

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 128 | 128 | 262144 | simple-dp | 32.54 ms | — | — | 1.00× | ok |
| 128 | 128 | 262144 | deterministic | 73.29 ms | 28.98 ms | 43.36 ms | 0.44× | ok |
| 128 | 128 | 262144 | randomized-k | 55.13 ms | 20.31 ms | 34.18 ms | 0.59× | ok |
| 128 | 128 | 262144 | optimized-peeling | 54.79 ms | 19.72 ms | 34.43 ms | 0.59× | ok |
| 128 | 128 | 262144 | paper-random | 1.631 s | 1.598 s | 29.74 ms | 0.02× | ok |
| 128 | 128 | 262144 | simplified | 78.08 ms | 43.77 ms | 33.43 ms | 0.42× | ok |
| 256 | 256 | 524288 | simple-dp | 97.68 ms | — | — | 1.00× | ok |
| 256 | 256 | 524288 | deterministic | 126.01 ms | 69.63 ms | 54.64 ms | 0.78× | ok |
| 256 | 256 | 524288 | randomized-k | 128.59 ms | 71.97 ms | 55.27 ms | 0.76× | ok |
| 256 | 256 | 524288 | optimized-peeling | 125.68 ms | 71.37 ms | 52.54 ms | 0.78× | ok |
| 256 | 256 | 524288 | paper-random | 4.039 s | 3.948 s | 85.79 ms | 0.02× | ok |
| 256 | 256 | 524288 | simplified | 193.35 ms | 141.71 ms | 50.18 ms | 0.51× | ok |
| 512 | 512 | 1048576 | simple-dp | 355.66 ms | — | — | 1.00× | ok |
| 512 | 512 | 1048576 | deterministic | 394.53 ms | 235.49 ms | 155.82 ms | 0.90× | ok |
| 512 | 512 | 1048576 | randomized-k | 381.33 ms | 231.14 ms | 147.14 ms | 0.93× | ok |
| 512 | 512 | 1048576 | optimized-peeling | 381.93 ms | 231.70 ms | 148.15 ms | 0.93× | ok |
| 512 | 512 | 1048576 | paper-random | 17.501 s | 17.314 s | 166.72 ms | 0.02× | ok |
| 512 | 512 | 1048576 | simplified | 620.54 ms | 465.56 ms | 151.70 ms | 0.57× | ok |

## Unbounded knapsack

| u | n | target | method | median time | kernel | propagation | vs DP | status |
|---:|---:|---:|:---|---:|---:|---:|---:|:---|
| 128 | 128 | 262144 | simple-dp | 19.72 ms | — | — | 1.00× | ok |
| 128 | 128 | 262144 | paper-kernel | 15.35 ms | 7.86 ms | 6.83 ms | 1.28× | ok |
| 256 | 256 | 524288 | simple-dp | 75.13 ms | — | — | 1.00× | ok |
| 256 | 256 | 524288 | paper-kernel | 50.99 ms | 27.02 ms | 22.48 ms | 1.47× | ok |
| 512 | 512 | 1048576 | simple-dp | 318.30 ms | — | — | 1.00× | ok |
| 512 | 512 | 1048576 | paper-kernel | 127.00 ms | 94.86 ms | 29.08 ms | 2.51× | ok |

## Interpretation notes

- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.
- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.
- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.
- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.
