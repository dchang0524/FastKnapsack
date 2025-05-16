// src/debug_kernel_to_file.cpp

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <numeric>
#include "convolution.h"   // for maxPlusCnv
#include "dp_structs.h"    // for solution
using namespace std;

static constexpr int NEG_INF = -1000000000;

int main(){
    //
    // === Bigger test‐case: n=15, u=20, t=150 ===
    //
    int n = 15;
    int u = 20;
    int t = 150;

    // 1‐indexed weights & profits (all weights distinct)
    vector<int> w;
    vector<int> p;
    vector<int> order;

    w     = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15};
    p     = {0,  3,  7,  6,  2, 10,  8,  1,  9,  5,  4, 11, 12, 14, 13, 16};
    order = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15};

    // compute k and allocate
    int k = static_cast<int>(floor(2.0*log2(u) + 1.0));
    int KU = k*u + 1;

    // our DP structures
    vector<solution> sol(max(KU, t+1), solution());
    vector<int> v    (KU, NEG_INF),
                f    (u+1, NEG_INF);

    // base case
    v[0] = 0;
    for(int i=1;i<=n;i++){
        f[ w[order[i]] ] = p[order[i]];
    }

    // prepare f_w for witness‐encoding
    vector<int> f_w(u+1, NEG_INF);
    for(int i=1;i<=n;i++){
        int wi = w[order[i]];
        f_w[wi] = (n+1)*f[wi] - i;
    }

    // temps
    vector<int> vPrime, v_w(KU), minW;

    // open our debug file
    ofstream out("kernel_debug.txt");
    out << "# debug of kernelComputation (n="<<n<<", u="<<u<<", t="<<t<<")\n";
    out << "# k="<<k<<"  KU="<<KU<<"\n\n";

    // main k–iteration loop
    for(int iter=1; iter<=k; ++iter){
        out << "=== Iteration " << iter << " ===\n";

        // 1) max‐plus convolve
        vPrime = maxPlusCnv(v, f);
        out << "vPrime:";
        for(int i=0;i<KU;i++) out << " " << vPrime[i];
        out << "\n";

        // 2) build scaled v_w
        fill(v_w.begin(), v_w.end(), NEG_INF);
        for(int i=0;i<KU;i++){
            if(v[i] > NEG_INF) 
                v_w[i] = (n+1)*v[i];
        }
        out << "v_w   :";
        for(int i=0;i<KU;i++) out << " " << v_w[i];
        out << "\n";

        // 3) encode witnesses
        minW = maxPlusCnv(v_w, f_w);
        out << "minW  :";
        for(int i=0;i<KU;i++) out << " " << ((n+1) - minW[i]%(n+1)) % (n+1);
        out << "\n";

        // 4) apply updates
        for(int c=1; c<KU; ++c){
            if(vPrime[c] > NEG_INF){
                // accept
                v[c] = vPrime[c];
                // decode the “-i”
                int negI     = minW[c] % (n+1);
                int witnessI = ((n+1) - negI) % (n+1);
                if(witnessI == 0) continue; 
                int coinIdx = order[witnessI];
                int prev    = c - w[coinIdx];
                if(prev >= 0){
                    sol[prev].copy(sol[c]);
                }
                sol[c].addCoin(coinIdx, w[coinIdx], p[coinIdx]);
            }
        }

        // dump all non‐empty solutions this iter
        out << "sol[]:";
        for(int c=0;c<KU;c++){
            if(sol[c].size>0){
                out 
                  << " ["<<c<<":sz="<<sol[c].size
                  <<",v="<<sol[c].value
                  <<",w="<<sol[c].weight<<"]";
            }
        }
        out << "\n";
        
        //output max support size
        int maxSize = 0;
        int maxInd = -1;
        for (int i = 0; i <= t && i < (int)sol.size(); i++) {
            //count the number of distinct coins in the solution
            int curr = 0;
            for (const auto& entry : sol[i].svec) {
                if (entry.second > 0) {
                    curr++;
                }
            }
            maxSize = max(maxSize, curr);
            if (curr == maxSize) {
                maxInd = i;
            }
        }
        out << "Max kernel support size: " << maxSize << "\n";
        out << "Max kernel index: " << maxInd << "\n";
        //print the entries of the solution with the maximum support size
        out << "Entries in solution with max support size (index " << maxInd << "): ";
        for (const auto& entry : sol[maxInd].svec) {
            out << "(" << entry.first << ", " << entry.second << ") "; // Format pair as (key, value)
        }
        out << "\n\n";
    }

    out.close();
    cout << "Debug written to kernel_debug.txt\n";
    return 0;
}
