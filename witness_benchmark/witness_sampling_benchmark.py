#!/usr/bin/env python3
import os, subprocess, random, time
import matplotlib.pyplot as plt

BASE     = os.path.dirname(os.path.realpath(__file__))
INCLUDE  = os.path.join(BASE, '..', 'include')
SOLVER   = os.path.join(BASE, 'optimized_sampling_test')
LOG_FILE = os.path.join(BASE, 'sampling_mismatches.log')

SIZES  = [256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072]
TRIALS = 3

def compile_solver():
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'optimized_sampling_test.cpp',
        os.path.join('..','src','convolution.cpp'),
        os.path.join('..','src','witness.cpp'),
        '-o', SOLVER
    ], cwd=BASE, check=True)

def verify(a, b, witness):
    n = len(a)
    R = 2*n - 1
    # boolean convolution c
    c = [0]*R
    for i in range(n):
        if not a[i]: continue
        for j in range(n):
            if b[j]:
                c[i+j] = 1

    for i in range(R):
        wi = witness[i]
        if c[i] == 0:
            if wi != -1:
                return False, f"i={i} c=0 but w={wi}"
        else:
            if not (0 <= wi < n):
                return False, f"i={i} c=1 but w={wi} OOB"
            if a[wi] != 1 or b[i-wi] != 1:
                return False, f"i={i} invalid a[{wi}]={a[wi]} b[{i-wi}]={b[i-wi]}"
    return True, ""

def run_benchmark():
    with open(LOG_FILE, 'w') as log:
        log.write("Sampling Mismatch Log\n\n")

    times = []
    print("   n    |  Time(s)  |  OK?")
    print("--------+-----------+------")

    for n in SIZES:
        a = [random.randint(0,1) for _ in range(n)]
        b = [random.randint(0,1) for _ in range(n)]

        inp = f"{n}\n"
        inp += ' '.join(map(str,a)) + "\n"
        inp += ' '.join(map(str,b)) + "\n"

        # time solver
        acc = 0.0
        for _ in range(TRIALS):
            t0 = time.perf_counter()
            proc = subprocess.run([SOLVER],
                                  input=inp, text=True,
                                  capture_output=True, check=True)
            acc += time.perf_counter() - t0
            out = proc.stdout.strip().split()
        t = acc / TRIALS
        witness = list(map(int, out))

        ok, msg = verify(a, b, witness)
        print(f"{n:<7}| {t:9.6f} | {'Y' if ok else 'N'}")
        times.append(t)

        if not ok:
            with open(LOG_FILE,'a') as log:
                log.write(f"n={n} FAIL: {msg}\n")
                log.write(f"a={a}\n b={b}\n witness={witness}\n\n")

    print(f"\nDetails logged to {LOG_FILE}")

    plt.plot(SIZES, times, marker='o')
    plt.xlabel('n')
    plt.ylabel('Sampling time (s)')
    plt.title('Randomized Witness Sampling')
    plt.tight_layout()
    plt.show()

if __name__=='__main__':
    random.seed(0)
    compile_solver()
    run_benchmark()
