#!/usr/bin/env python3
import os
import subprocess
import random
import time
import math
import sys
import matplotlib.pyplot as plt

BASE     = os.path.dirname(os.path.realpath(__file__))
INCLUDE  = os.path.join(BASE, '..', 'include')
SRC      = os.path.join(BASE, 'random_kWit_test.cpp')
SOLVER   = os.path.join(BASE, 'random_kWit_bench')
LOG_FILE = os.path.join(BASE, 'mismatch.log')

# test sizes and repetitions
SIZES         = [10, 20, 30, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
TRIALS        = 3
MAX_VALIDATE  = 10_000  # skip detailed validation for n > MAX_VALIDATE

# convolution counts true pairs
def convolution(a, b):
    n, m = len(a), len(b)
    c = [0] * (n + m - 1)
    for i in range(n):
        if a[i]:
            for j in range(m):
                if b[j]:
                    c[i + j] += 1
    return c

# validate witnesses: returns (ok, list of (index, reason, got_list))
def validate_witnesses(a, b, w, order, k, got):
    errs = []
    c = convolution(a, b)
    n = len(a)
    R = len(c)
    for i in range(R):
        ws = got[i]
        exp_cnt = min(c[i], k)
        if len(ws) != exp_cnt:
            errs.append((i, f"expected {exp_cnt}, got {len(ws)} witnesses", ws))
            continue
        seen = set()
        for pos in ws:
            if pos < 1 or pos >= n:
                errs.append((i, f"invalid position {pos}", ws)); break
            if pos in seen:
                errs.append((i, f"duplicate position {pos}", ws)); break
            seen.add(pos)
            idx = order[pos]
            j = i - w[idx]
            if j < 0 or j >= n:
                errs.append((i, f"witness pos {pos} gives j={j} out of range", ws)); break
            if not (a[idx] == 1 and b[j] == 1):
                errs.append((i, f"pos {pos}: a[{idx}]={a[idx]}, b[{j}]={b[j]} not both 1", ws)); break
    return (len(errs) == 0, errs)

# compile solver binary
def compile_solver():
    print("Compiling solver...")
    subprocess.run([
        'g++-14', '-std=c++20', '-O2',
        '-I', INCLUDE,
        SRC,
        os.path.join(BASE, '..', 'src', 'convolution.cpp'),
        os.path.join(BASE, '..', 'src', 'witness.cpp'),
        '-o', SOLVER
    ], check=True)

# generate input case: order[0]=0, a[order[i]]=1, w[0]=0, distinct weights
def generate_case(n):
    k = 2 * math.ceil(math.log2(n))
    order = list(range(n))
    rest = order[1:]
    random.shuffle(rest)
    order[1:] = rest
    a = [0] * n
    for pos in range(1, n):
        a[order[pos]] = 1
    b = [random.randint(0, 1) for _ in range(n)]
    w = [0] + random.sample(range(1, n), n-1)
    inp = "\n".join([
        f"{n} {k}",
        " ".join(map(str, a)),
        " ".join(map(str, b)),
        " ".join(map(str, w)),
        " ".join(map(str, order)),
        ""
    ])
    return inp, a, b, w, order, k

# run one trial: returns (time, ok)
def run_one(n):
    inp, a, b, w, order, k = generate_case(n)
    t0 = time.perf_counter()
    proc = subprocess.run([SOLVER], input=inp, text=True, capture_output=True, check=True)
    elapsed = time.perf_counter() - t0
    got = []
    for line in proc.stdout.strip().splitlines():
        parts = list(map(int, line.split())); cnt = parts[0]; got.append(parts[1:1+cnt])
    ok = True
    if n <= MAX_VALIDATE:
        ok, errs = validate_witnesses(a, b, w, order, k, got)
        if not ok:
            with open(LOG_FILE, 'a') as f:
                f.write(f"Sample n={n}, k={k}\n")
                f.write(f"a = {a}\n")
                f.write(f"b = {b}\n")
                f.write(f"w = {w}\n")
                f.write(f"order = {order}\n")
                for idx, reason, ws in errs:
                    f.write(f"Index {idx}: got {ws}, reason: {reason}\n")
                f.write("\n")
    return elapsed, ok

# benchmark loop
if __name__ == '__main__':
    random.seed(42)
    with open(LOG_FILE, 'w') as f:
        f.write("Mismatch Log\n\n")
    compile_solver()
    print(f"{'n':>6} | {'avg time (s)':>12} | {'OK?':>4}")
    print("-" * 30)
    times = []
    for n in SIZES:
        trial_times, trial_oks = [], []
        for _ in range(TRIALS):
            t, ok = run_one(n)
            trial_times.append(t); trial_oks.append(ok)
        avg = sum(trial_times) / TRIALS
        ok_all = all(trial_oks)
        times.append(avg)
        print(f"{n:6} | {avg:12.6f} | {('Y' if ok_all else 'N'):>4}")
    plt.plot(SIZES, times, marker='o')
    plt.xlabel('n')
    plt.ylabel('Average time (s)')
    plt.title('randomized_k_witness Benchmark')
    plt.grid(True)
    plt.show()
