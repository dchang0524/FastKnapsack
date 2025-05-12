#include <iostream>
#include <vector>
#include "algorithms.h"
#include "dp_structs.h"
using namespace std;

int main(){
    int n, u, t;
    // read number of item types, max weight per item, and max target weight
    cin >> n >> u >> t;

    // 1-indexed arrays
    vector<int> w(n+1), p(n+1), order(n+1);
    for(int i = 1; i <= n; i++){
        cin >> w[i] >> p[i];
        order[i] = i;                // identity lex order
    }

    // 1) Kernel computation (Alg.2)
    vector<solution> sol;
    kernelComputation(n, u, w, p, order, t, sol);

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
            cout << "-inf\n";
        } else {
            cout << sol[cval].value << "\n";
        }
    }

    return 0;
}
