#!/usr/bin/env python3
import os
import subprocess
import random
import time
import matplotlib.pyplot as plt

BASE_DIR   = os.path.dirname(os.path.realpath(__file__))
INCLUDE    = os.path.join(BASE_DIR, '..', 'include')
OPT_EXE    = os.path.join(BASE_DIR, 'optimized_witness_test')
NAIVE_EXE  = os.path.join(BASE_DIR, 'naive_witness_test')
LOG_FILE   = os.path.join(BASE_DIR, 'witness_mismatches.log')

# Sizes to test and trial count
SIZES  = [256, 512, 1024, 2048, 4096, 8192, 16384]
TRIALS = 3

opt_times   = []
naive_times = []

def compile_tests():
    # Optimized: needs witness.cpp + convolution.cpp
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'optimized_witness_test.cpp',
        os.path.join('..','src','witness.cpp'),
        os.path.join('..','src','convolution.cpp'),
        '-o', OPT_EXE
    ], cwd=BASE_DIR, check=True)

    # Naive reference
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        'naive_witness_test.cpp',
        '-o', NAIVE_EXE
    ], cwd=BASE_DIR, check=True)

def run_benchmark():
    # Prepare log
    with open(LOG_FILE, 'w') as log:
        log.write("Witness Mismatch Log\n")
        log.write("====================\n\n")

    print(' n     | Optimized(s) | Naive(s) | Match?')
    print('-------+--------------+----------+-------')

    for n in SIZES:
        # Generate random vectors
        a     = [random.randint(0,1) for _ in range(n)]
        b     = [random.randint(0,1) for _ in range(n)]
        # Use identity weights so w[i]=i
        w     = list(range(n))
        order = random.sample(range(n), n)

        # Build the input block
        inp = f"{n}\n"
        inp += ' '.join(map(str, a))     + "\n"
        inp += ' '.join(map(str, b))     + "\n"
        inp += ' '.join(map(str, w))     + "\n"
        inp += ' '.join(map(str, order)) + "\n"

        # Run optimized once for time+output
        tote = 0.0
        out_opt = None
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            proc = subprocess.run([OPT_EXE],
                                  input=inp, text=True,
                                  capture_output=True, check=True)
            tote += time.perf_counter() - t0
            out_opt = list(map(int, proc.stdout.split()))
        opt_time = tote / TRIALS

        # Run naive once for time+output
        totn = 0.0
        out_naive = None
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            proc = subprocess.run([NAIVE_EXE],
                                  input=inp, text=True,
                                  capture_output=True, check=True)
            totn += time.perf_counter() - t0
            out_naive = list(map(int, proc.stdout.split()))
        naive_time = totn / TRIALS

        ok = out_opt == out_naive
        print(f"{n:<7}| {opt_time:12.6f} | {naive_time:8.6f} | {'Y' if ok else 'N'}")

        # Log mismatches in detail
        if not ok:
            with open(LOG_FILE, 'a') as log:
                log.write(f'\nn = {n} mismatch\n')
                log.write(f'a     = {a}\n')
                log.write(f'b     = {b}\n')
                log.write(f'w     = {w}\n')
                log.write(f'order = {order}\n')
                log.write('idx : opt vs naive\n')
                for idx, (xo, xn) in enumerate(zip(out_opt, out_naive)):
                    if xo != xn:
                        log.write(f'  {idx}: {xo} vs {xn}\n')
                log.write('\n')

        opt_times.append(opt_time)
        naive_times.append(naive_time)

    print(f"\nDetails logged to {LOG_FILE}")

    # Plot
    plt.figure()
    plt.plot(SIZES, opt_times,   marker='o')
    plt.plot(SIZES, naive_times, marker='o')
    plt.xlabel('n (vector size)')
    plt.ylabel('Time (s)')
    plt.legend(['Optimized','Naive'])
    plt.title('Minimum-Witness Boolean Convolution Benchmark')
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    random.seed(0)
    compile_tests()
    run_benchmark()
