# Broader input benchmark

Updated: 2026-10-07T23:34:19-04:00

Machine: arm. Release build, serial processes, 2 trials per case, independent 600-second timeout.

`u` is the largest weight, `n` the number of distinct items, and `t` the largest target solved (all targets 0 through t are output).
Times below are median internal compute times, excluding process startup and output. Coin-change inputs use shuffled item order; knapsack profits are independent uniform integers in [1,4u].
Every completed optimized solve is compared byte for byte with simple DP; witness recovery is checked for exact cardinality, validity, and distinctness using an independent scan.

Input seed is recorded in JSON/CSV, along with the SHA-256 hash of each solver input. Sampling uses the production random_device seed, so its exact timings are not reproducible from the input seed.

## Coverage

- Dense: all weights 1..u. Random-quarter: u/4 weights including 1 and u. Sparse-32: 32 weights including 1 and u.
- Upper-half: every weight above u/2; no small weights. Geometric: powers of two plus u.
- Gcd-four: every multiple of four; three quarters of targets are unreachable. Random-no-one: u/4 random weights with u included, excluding 1.
- Short targets: all seven families at u=256,512,1024 and t=8u, randomized-k/simplified/DP plus knapsack.
- All five coin-change variants: all seven families at u=256 and t=8u. Larger deterministic/optimized-peeling cases are omitted because their large branches previously timed out; this is not a full all-variant scaling study.
- Long targets: all seven families at u=512 and t=4096u, randomized-k/DP plus knapsack. This tests whether the dense-input crossover generalizes.

## End-to-end comparisons

DP / method greater than 1 means the method wins. Status includes missing or failed trials; a median of completed trials does not imply all trials completed.

