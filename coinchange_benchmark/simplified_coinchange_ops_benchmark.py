#!/usr/bin/env python3
import os
import subprocess
import random
import matplotlib.pyplot as plt

BASE    = os.path.dirname(os.path.realpath(__file__))
INCLUDE = os.path.join(BASE, '..', 'include')
EXE     = os.path.join(BASE, 'simplified_coinchange_ops')
LOG     = os.path.join(BASE, 'coinchange_ops_benchmark.log')

US = [1000, 2000, 5000, 10000, 20000, 40000, 80000]
NS = [1000, 2000, 5000, 10000, 20000, 40000, 80000]
TRIALS = 1

def compile_solver():
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'simplified_coinchange_ops.cpp',
        os.path.join('..','src','dp_structs.cpp'),
        os.path.join('..','src','convolution.cpp'),
        '-o', EXE
    ], cwd=BASE, check=True)

def parse_metrics(stderr: str):
    m = {}
    for line in stderr.splitlines():
        if line.startswith("Kernel time:"):
            _, rest = line.split(":",1)
            m['kernel_time'] = float(rest.strip().split()[0])
        elif line.startswith("Count of O(1) blocks:"):
            _, rest = line.split(":",1)
            m['count_o1'] = int(rest.strip())
        elif line.startswith("Count of O(N"):
            _, rest = line.split(":",1)
            m['count_osqrt'] = int(rest.strip())
        elif line.startswith("Estimated total ops"):
            _, rest = line.split(":",1)
            m['est_ops'] = float(rest.strip().split()[-1])
    return m

def run_benchmark():
    compile_solver()
    with open(LOG, 'w') as f:
        f.write("coinchange_ops_benchmark\n\n")

    print("    u    |    n    | Kernel(s) |  O1-blocks | Conv-blocks | Est-ops")
    print("---------+---------+-----------+------------+-------------+----------")

    records = []
    for u, n in zip(US, NS):
        w = [0] + random.sample(range(1, u+1), n)
        p = [-1]*(n+1)

        inp = f"{n} {u} {u}\n"
        for i in range(1, n+1):
            inp += f"{w[i]} {p[i]}\n"

        proc = subprocess.run(
            [EXE],
            input=inp, text=True,
            capture_output=True, check=True
        )
        m = parse_metrics(proc.stderr)
        records.append((u, n, m))

        print(f"{u:8d} | {n:7d} | {m['kernel_time']:9.3f} | "
              f"{m['count_o1']:10d} | {m['count_osqrt']:11d} | "
              f"{m['est_ops']:8.0f}")

        with open(LOG, 'a') as f:
            f.write(f"\nu={u}, n={n}\n{proc.stderr}\n")

    print(f"\nDetails logged to {LOG}")

    us         = [r[0] for r in records]
    kernel_t   = [r[2]['kernel_time'] for r in records]
    o1_blocks  = [r[2]['count_o1']     for r in records]
    conv_blocks= [r[2]['count_osqrt']   for r in records]
    est_ops    = [r[2]['est_ops']       for r in records]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12,4), sharex=True)

    ax1.plot(us, kernel_t, marker='o')
    ax1.set_xlabel('u')
    ax1.set_ylabel('Kernel time (s)')
    ax1.set_title('Kernel Timing')
    ax1.set_xlim(min(US), max(US))

    ax2.plot(us, o1_blocks, marker='o', label='O(1)-blocks')
    ax2.plot(us, conv_blocks, marker='s', label='Conv-blocks')
    ax2.plot(us, est_ops,   marker='d', label='Est-ops')
    ax2.set_xlabel('u')
    ax2.set_ylabel('Count')
    ax2.set_title('Asymptotic Block Counts')
    ax2.legend(fontsize='small')
    ax2.set_xlim(min(US), max(US))

    plt.tight_layout()
    plt.show()

if __name__=='__main__':
    random.seed(0)
    run_benchmark()
