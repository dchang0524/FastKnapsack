#!/usr/bin/env python3
import os
import subprocess
import time
import random
import re
import matplotlib.pyplot as plt

# --- paths ----------------------------------------
SCRIPT = os.path.dirname(os.path.abspath(__file__))
INCLUDE = os.path.join(SCRIPT, '..', 'include')
TRAD    = os.path.join(SCRIPT, 'coinchange_traditional')
SOLVER  = os.path.join(SCRIPT, 'coinchange_simplified_solver')
# --------------------------------------------------

# Benchmark parameters
n, u    = 10000, 10000
Ts      = [2**i for i in range(10, 25)]
TRIALS  = 3

def compile_solvers():
    # traditional DP
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        'coinchange_traditional.cpp',
        '-o', TRAD
    ], cwd=SCRIPT, check=True)

    # kernel‐based solver + all deps
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'coinchange_simplified_solver.cpp',
        os.path.join('..','src','algorithms.cpp'),
        os.path.join('..','src','dp_structs.cpp'),
        os.path.join('..','src','witness.cpp'),
        os.path.join('..','src','convolution.cpp'),
        os.path.join('..','src','hitting_set.cpp'),
        '-o', SOLVER
    ], cwd=SCRIPT, check=True)

def gen_coins():
    weights = random.sample(range(1, u+1), n)
    profits = [-1]*n
    return weights, profits

def build_input(ws, ps, T):
    s = f"{n} {u} {T}\n"
    for w,p in zip(ws, ps):
        s += f"{w} {p}\n"
    return s

def parse_debug(stderr: str):
    """
    Parse the five debug lines from our solver's stderr:
      Kernel computation took X s
      Max kernel support size: Y
      Entries in solution with max support size (...)
      Witness propagation took X s
      Total elapsed time: X s
    """
    ktime = maxsup = ptime = tot = None
    pair_pat = re.compile(r"\((\d+), (\d+)\)")
    entries = []
    for line in stderr.splitlines():
        if line.startswith("Kernel computation took"):
            ktime = float(line.split()[3])
        elif line.startswith("Max kernel support size"):
            maxsup = int(line.split()[4])
        elif line.startswith("Entries in solution with max support size"):
            entries = [(int(m[1]),int(m[2])) 
                       for m in pair_pat.finditer(line)]
        elif line.startswith("Witness propagation took"):
            ptime = float(line.split()[3])
        elif line.startswith("Total elapsed time"):
            tot = float(line.split()[3])
    return ktime, maxsup, entries, ptime, tot

def run_benchmark():
    print("   T    |  KTime  MaxSup  PTime   Total     Trad   OK?")
    print("--------+--------------------------------------------")

    weights, profits = gen_coins()
    tot_times, trad_times = [], []

    for T in Ts:
        inp = build_input(weights, profits, T)

        # --- optimized run ---
        proc = subprocess.run(
            [SOLVER], input=inp, text=True,
            capture_output=True, check=True
        )
        ktime, maxsup, entries, ptime, tot = parse_debug(proc.stderr)
        out_opt = proc.stdout.splitlines()

        # --- traditional DP timing & output ---
        acc, out_trad = 0.0, None
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            p2 = subprocess.run(
                [TRAD], input=inp, text=True,
                capture_output=True, check=True
            )
            acc += time.perf_counter() - t0
            out_trad = p2.stdout.splitlines()
        trad = acc / TRIALS

        ok = "Y" if out_opt == out_trad else "N"
        print(f"{T:8d} | {ktime:7.4f} {maxsup:6d} {ptime:7.4f} {tot:8.4f} {trad:8.4f}   {ok}")

        tot_times.append(tot)
        trad_times.append(trad)

    # --- plot ---
    plt.figure()
    plt.plot(Ts, tot_times, marker='o')
    plt.plot(Ts, trad_times,  marker='o')
    plt.xscale('log', base=2)
    plt.xlabel("Target T")
    plt.ylabel("Time (s)")
    plt.legend(["Kernel + Propagation", "Traditional DP"])
    plt.title("CoinChange: Kernel vs Traditional")
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    random.seed(0)
    compile_solvers()
    run_benchmark()
