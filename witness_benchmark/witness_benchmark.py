#!/usr/bin/env python3
import subprocess
import time
import random
import os
import matplotlib.pyplot as plt

BASE_DIR   = os.path.dirname(os.path.realpath(__file__))
INCLUDE    = os.path.join(BASE_DIR, '..', 'include')
OPT_EXE    = os.path.join(BASE_DIR, 'optimized_witness_test')
NAIVE_EXE  = os.path.join(BASE_DIR, 'naive_witness_test')
LOG_FILE   = os.path.join(BASE_DIR, 'witness_mismatches.log')

# Sizes to test and trial count
SIZES  = [256, 512, 1024, 2048]
TRIALS = 3

opt_times   = []
naive_times = []

def compile_tests():
    # compile optimized version (needs witness.cpp + convolution.cpp)
    subprocess.run([
        'g++-14', '-std=c++20', '-O2',
        '-I', INCLUDE,
        'optimized_witness_test.cpp',
        os.path.join('..', 'src', 'witness.cpp'),
        os.path.join('..', 'src', 'convolution.cpp'),
        '-o', OPT_EXE
    ], cwd=BASE_DIR, check=True)
    # compile naive reference
    subprocess.run([
        'g++-14', '-std=c++20', '-O2',
        '-I', INCLUDE,
        'naive_witness_test.cpp',
        '-o', NAIVE_EXE
    ], cwd=BASE_DIR, check=True)

def run_benchmark():
    # Initialize log file
    with open(LOG_FILE, 'w') as log:
        log.write('Witness Mismatch Log\n')
        log.write('====================\n\n')

    # Print header
    print('n    | Optimized(s) | Naive(s) | Match?')
    print('-----+--------------+----------+-------')

    for n in SIZES:
        # generate random test data
        a     = [random.randint(0,1) for _ in range(n)]
        b     = [random.randint(0,1) for _ in range(n)]
        order = random.sample(range(n), n)

        # build input string: n, then a[], b[], order[] each on its own line
        inp  = f"{n}\n"
        inp += ' '.join(map(str, a)) + "\n"
        inp += ' '.join(map(str, b)) + "\n"
        inp += ' '.join(map(str, order)) + "\n"

        # correctness check
        proc_opt   = subprocess.run([OPT_EXE],   input=inp, text=True, capture_output=True)
        proc_naive = subprocess.run([NAIVE_EXE], input=inp, text=True, capture_output=True)
        out_opt    = list(map(int, proc_opt.stdout.strip().split()))
        out_naive  = list(map(int, proc_naive.stdout.strip().split()))
        match      = (out_opt == out_naive)

        if not match:
            # log first mismatch
            with open(LOG_FILE, 'a') as log:
                log.write(f'n = {n} mismatch at index:\n')
                for idx, (x, y) in enumerate(zip(out_opt, out_naive)):
                    if x != y:
                        log.write(f'  idx={idx}: opt={x}, naive={y}\n')
                        break
                log.write(f'  a     = {a}\n')
                log.write(f'  b     = {b}\n')
                log.write(f'  order = {order}\n\n')

        # benchmark optimized via wall-clock
        t_opt = 0.0
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            subprocess.run([OPT_EXE], input=inp, text=True, capture_output=True, check=True)
            t_opt += time.perf_counter() - t0
        t_opt /= TRIALS
        opt_times.append(t_opt)

        # benchmark naive via wall-clock
        t_naive = 0.0
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            subprocess.run([NAIVE_EXE], input=inp, text=True, capture_output=True, check=True)
            t_naive += time.perf_counter() - t0
        t_naive /= TRIALS
        naive_times.append(t_naive)

        print(f"{n:<5}| {t_opt:12.6f} | {t_naive:8.6f} | {'Y' if match else 'N'}")

    print(f"\nDetails logged to {LOG_FILE}")

    # plot results
    plt.figure()
    plt.plot(SIZES, opt_times, marker='o')
    plt.plot(SIZES, naive_times, marker='o')
    plt.xlabel('n')
    plt.ylabel('Time (s)')
    plt.legend(['Optimized', 'Naive'])
    plt.title('Minimum-Witness Boolean Convolution Benchmark')
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    random.seed(0)
    compile_tests()
    run_benchmark()
