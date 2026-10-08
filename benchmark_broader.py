#!/usr/bin/env python3
"""Broader input benchmarks with exact validation and incremental checkpoints."""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import math
import os
import random
import statistics
import subprocess
import time
from pathlib import Path

from benchmark_all import ROOT, COIN_METHODS, execute, format_seconds, machine_name, write_results

FAMILIES = ("dense", "random-quarter", "sparse-32", "upper-half",
            "geometric", "gcd-four", "random-no-one")
STATS = ("requested_enumeration_calls", "pair_enumeration_calls", "sampling_calls",
         "dilution_chains", "weighted_convolutions", "max_k")


def instance(problem, family, u, factor, seed):
    rng = random.Random(seed)
    if family == "dense":
        weights = list(range(1, u + 1))
    elif family == "random-quarter":
        weights = [1, u, *rng.sample(range(2, u), u // 4 - 2)]
    elif family == "sparse-32":
        weights = [1, u, *rng.sample(range(2, u), min(32, u) - 2)]
    elif family == "upper-half":
        weights = list(range(u // 2 + 1, u + 1))
    elif family == "geometric":
        weights = sorted({1 << i for i in range(u.bit_length())} | {u})
    elif family == "gcd-four":
        weights = list(range(4, u + 1, 4))
    elif family == "random-no-one":
        weights = [u, *rng.sample(range(2, u), u // 4 - 1)]
    else:
        raise ValueError(family)
    rng.shuffle(weights)
    profits = [-1] * len(weights) if problem == "coinchange" else [rng.randint(1, 4*u) for _ in weights]
    data = f"{len(weights)} {u} {factor*u}\n" + "".join(f"{w} {p}\n" for w,p in zip(weights, profits))
    return data, len(weights), math.gcd(*weights)


def med(rows, key):
    values = [r[key] for r in rows if r["status"] == "ok" and r.get(key) is not None]
    return statistics.median(values) if values else None


def report(records, args):
    lines = ["# Broader input benchmark", "", f"Updated: {dt.datetime.now().astimezone().isoformat(timespec='seconds')}", "",
             f"Machine: {machine_name()}. Release build, serial processes, {args.trials} trials per case, independent {args.timeout:g}-second timeout.", "",
             "`u` is the largest weight, `n` the number of distinct items, and `t` the largest target solved (all targets 0 through t are output).",
             "Times below are median internal compute times, excluding process startup and output. Coin-change inputs use shuffled item order; knapsack profits are independent uniform integers in [1,4u].",
             "Every completed optimized solve is compared byte for byte with simple DP; witness recovery is checked for exact cardinality, validity, and distinctness using an independent scan.", "",
             "Input seed is recorded in JSON/CSV, along with the SHA-256 hash of each solver input. Sampling uses the production random_device seed, so its exact timings are not reproducible from the input seed.", "",
             "## Coverage", "",
             "- Dense: all weights 1..u. Random-quarter: u/4 weights including 1 and u. Sparse-32: 32 weights including 1 and u.",
             "- Upper-half: every weight above u/2; no small weights. Geometric: powers of two plus u.",
             "- Gcd-four: every multiple of four; three quarters of targets are unreachable. Random-no-one: u/4 random weights with u included, excluding 1.",
             "- Short targets: all seven families at u=256,512,1024 and t=8u, randomized-k/simplified/DP plus knapsack.",
             "- All five coin-change variants: all seven families at u=256 and t=8u. Larger deterministic/optimized-peeling cases are omitted because their large branches previously timed out; this is not a full all-variant scaling study.",
             "- Long targets: all seven families at u=512 and t=4096u, randomized-k/DP plus knapsack. This tests whether the dense-input crossover generalizes.", "",
             "## End-to-end comparisons", "",
             "DP / method greater than 1 means the method wins. Status includes missing or failed trials; a median of completed trials does not imply all trials completed.", "",
             "| problem | family | u | n | t/u | method | time | DP / method | sampling calls | enumeration calls | status |",
             "|:--|:--|--:|--:|--:|:--|--:|--:|--:|--:|:--|"]
    solver = [r for r in records if r["problem"] != "witness"]
    groups = sorted({(r["problem"],r["family"],r["u"],r["factor"]) for r in solver})
    for problem, family, u, factor in groups:
        group = [r for r in solver if (r["problem"],r["family"],r["u"],r["factor"]) == (problem,family,u,factor)]
        dp = med([r for r in group if r["method"] == "simple-dp"], "algorithm_seconds")
        for method in ("simple-dp", *COIN_METHODS, "paper-kernel"):
            rows = [r for r in group if r["method"] == method]
            if not rows: continue
            elapsed = med(rows, "algorithm_seconds")
            speed = f"{dp/elapsed:.3f}×" if dp and elapsed else "—"
            sampling = med(rows, "sampling_calls")
            enum = med(rows, "requested_enumeration_calls")
            status = ", ".join(sorted({r["status"] for r in rows})) + f" ({len(rows)}/{args.trials})"
            lines.append(f"| {problem} | {family} | {u} | {rows[0]['n']} | {factor} | {method} | {format_seconds(elapsed)} | {speed} | {sampling if sampling is not None else '—'} | {enum if enum is not None else '—'} | {status} |")
    lines += ["", "## Actual randomized witness branch", "",
              "Arrays have lengths A and 2A, N=3A-1 outputs, and k=ceil(log2 N). The requested work exceeds 8,000,000 and every completed run asserts that sampling was used. Direct-k scans candidate positions independently for each requested output and stops at k; it is a practical comparison, not the production pair-enumeration branch.", "",
              "Dense-broad requests all outputs (many witness-count scales). Dense-narrow requests only sums A-1 through 2A-1 (each has A witnesses). Random-half and random-sparse independently retain positions with probabilities 1/2 and 0.02. Periodic uses multiples of 7 in a and 11 in b. Separated-blocks uses only the first/last eighth of each array.", "",
              "| family | A | k | randomized | direct-k | direct / random | chains | weighted convolutions | status |",
              "|:--|--:|--:|--:|--:|--:|--:|--:|:--|"]
    witness = [r for r in records if r["problem"] == "witness"]
    for family, length in sorted({(r["family"],r["u"]) for r in witness}):
        rows = [r for r in witness if r["family"] == family and r["u"] == length]
        rand, direct = med(rows,"random_seconds"), med(rows,"direct_seconds")
        speed = f"{direct/rand:.4f}×" if rand and direct else "—"
        lines.append(f"| {family} | {length} | {rows[0].get('k','—')} | {format_seconds(rand)} | {format_seconds(direct)} | {speed} | {med(rows,'dilution_chains')} | {med(rows,'weighted_convolutions')} | {', '.join(sorted({r['status'] for r in rows}))} ({len(rows)}/{args.trials}) |")
    lines += ["", "## Limits", "",
              "This is a finite, structured sweep, not a worst-case guarantee or a statistical survey of real workloads. Two trials give limited information about randomized tails. The witness primitive cases are synthetic and are reported separately from end-to-end Algorithm 4. Knapsack still uses the repository's quadratic max-plus convolution backend.", ""]
    return "\n".join(lines)


def summary(records, args):
    solver = [r for r in records if r["problem"] != "witness"]
    witness = [r for r in records if r["problem"] == "witness"]
    statuses = {s:sum(r["status"] == s for r in records) for s in sorted({r["status"] for r in records})}
    sampling = sum(r.get("sampling_calls") or 0 for r in solver)
    enum = sum(r.get("requested_enumeration_calls") or 0 for r in solver)
    def rows(problem,family,u,factor,method):
        return [r for r in solver if (r["problem"],r["family"],r["u"],r["factor"],r["method"]) == (problem,family,u,factor,method)]
    lines = ["# Broader benchmark findings", "", f"{len(solver)} end-to-end runs and {len(witness)} witness runs; statuses: {statuses}.", "",
             f"Release builds on {machine_name()}, {args.trials} trials, 600-second limit per algorithm. Full results, exact-check outcomes, input seeds, input hashes, and branch counters: [broader_inputs.md](broader_inputs.md), [JSON](broader_inputs.json), [CSV](broader_inputs.csv).",
             "", "## Short targets: coin change", "", "u=1024, t=8u. Times are medians of internal compute times; DP / randomized-k above 1 means randomized-k wins.", "",
             "| denomination family | n | simple DP | randomized-k | DP / randomized-k |", "|:--|--:|--:|--:|--:|"]
    for family in FAMILIES:
        dp_rows = rows("coinchange",family,1024,8,"simple-dp")
        rk_rows = rows("coinchange",family,1024,8,"randomized-k")
        if not dp_rows or not rk_rows: continue
        dp,rk = med(dp_rows,"algorithm_seconds"),med(rk_rows,"algorithm_seconds")
        speed = f"{dp/rk:.3g}×" if dp and rk else "—"
        lines.append(f"| {family} | {dp_rows[0]['n']} | {format_seconds(dp)} | {format_seconds(rk)} | {speed} |")
    lines += ["", "## Long targets: does the DP crossover generalize?", "", "u=512, t=4096u=2,097,152. The speedup columns are DP time divided by kernel-method time.", "",
              "| family | n | coin DP | randomized-k | coin speedup | knapsack DP | knapsack kernel | knapsack speedup |",
              "|:--|--:|--:|--:|--:|--:|--:|--:|"]
    for family in FAMILIES:
        groups = [rows(p,family,512,4096,m) for p,m in (("coinchange","simple-dp"),("coinchange","randomized-k"),("knapsack","simple-dp"),("knapsack","paper-kernel"))]
        if not all(groups): continue
        cd,cr,kd,kr = [med(group,"algorithm_seconds") for group in groups]
        cs = f"{cd/cr:.2f}×" if cd and cr else "—"
        ks = f"{kd/kr:.2f}×" if kd and kr else "—"
        lines.append(f"| {family} | {groups[0][0]['n']} | {format_seconds(cd)} | {format_seconds(cr)} | {cs} | {format_seconds(kd)} | {format_seconds(kr)} | {ks} |")
    lines += ["", "## Randomized sampling without enumeration shortcuts", "", "A=16,384; arrays have lengths A and 2A; k=16. Direct-k independently scans candidate positions for each output until k valid witnesses have been found. Every randomized run asserts that it used sampling.", "",
              "| witness family | randomized-k | direct-k | weighted convolutions |", "|:--|--:|--:|--:|"]
    for family in ("dense-broad","dense-narrow","random-half","random-sparse","periodic","separated-blocks"):
        group = [r for r in witness if r["family"] == family and r["u"] == 16384]
        if not group: continue
        lines.append(f"| {family} | {format_seconds(med(group,'random_seconds'))} | {format_seconds(med(group,'direct_seconds'))} | {med(group,'weighted_convolutions')} |")
    lines += ["", "## What these results establish", "",
              "- DP wins every tested short-target comparison (t=8u), including all five coin-change variants at u=256.",
              "- At u=512 and t=4096u, coin-change randomized-k beats DP only for the dense family in this sweep. Knapsack's kernel wins for dense, random-quarter, and gcd-four inputs; its 1.04× median upper-half advantage is too small to treat as decisive from two trials.",
              "- The actual randomized witness branch is slower than direct-k scanning in every measured witness family and size. Its restricted dense case is about 27× faster than its broad dense case at A=16,384.",
              f"- End-to-end solver runs recorded {sampling} randomized sampling calls and {enum} requested-output enumeration calls. The separate witness benchmark establishes the performance of the actual sampling branch.",
              "- Density, the availability of small denominations, unreachable targets, the number of denominations, and target length all affect the comparison. Long-target speedups on the dense family cannot be assumed for sparse families.",
              "- Dense-narrow requests the central interval, where every output shares all A witnesses. Dense-broad includes edge outputs with many different witness counts. This is a controlled example of input sensitivity; it does not isolate witness-count diversity from shared-set correlation.",
              "- These finite measurements neither prove a better asymptotic bound nor establish worst-case behavior. Two trials do not characterize random-runtime tails.",
              "- All coin-change variants were compared at u=256. At larger sizes, the focus is randomized-k/simplified/DP and knapsack; deterministic and optimized-peeling large branches were omitted because of their earlier timeouts. The README's u=65,536 claim remains untested.", ""]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=Path("/tmp/fastknapsack-broader-build"))
    parser.add_argument("--output", type=Path, default=ROOT/"benchmark_results/broader_inputs.md")
    parser.add_argument("--trials", type=int, default=2)
    parser.add_argument("--timeout", type=float, default=600)
    parser.add_argument("--seed", type=int, default=20261007)
    parser.add_argument("--phase", choices=["all","solver","witness"], default="all")
    args = parser.parse_args()
    if args.trials < 1 or args.timeout <= 0: parser.error("positive trials and timeout required")
    os.environ["FASTKNAPSACK_WITNESS_STATS"] = "1"
    raw = args.output.with_suffix(".json")
    records = json.loads(raw.read_text()) if raw.exists() else []
    def save(row):
        records.append(row)
        # Stable columns make checkpoint CSV valid across both benchmark types.
        fields = sorted({k for record in records for k in record})
        normalized = [{key: record.get(key) for key in fields} for record in records]
        write_results(normalized, report(records,args), args.output)
        print(f"{row['problem']} {row['family']} u={row['u']} factor={row.get('factor')} {row['method']} trial={row['trial']}: {row['status']} {row.get('algorithm_seconds',row.get('random_seconds'))}s", flush=True)
    def exists(problem,family,u,factor,method,trial):
        return any((r["problem"],r["family"],r["u"],r.get("factor"),r["method"],r["trial"]) == (problem,family,u,factor,method,trial) for r in records)
    if args.phase in ("all","solver"):
        cases = [(u,8) for u in (256,512,1024)] + [(512,4096)]
        for u,factor in cases:
            for fi,family in enumerate(FAMILIES):
                for problem in ("coinchange","knapsack"):
                    for trial in range(args.trials):
                        seed = args.seed + 1000003*trial + 9176*u + 101*fi + (problem == "knapsack")
                        data,n,gcd = instance(problem,family,u,factor,seed)
                        methods = (("randomized-k", "simplified") if factor == 8 else ("randomized-k",)) if problem == "coinchange" else ("paper-kernel",)
                        if problem == "coinchange" and u == 256: methods = COIN_METHODS
                        if all(exists(problem,family,u,factor,m,trial) for m in ("simple-dp",*methods)): continue
                        baseline = args.build_dir/("coinchange_traditional" if problem == "coinchange" else "knapsack_traditional")
                        reference = execute([str(baseline)],data,args.timeout)
                        for method in ("simple-dp",*methods):
                            if exists(problem,family,u,factor,method,trial): continue
                            command = [str(args.build_dir/("coinchange_solver" if problem == "coinchange" else "knapsack_solver"))]
                            if problem == "coinchange": command.append(method)
                            result = reference if method == "simple-dp" else execute(command,data,args.timeout)
                            status = result["status"]
                            if status == "ok" and method != "simple-dp":
                                status = ("ok" if result["stdout"] == reference["stdout"] else "mismatch") if reference["status"] == "ok" else "unverified"
                            row = dict(problem=problem,family=family,u=u,n=n,factor=factor,target=factor*u,gcd=gcd,seed=seed,input_sha256=hashlib.sha256(data.encode()).hexdigest(),method=method,trial=trial,status=status)
                            row.update({key:result.get(key) for key in ("wall_seconds","algorithm_seconds","kernel_seconds","propagation_seconds")})
                            for line in (result.get("stderr") or "").splitlines():
                                if line.startswith("Witness stats: "): row.update(zip(STATS,map(int,line.split(": ")[1].split())))
                            if status not in ("ok","timeout"): row["diagnostic"] = (result.get("stderr") or "")[-2000:]
                            save(row)
    if args.phase in ("all","witness"):
        for length in (4096,8192,16384):
            for family in ("dense-broad","dense-narrow","random-half","random-sparse","periodic","separated-blocks"):
                for trial in range(args.trials):
                    if exists("witness",family,length,None,"randomized-k",trial): continue
                    seed = args.seed + 1000003*trial + length
                    started = time.perf_counter()
                    result = execute([str(args.build_dir/"witness_profile"),family,str(length),str(seed)],"",args.timeout)
                    row = dict(problem="witness",family=family,u=length,method="randomized-k",trial=trial,seed=seed,status=result["status"],wall_seconds=time.perf_counter()-started)
                    if result["status"] == "ok": row.update(json.loads(result["stdout"]))
                    else: row["diagnostic"] = result.get("stderr")
                    save(row)
    print(f"Report: {args.output}",flush=True)
    summary_path = args.output.with_name(args.output.stem + "_summary.md")
    summary_path.write_text(summary(records,args))
    print(f"Summary: {summary_path}",flush=True)


if __name__ == "__main__": main()
