#!/usr/bin/env python3
import os, subprocess, random, time, math
import matplotlib.pyplot as plt

BASE = os.path.dirname(os.path.realpath(__file__))
SRC_DIR = os.path.normpath(os.path.join(BASE, '..', 'src'))
INC_DIR = os.path.normpath(os.path.join(BASE, '..', 'include'))
SOLVER   = os.path.join(BASE, 'knapsack_k_witness_bench')
LOG_FILE = os.path.join(BASE, 'knapsack_mismatch.log')


SIZES     = [10,20,30,256,512,1024,2048,4096,8192,16384,32768]
TRIALS    = 1
MAX_VAL   = 1000

def convolution(a,b):
    R = len(a)+len(b)-1
    c = [0]*R
    for i,ai in enumerate(a):
        if ai:
            for j,bj in enumerate(b):
                if bj:
                    c[i+j]+=1
    return c

def validate(a,b,w,order,k,got):
    errs=[]
    c = convolution(a,b)
    R = len(c)
    for i in range(R):
        ws = got[i]
        exp = min(c[i],k)
        if len(ws)!=exp:
            errs.append((i,f"exp {exp} got {len(ws)}",ws))
            continue
        seen=set()
        for pos in ws:
            if pos<1 or pos>=len(a):
                errs.append((i,f"pos {pos} out of range",ws)); break
            if pos in seen:
                errs.append((i,f"dup {pos}",ws)); break
            seen.add(pos)
            idx = order[pos]
            j   = i - w[idx]
            if j<0 or j>=len(b):
                errs.append((i,f"j={j} bad",ws)); break
            if not (a[idx] and b[j]):
                errs.append((i,f"a[{idx}]={a[idx]},b[{j}]={b[j]}",ws)); break
    return not errs, errs

def compile_solver():
    print("Compiling solver...")
    subprocess.run([
        'g++-14', '-std=c++20', '-O2',
        '-I', INC_DIR,
        # solver entry
        os.path.join(BASE, 'knapsack_k_witness_solver.cpp'),
        # library implementations
        os.path.join(SRC_DIR, 'convolution.cpp'),
        os.path.join(SRC_DIR, 'witness.cpp'),
        os.path.join(SRC_DIR, 'peeling.cpp'),
        '-o', os.path.join(BASE, 'knapsack_k_witness_bench')
    ], check=True)


def gen_case(n):
    k     = 2*math.ceil(math.log2(n))
    order = list(range(n))
    rest  = order[1:]
    random.shuffle(rest)
    order[1:]=rest
    a = [0]*n
    for p in range(1,n):
        a[order[p]] = 1
    b = [random.randint(0,1) for _ in range(n)]
    w = [0]+random.sample(range(1,n),n-1)
    parts = [f"{n} {k}",
             " ".join(map(str,a)),
             " ".join(map(str,b)),
             " ".join(map(str,w)),
             " ".join(map(str,order))]
    return "\n".join(parts)+"\n", a,b,w,order,k

def run_one(n):
    inp,a,b,w,order,k = gen_case(n)
    t0 = time.perf_counter()
    proc = subprocess.run([SOLVER], input=inp, text=True,
                          capture_output=True, check=True)
    dt = time.perf_counter()-t0
    got=[]
    for L in proc.stdout.strip().splitlines():
        nums = list(map(int,L.split()))
        got.append(nums[1:1+nums[0]])
    ok=True
    if n<=MAX_VAL:
        ok,errs = validate(a,b,w,order,k,got)
        if not ok:
            with open(LOG_FILE,'a') as f:
                f.write(f"n={n}\n")
                for e in errs: f.write(f"{e}\n")
                f.write("\n")
    return dt,ok

if __name__=='__main__':
    random.seed(42)
    open(LOG_FILE,'w').write("Knapsack Witness Mismatches\n\n")
    compile_solver()
    print(f"{'n':>6} | {'time(s)':>8} | {'OK?':>3}")
    print("-"*25)
    results=[]
    for n in SIZES:
        ts,oks = [],[]
        for _ in range(TRIALS):
            t,ok = run_one(n)
            ts.append(t); oks.append(ok)
        avg = sum(ts)/len(ts)
        print(f"{n:6} | {avg:8.4f} | {('Y' if all(oks) else 'N'):>3}")
        results.append((n,avg))
    xs,ys = zip(*results)
    plt.plot(xs,ys,marker='o')
    plt.xlabel('n')
    plt.ylabel('avg time (s)')
    plt.title('k_find_witnesses_knapsack Benchmark')
    plt.grid(True)
    plt.show()
