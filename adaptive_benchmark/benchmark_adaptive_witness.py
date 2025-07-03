#!/usr/bin/env python3
import os, subprocess, random, time, math
import matplotlib.pyplot as plt

BASE     = os.path.dirname(os.path.realpath(__file__))
SOLVER   = os.path.join(BASE, 'adaptive_min_witness_bench')
LOG_FILE = os.path.join(BASE, 'adaptive_mismatch.log')

SIZES    = [10,20,30,256,512,1024,2048,4096,8192,16384, 32768, 65536]
TRIALS   = 3
MAX_VAL  = 10000  # only validate for n ≤ this

def convolution(a,b):
    R = len(a)+len(b)-1
    c = [0]*R
    for i,av in enumerate(a):
        if av:
            for j,bv in enumerate(b):
                if bv: c[i+j]+=1
    return c

def validate(a, b, out):
    """
    a, b: input binary vectors of length n
    out:  solver’s list of length 2n-1, where out[j] is the claimed witness idx or -1
    """
    n = len(a)
    R = 2*n - 1
    c = convolution(a, b)
    errs = []

    for j, w in enumerate(out):
        # did we expect no witness here?
        if c[j] == 0:
            if w != -1:
                errs.append((j, f"expected -1, got {w}"))
            continue

        # we expected at least one witness
        # 1) is w in [0,n)?
        if w < 0 or w >= n:
            errs.append((j, f"witness {w} out of range"))
            continue

        # 2) compute the paired index
        jj = j - w
        if jj < 0 or jj >= n:
            errs.append((j, f"paired index j-w = {jj} out of range"))
            continue

        # 3) finally check that a[w] and b[jj] are both 1
        if not (a[w] and b[jj]):
            errs.append((j, f"a[{w}]={a[w]}, b[{jj}]={b[jj]} invalid"))
            continue

    return (len(errs) == 0), errs


def compile_solver():
    print("Compiling solver...")
    SRC_DIR = os.path.normpath(os.path.join(BASE,'..','src'))
    INC_DIR = os.path.normpath(os.path.join(BASE,'..','include'))
    subprocess.run([
      'g++-14','-std=c++20','-O2',
      '-I', INC_DIR,
      os.path.join(BASE,'adaptive_min_witness_solver.cpp'),
      os.path.join(SRC_DIR,'convolution.cpp'),
      os.path.join(SRC_DIR,'witness.cpp'),
      os.path.join(SRC_DIR,'algorithms.cpp'),
      os.path.join(SRC_DIR,'dp_structs.cpp'),
      os.path.join(SRC_DIR,'peeling.cpp'),
      os.path.join(SRC_DIR,'hitting_set.cpp'),
      '-o', SOLVER
    ], check=True)

def gen_case(n):
    p = 1
    a = [random.randint(0,1) for _ in range(n)]
    b = [random.randint(0,1) for _ in range(n)]
    # ensure at least one 1 in each
    if not any(a): a[random.randrange(n)] = 1
    if not any(b): b[random.randrange(n)] = 1
    w = list(range(n)); random.shuffle(w)
    order = list(range(n)); random.shuffle(order)
    inp = f"{p} {n}\n"
    inp += " ".join(map(str,a))+"\n"
    inp += " ".join(map(str,b))+"\n"
    inp += " ".join(map(str,w))+"\n"
    inp += " ".join(map(str,order))+"\n"
    return inp,a,b

def run_one(n):
    inp,a,b = gen_case(n)
    t0 = time.perf_counter()
    proc = subprocess.run([SOLVER], input=inp, text=True,
                          capture_output=True, check=True)
    dt = time.perf_counter()-t0
    out = list(map(int, proc.stdout.strip().split()))
    ok, errs = True, []
    if n <= MAX_VAL:
        ok, errs = validate(a,b,out)
        if not ok:
            with open(LOG_FILE,'a') as f:
                f.write(f"n={n}\n")
                for e in errs: f.write(f"{e}\n")
                f.write("\n")
    return dt, ok

if __name__=='__main__':
    random.seed(0)
    open(LOG_FILE,'w').write("Adaptive Min-Witness Mismatches\n\n")
    compile_solver()
    print(f"{'n':>6} | {'time(s)':>8} | {'OK?':>3}")
    print("-"*25)
    results = []
    for n in SIZES:
        ts, oks = [], []
        for _ in range(TRIALS):
            t,ok = run_one(n)
            ts.append(t); oks.append(ok)
        avg = sum(ts)/len(ts)
        print(f"{n:6} | {avg:8.4f} | {('Y' if all(oks) else 'N'):>3}")
        results.append((n,avg))
    xs, ys = zip(*results)
    plt.plot(xs, ys, marker='o')
    plt.xlabel('n')
    plt.ylabel('avg time (s)')
    plt.title('Adaptive Min-Witness Benchmark')
    plt.grid(True)
    plt.show()
