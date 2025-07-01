#!/usr/bin/env python3
import os, subprocess, random, time
import matplotlib.pyplot as plt

BASE     = os.path.dirname(os.path.realpath(__file__))
INCLUDE  = os.path.join(BASE, '..', 'include')
SOLVER   = os.path.join(BASE, 'randomized_min_witness_test')
LOG_FILE = os.path.join(BASE, 'random_witness_mismatches.log')

SIZES  = [256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
TRIALS = 3

def compile_solver():
    subprocess.run([
        'g++-14','-std=c++20','-O2',
        '-I', INCLUDE,
        'randomized_min_witness_test.cpp',
        os.path.join('..','src','convolution.cpp'),
        os.path.join('..','src','witness.cpp'),
        '-o', SOLVER
    ], cwd=BASE, check=True)

def brute_min_witness(a, b, w, order):
    n = len(a)
    R = n + n - 1
    c = [0]*R
    for i in range(n):
        if a[i]:
            for j in range(n):
                if b[j]:
                    c[i+j] = 1
    inv = { order[pos]: pos for pos in range(n) }
    W = [-1]*R
    for i in range(R):
        if c[i]:
            for pos in range(n):
                coin = order[pos]
                j = i - w[coin]
                if 0 <= j < n and a[coin] and b[j]:
                    W[i] = pos
                    break
    return c, W

def verify(a, b, w, order, witness):
    c, Wtrue = brute_min_witness(a,b,w,order)
    for i,(got,exp) in enumerate(zip(witness, Wtrue)):
        if got != exp:
            return False, i, got, exp, c[i], Wtrue
    return True, None, None, None, None, Wtrue

def run_benchmark():
    with open(LOG_FILE,'w') as f:
        f.write("RandomizedMinWitness Mismatch Log\n\n")

    times = []
    print("   n    |   Time(s)   |  OK?")
    print("--------+-------------+------")

    for n in SIZES:
        # generate inputs
        w = random.sample(range(1, n), n-1)
        w = [0] + w
        a = [1 if i in w[1:] else 0 for i in range(n)]
        b = [random.randint(0,1) for _ in range(n)]
        perm = random.sample(range(1, n), n-1)
        order = [0] + perm

        inp  = f"{n}\n"
        inp += ' '.join(map(str,a))     + "\n"
        inp += ' '.join(map(str,b))     + "\n"
        inp += ' '.join(map(str,w))     + "\n"
        inp += ' '.join(map(str,order)) + "\n"

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
        witness = list(map(int,out))

        # verify
        ok, idx, got, exp, ci, Wtrue = verify(a,b,w,order,witness)
        print(f"{n:<7}| {t:11.6f} | {'Y' if ok else 'N'}")
        times.append(t)

        if not ok:
            with open(LOG_FILE,'a') as f:
                f.write(f"\nn={n} mismatch at i={idx}\n")
                f.write(f"  convolution c[{idx}] = {ci}\n")
                f.write(f"  got witness      = {got}\n")
                f.write(f"  expected witness = {exp}\n")
                f.write(f"\nFull witness arrays:\n")
                f.write(f"  actual   = {witness}\n")
                f.write(f"  expected = {Wtrue}\n")
                f.write(f"\nInputs:\n")
                f.write(f"  a     = {a}\n")
                f.write(f"  b     = {b}\n")
                f.write(f"  w     = {w}\n")
                f.write(f"  order = {order}\n\n")

    print(f"\nDetails logged to {LOG_FILE}")

    plt.plot(SIZES, times, marker='o')
    plt.xlabel('n')
    plt.ylabel('Time (s)')
    plt.title('RandomizedMinWitness vs n')
    plt.tight_layout()
    plt.show()

if __name__=='__main__':
    random.seed(0)
    compile_solver()
    run_benchmark()
