#include <iostream>
#include <vector>
#include "algorithms.h"
#include "dp_structs.h"
#include <cmath>
using namespace std;

int main(){
    int n, u, t;
    // read number of item types, max weight per item, and max target weight
    cin >> n >> u >> t;
    cout << "item info inputted" << endl;

    // 1-indexed arrays
    vector<int> w(n+1), p(n+1), order(n+1);
    for(int i = 1; i <= n; i++){
        cin >> w[i] >> p[i];
        order[i] = i;                // identity lex order
    }
    cout << "arrays initialized" << endl;
    // 1) Kernel computation (Alg.2)
    vector<solution> sol(t+1, solution(n));    // size = t+1
    cout << "sol initialized" << endl;
    
    kernelComputation(n, u, w, p, order, t, sol);
    cout << "kernels computed" << endl;
    // for (int i = 0; i <= sol.size(); i++) {
    //     cout << "c = " << i << " v = " << sol[i].value << endl;
    // }

    cout << endl;
    // 2) Build support sets for each kernel i
    vector<vector<int>> supp(t+1);
    for(int i = 0; i <= t && i < (int)sol.size(); i++){
        // any j with sol[i].svec[j] > 0 is in the support
        for(int j = 1; j <= n; j++){
            if(sol[i].svec[j] > 0)
                supp[i].push_back(j);
        }
    }

    // 3) Propagate to fill all targets up to t (Alg.1)
    propagation(w, p, t, sol, supp);

    // 4) Output results: best profit for each c in [0..t]
    for(int cval = 0; cval <= t; cval++){
        if(cval >= (int)sol.size() || sol[cval].size == 0) {
            cout << "c = " << cval <<", v = -inf\n";
        } else {
            cout << "c = " << cval <<", v = " << sol[cval].value << "\n";
        }
    }

    return 0;
}
