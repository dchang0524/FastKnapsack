#!/usr/bin/env python3
import os
import subprocess
import time
import random
import sys
import re
import matplotlib.pyplot as plt

# --- directory setup -----------------------------------------
# this script now lives in ./knapsack_benchmark/
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
#ROOT_DIR   = os.path.dirname(SCRIPT_DIR)
# --------------------------------------------------------------

# List of target values to test
Ts = [2**i for i in range(10, 25)]

# Problem size parameters
n = 1000   # number of coin types
u = 1000   # maximum coin weight/value

def compile_solvers():
    # optimized solver with debug prints
    subprocess.run([
        "g++-14", "-std=c++20", "-O2",
        "-I", os.path.join(SCRIPT_DIR, "..", "include"),
        os.path.join(SCRIPT_DIR, "..", "src", "convolution.cpp"),
        os.path.join(SCRIPT_DIR, "..", "src", "witness.cpp"),
        os.path.join(SCRIPT_DIR, "..", "src", "hitting_set.cpp"),
        os.path.join(SCRIPT_DIR, "..", "src", "dp_structs.cpp"),
        os.path.join(SCRIPT_DIR, "..", "src", "algorithms.cpp"),
        os.path.join(SCRIPT_DIR, "knapsack.cpp"),
        "-o", os.path.join(SCRIPT_DIR, "knapsack_solver")
    ], check=True)
    # classic DP solver
    subprocess.run([
        "g++-14", "-std=c++20", "-O2",
        os.path.join(SCRIPT_DIR, "traditional_knapsack.cpp"),
        "-o", os.path.join(SCRIPT_DIR, "knapsack_traditional")
    ], check=True)

def generate_coins(n, u):
    # pick n distinct weights from 1..u
    weights = random.sample(range(1, u+1), n)
    # assign random profits independently
    profits = [random.randint(1, u) for _ in range(n)]
    return weights, profits

def build_input(weights, profits, T):
    lines = [f"{n} {u} {T}\n"]
    for w, p in zip(weights, profits):
        lines.append(f"{w} {p}\n")
    return "".join(lines)

def parse_debug(stderr: str):
    # expects exactly these lines in stderr:
    # Kernel computation took X s
    # Max kernel support size: Y
    # Entries in solution with max support size (index Z): (A, B) (C, D) ...
    ktime = maxsup = ptime = tot = None
    max_entries = []
    pair_pattern = re.compile(r"\((\d+), (\d+)\)")  # Regex to match (key, value) pairs

    for line in stderr.splitlines():
        if line.startswith("Kernel computation took"):
            ktime = float(line.split()[3])
        elif line.startswith("Max kernel support size"):
            maxsup = int(line.split()[4])
        elif line.startswith("Entries in solution with max support size"):
            max_entries = [
                (int(m.group(1)), int(m.group(2)))
                for m in pair_pattern.finditer(line)
            ]
        elif line.startswith("Witness propagation took"):
            ptime = float(line.split()[3])
        elif line.startswith("Total elapsed time"):
            tot = float(line.split()[3])
    return ktime, maxsup, ptime, tot, max_entries

def run_benchmark(trials=3):
    # storage for plotting
    opt_totals  = []
    trad_totals = []
    print(" T     |   KTime   MaxSup   PTime   OptTotal   TradTime   OK?")
    print("-------+-------------------------------------------------")
    # generate one coin set once
    weights, profits = generate_coins(n, u)

    for T in Ts:
        inp = build_input(weights, profits, T)

        # optimized solver (capture stdout & stderr)
        proc = subprocess.run(
            ["./knapsack_solver"],
            input=inp,
            text=True,
            capture_output=True,
            check=True
        )
        ktime, maxsup, ptime, opttot, svec = parse_debug(proc.stderr)
        out_opt = proc.stdout.splitlines()
        #print("svec:", svec)
        # traditional solver (just measure wall time, but also capture stdout)
        trad_acc = 0.0
        out_trad = None
        for _ in range(trials):
            t0 = time.perf_counter()
            proc2 = subprocess.run(
                ["./knapsack_traditional"],
                input=inp,
                text=True,
                capture_output=True,
                check=True
            )
            trad_acc += time.perf_counter() - t0
            out_trad = proc2.stdout.splitlines()
        trad_time = trad_acc / trials

        ok = "Y" if out_opt == out_trad else "N"
        if out_opt != out_trad:
            #print the first mistmatch line
            for i, (o1, o2) in enumerate(zip(out_opt, out_trad)):
                if o1 != o2:
                    print(f"Mismatch at line {i}: '{o1}' vs '{o2}'")
                    break

        print(f"{T:6d} | {ktime:8.4f} {maxsup:8d} {ptime:8.4f} {opttot:9.4f} {trad_time:10.4f} {ok}")
        # record for plotting
        opt_totals.append(opttot)
        trad_totals.append(trad_time)
    #plot
    plt.figure()
    plt.plot(Ts, opt_totals)
    plt.plot(Ts, trad_totals)
    plt.xlabel("Target value T")
    plt.ylabel("Run time (s)")
    plt.legend(["Optimized knapsack", "Traditional DP"])
    plt.title("Knapsack: Optimized vs Traditional run time. u = n = 1000")
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    random.seed(0)
    compile_solvers()
    run_benchmark()
