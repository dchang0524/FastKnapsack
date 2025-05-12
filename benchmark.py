#!/usr/bin/env python3
import subprocess
import time
import random
import matplotlib.pyplot as plt

def compile_solver():
    # Adjust compiler and flags as needed
    cmd = [
        "g++-14",
        "-std=c++20",
        "-O2",
        "-Iinclude",
        "src/convolution.cpp",
        "src/peeling.cpp",
        "src/hitting_set.cpp",
        "src/dp_structs.cpp",
        "src/algorithms.cpp",
        "main.cpp",
        "-o", "knapsack_solver"
    ]
    subprocess.run(cmd, check=True)

def generate_instance(n, u, t):
    # Random weights in [1, u], random profits in [1, u]
    w = [random.randint(1, u) for _ in range(n)]
    p = [random.randint(1, u) for _ in range(n)]
    lines = [f"{n} {u} {t}\n"]
    for wi, pi in zip(w, p):
        lines.append(f"{wi} {pi}\n")
    return "".join(lines)

def benchmark(solver="./knapsack_solver", trials=3):
    Ns = [2**i for i in range(10, 17)]  # sample T values from 1k to 128k
    avg_times = []
    for T in Ns:
        u = T // 2 or 1
        n = min(20, T)  # keep n small relative to T
        total = 0.0
        for _ in range(trials):
            inp = generate_instance(n, u, T)
            start = time.perf_counter()
            subprocess.run([solver],
                           input=inp,
                           text=True,
                           stdout=subprocess.DEVNULL,
                           stderr=subprocess.DEVNULL,
                           check=True)
            total += time.perf_counter() - start
        avg = total / trials
        avg_times.append(avg)
        print(f"T={T:6d}  time={avg:.4f}s")
    return Ns, avg_times

def plot_results(Ns, times):
    plt.figure()
    plt.plot(Ns, times, marker='o', linestyle='-')
    plt.xscale('log', base=2)
    plt.yscale('log')
    plt.xlabel('Maximum target T')
    plt.ylabel('Average runtime (s)')
    plt.title('Knapsack Solver Scaling')
    plt.grid(True, which='both', linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    compile_solver()
    Ns, times = benchmark()
    plot_results(Ns, times)
