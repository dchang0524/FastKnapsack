#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <utility>
#include "algorithms.h"
#include "dp_structs.h"
using namespace std;

int main(){
    int n, u, t;
    // read number of item types, max weight per item, and max target weight
    if(!(cin >> n >> u >> t)) return 0;

    // record overall start
    auto total_start = chrono::high_resolution_clock::now();

    // 1-indexed arrays
    vector<int> w(n+1), p(n+1), order(n+1);
    for(int i = 1; i <= n; i++){
        cin >> w[i] >> p[i];
        order[i] = i;                // identity lex order
    }

    // 1) Kernel computation (Alg.2)
    vector<solution> sol;           // will hold sol[0..max(k·u, t)]
    {
        auto t0 = chrono::high_resolution_clock::now();
        kernelComputation_knapsack(n, u, w, p, order, t, sol);
        auto t1 = chrono::high_resolution_clock::now();
        double secs = chrono::duration<double>(t1 - t0).count();
        cerr << "Kernel computation took " << secs << " s\n";
    }

    // compute max support size over the kernels
    int maxSize = 0;
    int maxInd = -1;
    for (int i = 0; i <= t && i < (int)sol.size(); i++) {
        maxSize = max(maxSize, static_cast<int>(sol[i].svec.size()));
        maxInd = i;
    }
    // Print the entries of the solution with the maximum support size
    if (maxInd != -1) {
        cerr << "Max kernel support size: " << maxSize << "\n";
        cerr << "Entries in solution with max support size (index " << maxInd << "): ";
        for (const auto& entry : sol[maxInd].svec) {
            cerr << "(" << entry.first << ", " << entry.second << ") "; // Format pair as (key, value)
        }
        cerr << "\n";
    }
   

    // 2) Witness‐propagation (Alg.1)
    {
        auto t0 = chrono::high_resolution_clock::now();
        propagation(w, p, t, sol);
        auto t1 = chrono::high_resolution_clock::now();
        double secs = chrono::duration<double>(t1 - t0).count();
        cerr << "Witness propagation took " << secs << " s\n";
    }

    // record and print total time
    {
        auto total_end = chrono::high_resolution_clock::now();
        double total_secs = chrono::duration<double>(total_end - total_start).count();
        cerr << "Total elapsed time: " << total_secs << " s\n";
    }

    // 3) Output results: best profit for each c in [0..t]
    //    Only these go to stdout.
    for(int cval = 0; cval <= t; cval++){
        if(sol[cval].size == 0 && cval != 0) {
            cout << -1000000000 << "\n";
        } else {
            cout << sol[cval].value << "\n";
        }
    }

    return 0;
}
