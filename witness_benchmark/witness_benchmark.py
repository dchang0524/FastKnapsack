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
SIZES  = [256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
TRIALS = 3

opt_times   = []
naive_times = []

def compile_tests():
    # optimized needs witness.cpp + convolution.cpp
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'optimized_witness_test.cpp',
        os.path.join('..','src','witness.cpp'),
        os.path.join('..','src','convolution.cpp'),
        '-o', OPT_EXE
    ], cwd=BASE_DIR, check=True)
    # naive reference
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'naive_witness_test.cpp',
        '-o', NAIVE_EXE
    ], cwd=BASE_DIR, check=True)

def run_benchmark():
    # init log
    with open(LOG_FILE, 'w') as log:
        log.write('Witness Mismatch Log\n')
        log.write('====================\n\n')

    print('n    | Optimized(s) | Naive(s) | Match?')
    print('-----+--------------+----------+-------')

    for n in SIZES:
        # generate data; a[0]=0 placeholder, order[0]=0 placeholder
        a     = [0] + [random.randint(0,1) for _ in range(n-1)]
        b     = [random.randint(0,1) for _ in range(n)]
        order = [0] + random.sample(range(1, n), n-1)

        # build input
        inp  = f"{n}\n"
        inp += ' '.join(map(str, a)) + "\n"
        inp += ' '.join(map(str, b)) + "\n"
        inp += ' '.join(map(str, order)) + "\n"

        # run optimized and naive once for correctness
        proc_opt   = subprocess.run([OPT_EXE],   input=inp, text=True, capture_output=True)
        proc_naive = subprocess.run([NAIVE_EXE], input=inp, text=True, capture_output=True)

        out_opt   = list(map(int, proc_opt.stdout.strip().split()))
        out_naive = list(map(int, proc_naive.stdout.strip().split()))
        match     = (out_opt == out_naive)

        if not match:
            # compute true boolean convolution result c
            R = n + n - 1
            c_list = []
            for k in range(R):
                val = 0
                for j in range(max(0, k-(n-1)), min(n, k+1)):
                    if a[j] and b[k-j]:
                        val = 1
                        break
                c_list.append(val)

            # log mismatch details
            with open(LOG_FILE, 'a') as log:
                log.write(f'n = {n} mismatch detected\n')
                for idx, (xo, xn) in enumerate(zip(out_opt, out_naive)):
                    if xo != xn:
                        # convert lex-rank back to original index
                        opt_idx   = xo
                        naive_idx = xn
                        # values from a and b used in convolution
                        opt_b_idx   = idx - opt_idx
                        naive_b_idx = idx - naive_idx

                        if 0 <= opt_b_idx < len(b):
                            b_opt_val = b[opt_b_idx]
                        else:
                            b_opt_val = '<invalid>'

                        if 0 <= naive_b_idx < len(b):
                            b_naive_val = b[naive_b_idx]
                        else:
                            b_naive_val = '<invalid>'
                        log.write(
                            f'  idx={idx}: c={c_list[idx]}\n'
                            f'    optimized chose a[{opt_idx}]={a[opt_idx]}, '
                            f'b[{opt_b_idx}]={b_opt_val}\n'
                            f'    naive     chose a[{naive_idx}]={a[naive_idx]}, '
                            f'b[{naive_b_idx}]={b_naive_val}\n\n'
                        )
                        break
                log.write(f'Full c array           = {c_list}\n')
                log.write(f'Optimized witnesses    = {out_opt}\n')
                log.write(f'Naive witnesses        = {out_naive}\n')
                log.write(f'a vector               = {a}\n')
                log.write(f'b vector               = {b}\n')
                log.write(f'order permutation      = {order}\n\n')

        # benchmark optimized (wall-clock)
        t_opt = 0.0
        for _ in range(TRIALS):
            start = time.perf_counter()
            subprocess.run([OPT_EXE], input=inp, text=True, capture_output=True, check=True)
            t_opt += time.perf_counter() - start
        t_opt /= TRIALS
        opt_times.append(t_opt)

        # benchmark naive (wall-clock)
        t_naive = 0.0
        for _ in range(TRIALS):
            start = time.perf_counter()
            subprocess.run([NAIVE_EXE], input=inp, text=True, capture_output=True, check=True)
            t_naive += time.perf_counter() - start
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
    plt.legend(['Optimized','Naive'])
    plt.title('Minimum-Witness Boolean Convolution Benchmark')
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    random.seed(0)
    compile_tests()
    run_benchmark()