| problem | family | u | n | t/u | method | time | DP / method | sampling calls | enumeration calls | status |
|:--|:--|--:|--:|--:|:--|--:|--:|--:|--:|:--|
| coinchange | dense | 256 | 256 | 8 | simple-dp | 354 µs | 1.000× | — | — | ok (2/2) |
| coinchange | dense | 256 | 256 | 8 | deterministic | 70.34 ms | 0.005× | 0.0 | 0.0 | ok (2/2) |
| coinchange | dense | 256 | 256 | 8 | randomized-k | 57.17 ms | 0.006× | 0.0 | 153.0 | ok (2/2) |
| coinchange | dense | 256 | 256 | 8 | optimized-peeling | 120.76 ms | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | dense | 256 | 256 | 8 | paper-random | 6.351 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | dense | 256 | 256 | 8 | simplified | 136.80 ms | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | dense | 512 | 512 | 8 | simple-dp | 1.33 ms | 1.000× | — | — | ok (2/2) |
| coinchange | dense | 512 | 512 | 8 | randomized-k | 165.87 ms | 0.008× | 0.0 | 190.0 | ok (2/2) |
| coinchange | dense | 512 | 512 | 8 | simplified | 460.60 ms | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | dense | 512 | 512 | 4096 | simple-dp | 781.24 ms | 1.000× | — | — | ok (2/2) |
| coinchange | dense | 512 | 512 | 4096 | randomized-k | 501.79 ms | 1.557× | 0.0 | 190.0 | ok (2/2) |
| coinchange | dense | 1024 | 1024 | 8 | simple-dp | 5.07 ms | 1.000× | — | — | ok (2/2) |
| coinchange | dense | 1024 | 1024 | 8 | randomized-k | 570.68 ms | 0.009× | 0.0 | 231.0 | ok (2/2) |
| coinchange | dense | 1024 | 1024 | 8 | simplified | 1.510 s | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | simple-dp | 63 µs | 1.000× | — | — | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | deterministic | 35.84 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | randomized-k | 29.22 ms | 0.002× | 0.0 | 153.0 | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | optimized-peeling | 35.17 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | paper-random | 1.456 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 256 | 64 | 8 | simplified | 74.39 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 512 | 128 | 8 | simple-dp | 224 µs | 1.000× | — | — | ok (2/2) |
| coinchange | gcd-four | 512 | 128 | 8 | randomized-k | 81.95 ms | 0.003× | 0.0 | 190.0 | ok (2/2) |
| coinchange | gcd-four | 512 | 128 | 8 | simplified | 284.86 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | gcd-four | 512 | 128 | 4096 | simple-dp | 123.25 ms | 1.000× | — | — | ok (2/2) |
| coinchange | gcd-four | 512 | 128 | 4096 | randomized-k | 151.73 ms | 0.812× | 0.0 | 190.0 | ok (2/2) |
| coinchange | gcd-four | 1024 | 256 | 8 | simple-dp | 912 µs | 1.000× | — | — | ok (2/2) |
| coinchange | gcd-four | 1024 | 256 | 8 | randomized-k | 231.41 ms | 0.004× | 0.0 | 231.0 | ok (2/2) |
| coinchange | gcd-four | 1024 | 256 | 8 | simplified | 833.35 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | simple-dp | 16 µs | 1.000× | — | — | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | deterministic | 30.12 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | randomized-k | 36.05 ms | 0.000× | 0.0 | 153.0 | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | optimized-peeling | 31.00 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | paper-random | 3.615 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 256 | 9 | 8 | simplified | 43.42 ms | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 512 | 10 | 8 | simple-dp | 37 µs | 1.000× | — | — | ok (2/2) |
| coinchange | geometric | 512 | 10 | 8 | randomized-k | 104.35 ms | 0.000× | 0.0 | 190.0 | ok (2/2) |
| coinchange | geometric | 512 | 10 | 8 | simplified | 93.74 ms | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | geometric | 512 | 10 | 4096 | simple-dp | 18.98 ms | 1.000× | — | — | ok (2/2) |
| coinchange | geometric | 512 | 10 | 4096 | randomized-k | 512.09 ms | 0.037× | 0.0 | 190.0 | ok (2/2) |
| coinchange | geometric | 1024 | 11 | 8 | simple-dp | 67 µs | 1.000× | — | — | ok (2/2) |
| coinchange | geometric | 1024 | 11 | 8 | randomized-k | 319.67 ms | 0.000× | 0.0 | 231.0 | ok (2/2) |
| coinchange | geometric | 1024 | 11 | 8 | simplified | 238.78 ms | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | simple-dp | 83 µs | 1.000× | — | — | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | deterministic | 53.28 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | randomized-k | 52.14 ms | 0.002× | 0.0 | 153.0 | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | optimized-peeling | 53.01 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | paper-random | 7.741 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 256 | 64 | 8 | simplified | 77.24 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 512 | 128 | 8 | simple-dp | 314 µs | 1.000× | — | — | ok (2/2) |
| coinchange | random-no-one | 512 | 128 | 8 | randomized-k | 178.41 ms | 0.002× | 0.0 | 190.0 | ok (2/2) |
| coinchange | random-no-one | 512 | 128 | 8 | simplified | 258.87 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-no-one | 512 | 128 | 4096 | simple-dp | 187.93 ms | 1.000× | — | — | ok (2/2) |
| coinchange | random-no-one | 512 | 128 | 4096 | randomized-k | 507.76 ms | 0.370× | 0.0 | 190.0 | ok (2/2) |
| coinchange | random-no-one | 1024 | 256 | 8 | simple-dp | 1.23 ms | 1.000× | — | — | ok (2/2) |
| coinchange | random-no-one | 1024 | 256 | 8 | randomized-k | 568.04 ms | 0.002× | 0.0 | 231.0 | ok (2/2) |
| coinchange | random-no-one | 1024 | 256 | 8 | simplified | 827.02 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | simple-dp | 91 µs | 1.000× | — | — | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | deterministic | 52.17 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | randomized-k | 51.70 ms | 0.002× | 0.0 | 153.0 | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | optimized-peeling | 52.69 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | paper-random | 7.632 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 256 | 64 | 8 | simplified | 77.83 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 512 | 128 | 8 | simple-dp | 326 µs | 1.000× | — | — | ok (2/2) |
| coinchange | random-quarter | 512 | 128 | 8 | randomized-k | 175.15 ms | 0.002× | 0.0 | 190.0 | ok (2/2) |
| coinchange | random-quarter | 512 | 128 | 8 | simplified | 249.68 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | random-quarter | 512 | 128 | 4096 | simple-dp | 199.68 ms | 1.000× | — | — | ok (2/2) |
| coinchange | random-quarter | 512 | 128 | 4096 | randomized-k | 485.49 ms | 0.411× | 0.0 | 190.0 | ok (2/2) |
| coinchange | random-quarter | 1024 | 256 | 8 | simple-dp | 1.50 ms | 1.000× | — | — | ok (2/2) |
| coinchange | random-quarter | 1024 | 256 | 8 | randomized-k | 636.00 ms | 0.002× | 0.0 | 231.0 | ok (2/2) |
| coinchange | random-quarter | 1024 | 256 | 8 | simplified | 813.97 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | simple-dp | 44 µs | 1.000× | — | — | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | deterministic | 37.39 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | randomized-k | 39.52 ms | 0.001× | 0.0 | 153.0 | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | optimized-peeling | 38.17 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | paper-random | 3.907 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 256 | 32 | 8 | simplified | 60.27 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 512 | 32 | 8 | simple-dp | 96 µs | 1.000× | — | — | ok (2/2) |
| coinchange | sparse-32 | 512 | 32 | 8 | randomized-k | 123.11 ms | 0.001× | 0.0 | 190.0 | ok (2/2) |
| coinchange | sparse-32 | 512 | 32 | 8 | simplified | 152.83 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | sparse-32 | 512 | 32 | 4096 | simple-dp | 49.93 ms | 1.000× | — | — | ok (2/2) |
| coinchange | sparse-32 | 512 | 32 | 4096 | randomized-k | 432.14 ms | 0.116× | 0.0 | 190.0 | ok (2/2) |
| coinchange | sparse-32 | 1024 | 32 | 8 | simple-dp | 180 µs | 1.000× | — | — | ok (2/2) |
| coinchange | sparse-32 | 1024 | 32 | 8 | randomized-k | 376.74 ms | 0.000× | 0.0 | 231.0 | ok (2/2) |
| coinchange | sparse-32 | 1024 | 32 | 8 | simplified | 356.54 ms | 0.001× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | simple-dp | 174 µs | 1.000× | — | — | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | deterministic | 64.07 ms | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | randomized-k | 58.51 ms | 0.003× | 0.0 | 153.0 | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | optimized-peeling | 64.25 ms | 0.003× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | paper-random | 1.835 s | 0.000× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 256 | 128 | 8 | simplified | 96.84 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 512 | 256 | 8 | simple-dp | 856 µs | 1.000× | — | — | ok (2/2) |
| coinchange | upper-half | 512 | 256 | 8 | randomized-k | 225.36 ms | 0.004× | 0.0 | 190.0 | ok (2/2) |
| coinchange | upper-half | 512 | 256 | 8 | simplified | 348.89 ms | 0.002× | 0.0 | 0.0 | ok (2/2) |
| coinchange | upper-half | 512 | 256 | 4096 | simple-dp | 385.42 ms | 1.000× | — | — | ok (2/2) |
| coinchange | upper-half | 512 | 256 | 4096 | randomized-k | 490.29 ms | 0.786× | 0.0 | 190.0 | ok (2/2) |
| coinchange | upper-half | 1024 | 512 | 8 | simple-dp | 2.53 ms | 1.000× | — | — | ok (2/2) |
| coinchange | upper-half | 1024 | 512 | 8 | randomized-k | 585.04 ms | 0.004× | 0.0 | 231.0 | ok (2/2) |
| coinchange | upper-half | 1024 | 512 | 8 | simplified | 1.128 s | 0.002× | 0.0 | 0.0 | ok (2/2) |
| knapsack | dense | 256 | 256 | 8 | simple-dp | 504 µs | 1.000× | — | — | ok (2/2) |
| knapsack | dense | 256 | 256 | 8 | paper-kernel | 35.62 ms | 0.014× | — | — | ok (2/2) |
| knapsack | dense | 512 | 512 | 8 | simple-dp | 1.07 ms | 1.000× | — | — | ok (2/2) |
| knapsack | dense | 512 | 512 | 8 | paper-kernel | 91.72 ms | 0.012× | — | — | ok (2/2) |
| knapsack | dense | 512 | 512 | 4096 | simple-dp | 702.16 ms | 1.000× | — | — | ok (2/2) |
| knapsack | dense | 512 | 512 | 4096 | paper-kernel | 159.22 ms | 4.410× | — | — | ok (2/2) |
| knapsack | dense | 1024 | 1024 | 8 | simple-dp | 4.18 ms | 1.000× | — | — | ok (2/2) |
| knapsack | dense | 1024 | 1024 | 8 | paper-kernel | 380.90 ms | 0.011× | — | — | ok (2/2) |
| knapsack | gcd-four | 256 | 64 | 8 | simple-dp | 57 µs | 1.000× | — | — | ok (2/2) |
| knapsack | gcd-four | 256 | 64 | 8 | paper-kernel | 4.65 ms | 0.012× | — | — | ok (2/2) |
| knapsack | gcd-four | 512 | 128 | 8 | simple-dp | 227 µs | 1.000× | — | — | ok (2/2) |
| knapsack | gcd-four | 512 | 128 | 8 | paper-kernel | 15.00 ms | 0.015× | — | — | ok (2/2) |
| knapsack | gcd-four | 512 | 128 | 4096 | simple-dp | 123.93 ms | 1.000× | — | — | ok (2/2) |
| knapsack | gcd-four | 512 | 128 | 4096 | paper-kernel | 58.48 ms | 2.119× | — | — | ok (2/2) |
| knapsack | gcd-four | 1024 | 256 | 8 | simple-dp | 908 µs | 1.000× | — | — | ok (2/2) |
| knapsack | gcd-four | 1024 | 256 | 8 | paper-kernel | 54.63 ms | 0.017× | — | — | ok (2/2) |
| knapsack | geometric | 256 | 9 | 8 | simple-dp | 16 µs | 1.000× | — | — | ok (2/2) |
| knapsack | geometric | 256 | 9 | 8 | paper-kernel | 15.66 ms | 0.001× | — | — | ok (2/2) |
| knapsack | geometric | 512 | 10 | 8 | simple-dp | 34 µs | 1.000× | — | — | ok (2/2) |
| knapsack | geometric | 512 | 10 | 8 | paper-kernel | 46.72 ms | 0.001× | — | — | ok (2/2) |
| knapsack | geometric | 512 | 10 | 4096 | simple-dp | 19.27 ms | 1.000× | — | — | ok (2/2) |
| knapsack | geometric | 512 | 10 | 4096 | paper-kernel | 129.94 ms | 0.148× | — | — | ok (2/2) |
| knapsack | geometric | 1024 | 11 | 8 | simple-dp | 73 µs | 1.000× | — | — | ok (2/2) |
| knapsack | geometric | 1024 | 11 | 8 | paper-kernel | 154.73 ms | 0.000× | — | — | ok (2/2) |
| knapsack | random-no-one | 256 | 64 | 8 | simple-dp | 73 µs | 1.000× | — | — | ok (2/2) |
| knapsack | random-no-one | 256 | 64 | 8 | paper-kernel | 18.24 ms | 0.004× | — | — | ok (2/2) |
| knapsack | random-no-one | 512 | 128 | 8 | simple-dp | 269 µs | 1.000× | — | — | ok (2/2) |
| knapsack | random-no-one | 512 | 128 | 8 | paper-kernel | 64.78 ms | 0.004× | — | — | ok (2/2) |
| knapsack | random-no-one | 512 | 128 | 4096 | simple-dp | 168.59 ms | 1.000× | — | — | ok (2/2) |
| knapsack | random-no-one | 512 | 128 | 4096 | paper-kernel | 333.03 ms | 0.506× | — | — | ok (2/2) |
| knapsack | random-no-one | 1024 | 256 | 8 | simple-dp | 1.05 ms | 1.000× | — | — | ok (2/2) |
| knapsack | random-no-one | 1024 | 256 | 8 | paper-kernel | 244.33 ms | 0.004× | — | — | ok (2/2) |
| knapsack | random-quarter | 256 | 64 | 8 | simple-dp | 93 µs | 1.000× | — | — | ok (2/2) |
| knapsack | random-quarter | 256 | 64 | 8 | paper-kernel | 19.07 ms | 0.005× | — | — | ok (2/2) |
| knapsack | random-quarter | 512 | 128 | 8 | simple-dp | 290 µs | 1.000× | — | — | ok (2/2) |
| knapsack | random-quarter | 512 | 128 | 8 | paper-kernel | 66.04 ms | 0.004× | — | — | ok (2/2) |
| knapsack | random-quarter | 512 | 128 | 4096 | simple-dp | 186.92 ms | 1.000× | — | — | ok (2/2) |
| knapsack | random-quarter | 512 | 128 | 4096 | paper-kernel | 131.10 ms | 1.426× | — | — | ok (2/2) |
| knapsack | random-quarter | 1024 | 256 | 8 | simple-dp | 1.10 ms | 1.000× | — | — | ok (2/2) |
| knapsack | random-quarter | 1024 | 256 | 8 | paper-kernel | 235.76 ms | 0.005× | — | — | ok (2/2) |
| knapsack | sparse-32 | 256 | 32 | 8 | simple-dp | 44 µs | 1.000× | — | — | ok (2/2) |
| knapsack | sparse-32 | 256 | 32 | 8 | paper-kernel | 18.72 ms | 0.002× | — | — | ok (2/2) |
| knapsack | sparse-32 | 512 | 32 | 8 | simple-dp | 79 µs | 1.000× | — | — | ok (2/2) |
| knapsack | sparse-32 | 512 | 32 | 8 | paper-kernel | 67.37 ms | 0.001× | — | — | ok (2/2) |
| knapsack | sparse-32 | 512 | 32 | 4096 | simple-dp | 41.56 ms | 1.000× | — | — | ok (2/2) |
| knapsack | sparse-32 | 512 | 32 | 4096 | paper-kernel | 135.78 ms | 0.306× | — | — | ok (2/2) |
| knapsack | sparse-32 | 1024 | 32 | 8 | simple-dp | 181 µs | 1.000× | — | — | ok (2/2) |
| knapsack | sparse-32 | 1024 | 32 | 8 | paper-kernel | 224.43 ms | 0.001× | — | — | ok (2/2) |
| knapsack | upper-half | 256 | 128 | 8 | simple-dp | 144 µs | 1.000× | — | — | ok (2/2) |
| knapsack | upper-half | 256 | 128 | 8 | paper-kernel | 17.30 ms | 0.008× | — | — | ok (2/2) |
| knapsack | upper-half | 512 | 256 | 8 | simple-dp | 543 µs | 1.000× | — | — | ok (2/2) |
| knapsack | upper-half | 512 | 256 | 8 | paper-kernel | 63.07 ms | 0.009× | — | — | ok (2/2) |
| knapsack | upper-half | 512 | 256 | 4096 | simple-dp | 453.46 ms | 1.000× | — | — | ok (2/2) |
| knapsack | upper-half | 512 | 256 | 4096 | paper-kernel | 436.86 ms | 1.038× | — | — | ok (2/2) |
| knapsack | upper-half | 1024 | 512 | 8 | simple-dp | 2.13 ms | 1.000× | — | — | ok (2/2) |
| knapsack | upper-half | 1024 | 512 | 8 | paper-kernel | 240.15 ms | 0.009× | — | — | ok (2/2) |

