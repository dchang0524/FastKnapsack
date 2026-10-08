#!/usr/bin/env python3
"""Benchmark every implemented coin-change and knapsack algorithm.

Each subprocess has an independent timeout. Outputs are compared byte for
byte with the corresponding simple dynamic-programming implementation before
the timing is accepted.
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import platform
import random
import re
import statistics
import subprocess
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parent
COIN_METHODS = (
    "deterministic",
    "randomized-k",
    "optimized-peeling",
    "paper-random",
    "simplified",
)
TIME_PATTERN = re.compile(
    r"^(Kernel computation|Witness propagation|Total elapsed time)(?: took|:) "
    r"([\d.eE+-]+) s$",
    re.MULTILINE,
)


def configure_and_build(build_dir: Path, timeout: float) -> None:
    subprocess.run(
        ["cmake", "-S", str(ROOT), "-B", str(build_dir),
         "-DCMAKE_BUILD_TYPE=Release"],
        check=True, timeout=timeout,
    )
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--config", "Release",
         "--target", "coinchange_solver", "coinchange_traditional",
         "knapsack_solver", "knapsack_traditional", "-j2"],
        check=True, timeout=timeout,
    )


def binary_directory(build_dir: Path) -> Path:
    release = build_dir / "Release"
    return release if release.is_dir() else build_dir


def parse_timings(stderr: str) -> dict[str, float]:
    names = {
        "Kernel computation": "kernel_seconds",
        "Witness propagation": "propagation_seconds",
        "Total elapsed time": "algorithm_seconds",
    }
    return {names[label]: float(value)
            for label, value in TIME_PATTERN.findall(stderr)}


def execute(command: list[str], data: str, timeout: float) -> dict:
    start = time.perf_counter()
    try:
        process = subprocess.run(
            command, input=data, text=True, capture_output=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return {"status": "timeout", "wall_seconds": timeout,
                "stdout": None, "stderr": None}
    wall = time.perf_counter() - start
    if process.returncode:
        return {"status": "error", "wall_seconds": wall,
                "stdout": process.stdout, "stderr": process.stderr,
                "returncode": process.returncode}
    return {"status": "ok", "wall_seconds": wall,
            "stdout": process.stdout, "stderr": process.stderr,
            **parse_timings(process.stderr)}


def make_instance(problem: str, u: int, coin_limit: int,
                  target_factor: int, seed: int) -> tuple[str, int, int]:
    if u < 2:
        raise ValueError("u must be at least 2")
    n = min(u, coin_limit) if coin_limit else u
    n = max(2, n)
    rng = random.Random(seed)
    if n == u:
        weights = list(range(1, u + 1))
        rng.shuffle(weights)
    else:
        middle = rng.sample(range(2, u), n - 2) if n > 2 else []
        weights = [1, u, *middle]
        rng.shuffle(weights)
    profits = ([-1] * n if problem == "coinchange" else
               [rng.randint(1, 4 * u) for _ in range(n)])
    target = target_factor * u
    data = f"{n} {u} {target}\n" + "".join(
        f"{weight} {profit}\n" for weight, profit in zip(weights, profits)
    )
    return data, n, target


def add_record(records: list[dict], problem: str, method: str, u: int,
               n: int, target: int, trial: int, result: dict,
               reference: str | None) -> None:
    status = result["status"]
    if status == "ok" and reference is not None and result["stdout"] != reference:
        status = "mismatch"
    record = {
        "problem": problem,
        "method": method,
        "u": u,
        "n": n,
        "target": target,
        "trial": trial,
        "status": status,
        "wall_seconds": result.get("wall_seconds"),
        "algorithm_seconds": result.get("algorithm_seconds"),
        "kernel_seconds": result.get("kernel_seconds"),
        "propagation_seconds": result.get("propagation_seconds"),
    }
    records.append(record)


def benchmark(args, binaries: Path) -> list[dict]:
    records: list[dict] = []
    coin_solver = binaries / "coinchange_solver"
    coin_dp = binaries / "coinchange_traditional"
    knapsack_solver = binaries / "knapsack_solver"
    knapsack_dp = binaries / "knapsack_traditional"
    for problem, sizes, solver, baseline in (
        ("coinchange", args.coin_sizes, coin_solver, coin_dp),
        ("knapsack", args.knapsack_sizes, knapsack_solver, knapsack_dp),
    ):
        methods = (tuple(args.coin_methods) if problem == "coinchange"
                   else ("paper-kernel",))
        for u in sizes:
            for trial in range(args.trials):
                data, n, target = make_instance(
                    problem, u, args.coins, args.target_factor,
                    args.seed + 1000003 * trial + 9176 * u + (problem == "knapsack"),
                )
                reference_result = execute([str(baseline)], data, args.timeout)
                reference = (reference_result["stdout"]
                             if reference_result["status"] == "ok" else None)
                add_record(records, problem, "simple-dp", u, n, target,
                           trial, reference_result, reference)
                for method in methods:
                    command = [str(solver)]
                    if problem == "coinchange":
                        command.append(method)
                    result = execute(command, data, args.timeout)
                    add_record(records, problem, method, u, n, target,
                               trial, result, reference)
                print(f"finished {problem}: u={u} n={n} t={target} "
                      f"trial={trial + 1}/{args.trials}", flush=True)
    return records


def median(rows: list[dict], field: str) -> float | None:
    values = [row[field] for row in rows
              if row["status"] == "ok" and row.get(field) is not None]
    return statistics.median(values) if values else None


def format_seconds(value: float | None) -> str:
    if value is None:
        return "—"
    if value < 0.001:
        return f"{value * 1e6:.0f} µs"
    if value < 1:
        return f"{value * 1000:.2f} ms"
    return f"{value:.3f} s"


def machine_name() -> str:
    if platform.system() == "Darwin":
        try:
            return subprocess.run(
                ["sysctl", "-n", "machdep.cpu.brand_string"], text=True,
                capture_output=True, check=True,
            ).stdout.strip()
        except (OSError, subprocess.CalledProcessError):
            pass
    return platform.processor() or platform.machine()


def markdown_report(records: list[dict], args) -> str:
    generated = dt.datetime.now().astimezone().isoformat(timespec="seconds")
    lines = [
        "# FastKnapsack algorithm benchmark",
        "",
        f"Generated: {generated}",
        "",
        f"Machine: {machine_name()}, {platform.system()} {platform.release()}",
        "",
        (f"Configuration: Release build, median of {args.trials} trial(s), "
         f"timeout {args.timeout:g} seconds per subprocess, "
         f"target `t = {args.target_factor}u`, and "
         f"`n = min(u, {args.coins})`." if args.coins else
         f"Configuration: Release build, median of {args.trials} trial(s), "
         f"timeout {args.timeout:g} seconds per subprocess, "
         f"target `t = {args.target_factor}u`, and `n = u`."),
        "",
        "Every successful optimized result was compared byte for byte with simple DP.",
        "Times are the algorithms' internal compute times and exclude process startup and output.",
        "",
    ]
    for problem in ("coinchange", "knapsack"):
        title = "Coin change" if problem == "coinchange" else "Unbounded knapsack"
        lines.extend([
            f"## {title}", "",
            "| u | n | target | method | median time | kernel | propagation | vs DP | status |",
            "|---:|---:|---:|:---|---:|---:|---:|---:|:---|",
        ])
        problem_rows = [row for row in records if row["problem"] == problem]
        for u in sorted({row["u"] for row in problem_rows}):
            at_size = [row for row in problem_rows if row["u"] == u]
            dp_rows = [row for row in at_size if row["method"] == "simple-dp"]
            dp_time = median(dp_rows, "algorithm_seconds")
            order = (("simple-dp", *args.coin_methods) if problem == "coinchange"
                     else ("simple-dp", "paper-kernel"))
            for method in order:
                rows = [row for row in at_size if row["method"] == method]
                if not rows:
                    continue
                elapsed = median(rows, "algorithm_seconds")
                speedup = (f"{dp_time / elapsed:.2f}×" if dp_time and elapsed else "—")
                statuses = sorted({row["status"] for row in rows})
                status = "ok" if statuses == ["ok"] else ", ".join(statuses)
                lines.append(
                    f"| {u} | {rows[0]['n']} | {rows[0]['target']} | {method} | "
                    f"{format_seconds(elapsed)} | "
                    f"{format_seconds(median(rows, 'kernel_seconds'))} | "
                    f"{format_seconds(median(rows, 'propagation_seconds'))} | "
                    f"{speedup} | {status} |"
                )
        lines.append("")
    lines.extend([
        "## Interpretation notes", "",
        "- `vs DP` is DP time divided by the method time; values above 1 mean the method was faster.",
        "- `paper-kernel` uses the repository's built-in quadratic max-plus convolution backend.",
        "- Randomized methods are timed as Las Vegas implementations: a run returns only after producing an exact verified result.",
        "- Small cases include allocation and setup costs, so their scaling is more useful than isolated microsecond differences.",
        "",
    ])
    return "\n".join(lines)


def write_results(records: list[dict], report: str, output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(report)
    json_path = output.with_suffix(".json")
    json_path.write_text(json.dumps(records, indent=2) + "\n")
    csv_path = output.with_suffix(".csv")
    with csv_path.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(records[0]))
        writer.writeheader()
        writer.writerows(records)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coin-sizes", type=int, nargs="+",
                        default=[32, 64, 128, 256, 512, 1024])
    parser.add_argument("--knapsack-sizes", type=int, nargs="+",
                        default=[32, 64, 128, 256, 512, 1024])
    parser.add_argument("--coins", type=int, default=0,
                        help="cap n; zero means n=u")
    parser.add_argument("--coin-methods", nargs="+", choices=COIN_METHODS,
                        default=list(COIN_METHODS))
    parser.add_argument("--target-factor", type=int, default=8)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument("--seed", type=int, default=20261006)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build-benchmark")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--output", type=Path,
                        default=ROOT / "benchmark_results" / "algorithm_comparison.md")
    args = parser.parse_args()
    if (min(*args.coin_sizes, *args.knapsack_sizes, args.target_factor,
            args.trials) < 1 or args.timeout <= 0 or args.coins < 0):
        parser.error("sizes, factors, trials, and timeout must be positive")
    if not args.no_build:
        configure_and_build(args.build_dir, args.timeout)
    records = benchmark(args, binary_directory(args.build_dir))
    report = markdown_report(records, args)
    write_results(records, report, args.output)
    print(report)
    print(f"Raw results: {args.output.with_suffix('.csv')}")


if __name__ == "__main__":
    main()
