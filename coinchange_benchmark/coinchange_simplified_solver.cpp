#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include "algorithms.h"
#include "dp_structs.h"
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, u, T;
    if(!(cin >> n >> u >> T)) return 0;

    // Read coins (1-indexed) and set up identity order
    vector<int> w(n+1), p(n+1), order(n+1);
    for(int i = 1; i <= n; i++){
        cin >> w[i] >> p[i];
        order[i] = i;
    }

    vector<solution> sol;
    auto total_start = chrono::high_resolution_clock::now();

    // 1) Kernel computation
    auto k0 = chrono::high_resolution_clock::now();
    kernelComputation_coinchange_simple(n, u, w, p, order, T, sol);
    auto k1 = chrono::high_resolution_clock::now();
    double ktime = chrono::duration<double>(k1 - k0).count();
    cerr << "Kernel computation took " << ktime << " s\n";

    // 2) Max support size
    int maxsup = 0, maxInd = -1;
    for(int i = 0; i <= T && i < (int)sol.size(); ++i){
        int sz = sol[i].svec.size();
        if(sz > maxsup){
            maxsup = sz;
            maxInd = i;
        }
    }
    if(maxInd != -1){
        cerr << "Max kernel support size: " << maxsup << "\n";
        cerr << "Entries in solution with max support size (index " << maxInd << "): ";
        for(auto &e : sol[maxInd].svec){
            cerr << "(" << e.first << ", " << e.second << ") ";
        }
        cerr << "\n";
    }

    // 3) Witness propagation
    auto p0 = chrono::high_resolution_clock::now();
    propagation(w, p, T, sol, order);
    auto p1 = chrono::high_resolution_clock::now();
    double ptime = chrono::duration<double>(p1 - p0).count();
    cerr << "Witness propagation took " << ptime << " s\n";

    // 4) Total time
    auto total_end = chrono::high_resolution_clock::now();
    double tot = chrono::duration<double>(total_end - total_start).count();
    cerr << "Total elapsed time: " << tot << " s\n";

    // 5) Emit the coin‐change result (# coins or -1)
    for(int t = 0; t <= T; t++){
        if(sol[t].size == 0 && t != 0) 
            cout << -1 << "\n";
        else 
            cout << -sol[t].value << "\n";
    }
    return 0;
}