## Actual randomized witness branch

Arrays have lengths A and 2A, N=3A-1 outputs, and k=ceil(log2 N). The requested work exceeds 8,000,000 and every completed run asserts that sampling was used. Direct-k scans candidate positions independently for each requested output and stops at k; it is a practical comparison, not the production pair-enumeration branch.

Dense-broad requests all outputs (many witness-count scales). Dense-narrow requests only sums A-1 through 2A-1 (each has A witnesses). Random-half and random-sparse independently retain positions with probabilities 1/2 and 0.02. Periodic uses multiples of 7 in a and 11 in b. Separated-blocks uses only the first/last eighth of each array.

| family | A | k | randomized | direct-k | direct / random | chains | weighted convolutions | status |
|:--|--:|--:|--:|--:|--:|--:|--:|:--|
| dense-broad | 4096 | 14 | 571.80 ms | 4.58 ms | 0.0080× | 63.0 | 547.5 | ok (2/2) |
| dense-broad | 8192 | 15 | 1.527 s | 4.55 ms | 0.0030× | 74.0 | 685.0 | ok (2/2) |
| dense-broad | 16384 | 16 | 3.840 s | 9.26 ms | 0.0024× | 75.5 | 805.0 | ok (2/2) |
| dense-narrow | 4096 | 14 | 32.17 ms | 702 µs | 0.0218× | 44.0 | 26.5 | ok (2/2) |
| dense-narrow | 8192 | 15 | 56.41 ms | 1.51 ms | 0.0268× | 35.0 | 20.5 | ok (2/2) |
| dense-narrow | 16384 | 16 | 143.36 ms | 2.82 ms | 0.0197× | 37.5 | 26.0 | ok (2/2) |
| periodic | 4096 | 14 | 421.55 ms | 10.81 ms | 0.0256× | 72.5 | 402.5 | ok (2/2) |
| periodic | 8192 | 15 | 1.090 s | 22.99 ms | 0.0211× | 76.5 | 506.0 | ok (2/2) |
| periodic | 16384 | 16 | 2.929 s | 49.98 ms | 0.0171× | 82.5 | 609.0 | ok (2/2) |
| random-half | 4096 | 14 | 761.67 ms | 4.95 ms | 0.0065× | 85.5 | 736.0 | ok (2/2) |
| random-half | 8192 | 15 | 1.919 s | 9.66 ms | 0.0050× | 92.5 | 860.5 | ok (2/2) |
| random-half | 16384 | 16 | 4.823 s | 25.18 ms | 0.0052× | 101.0 | 1007.0 | ok (2/2) |
| random-sparse | 4096 | 14 | 85.49 ms | 28.67 ms | 0.3353× | 26.0 | 85.5 | ok (2/2) |
| random-sparse | 8192 | 15 | 325.67 ms | 118.54 ms | 0.3640× | 40.5 | 152.0 | ok (2/2) |
| random-sparse | 16384 | 16 | 1.315 s | 496.68 ms | 0.3776× | 65.0 | 280.0 | ok (2/2) |
| separated-blocks | 4096 | 14 | 539.06 ms | 18.65 ms | 0.0346× | 74.0 | 532.0 | ok (2/2) |
| separated-blocks | 8192 | 15 | 1.335 s | 70.49 ms | 0.0528× | 75.5 | 598.0 | ok (2/2) |
| separated-blocks | 16384 | 16 | 3.523 s | 283.85 ms | 0.0806× | 78.5 | 705.5 | ok (2/2) |

## Limits

This is a finite, structured sweep, not a worst-case guarantee or a statistical survey of real workloads. Two trials give limited information about randomized tails. The witness primitive cases are synthetic and are reported separately from end-to-end Algorithm 4. Knapsack still uses the repository's quadratic max-plus convolution backend.
