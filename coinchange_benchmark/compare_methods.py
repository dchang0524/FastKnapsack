#!/usr/bin/env python3
"""Compare README coin-change implementations on identical verified inputs.

Examples:
    python3 coinchange_benchmark/compare_methods.py
    python3 coinchange_benchmark/compare_methods.py --sizes 65536 --coins 64 --trials 1
"""

import argparse
import random
import re
import statistics
import subprocess
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
METHODS = (
    "deterministic", "randomized-k", "optimized-peeling",
    "paper-random", "simplified", "traditional",
)


def build(build_dir):
    subprocess.run(
        ["cmake", "-S", str(ROOT), "-B", str(build_dir),
         "-DCMAKE_BUILD_TYPE=Release"], check=True,
    )
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--config", "Release",
         "--target", "coinchange_solver", "coinchange_traditional", "-j2"],
        check=True,
    )


def run(executable, method, data):
    command = [str(executable)]
    if method != "traditional":
        command.append(method)
    start = time.perf_counter()
    result = subprocess.run(command, input=data, text=True,
                            capture_output=True, check=True)
    elapsed = time.perf_counter() - start
    match = re.search(r"Kernel computation took ([\d.eE+-]+) s", result.stderr)
    kernel = float(match.group(1)) if match else None
    return result.stdout, elapsed, kernel


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sizes", type=int, nargs="+", default=[32, 64, 128])
    parser.add_argument("--coins", type=int, default=64,
                        help="maximum distinct denominations per instance")
    parser.add_argument("--target-factor", type=int, default=8)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--seed", type=int, default=2026)
    parser.add_argument("--methods", nargs="+", choices=METHODS,
                        default=list(METHODS))
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--no-build", action="store_true")
    args = parser.parse_args()
    if (min(args.sizes) < 1 or args.coins < 1 or
            args.target_factor < 1 or args.trials < 1):
        parser.error("sizes, coins, target factor, and trials must be positive")

    if not args.no_build:
        build(args.build_dir)
    binary_dir = args.build_dir / "Release" if (args.build_dir / "Release").is_dir() else args.build_dir
    solver = binary_dir / "coinchange_solver"
    traditional = binary_dir / "coinchange_traditional"
    rng = random.Random(args.seed)
    for u in args.sizes:
        n = min(args.coins, u)
        samples = {method: [] for method in args.methods}
        kernels = {method: [] for method in args.methods}
        for _ in range(args.trials):
            weights = [1] + rng.sample(range(2, u + 1), n - 1)
            data = f"{n} {u} {args.target_factor * u}\n" + "".join(
                f"{weight} -1\n" for weight in weights)
            reference, _, _ = run(traditional, "traditional", data)
            for method in args.methods:
                if method == "traditional":
                    output, elapsed, kernel = run(traditional, method, data)
                else:
                    output, elapsed, kernel = run(solver, method, data)
                if output != reference:
                    raise AssertionError(f"{method} disagreed with traditional DP at u={u}")
                samples[method].append(elapsed)
                if kernel is not None:
                    kernels[method].append(kernel)
        print(f"u={u} n={n} t={args.target_factor * u} trials={args.trials}")
        for method in args.methods:
            total = statistics.median(samples[method])
            kernel = (f" kernel={statistics.median(kernels[method]):.6f}s"
                      if kernels[method] else "")
            print(f"  {method:18s} total={total:.6f}s{kernel}")
        if "paper-random" in kernels and "randomized-k" in kernels:
            ratio = (statistics.median(kernels["paper-random"]) /
                     statistics.median(kernels["randomized-k"]))
            print(f"  paper-random/randomized-k kernel ratio={ratio:.2f}x")


if __name__ == "__main__":
    main()
