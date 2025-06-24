#include <bits/stdc++.h>
#include "algorithms.h"
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n,u,T;
    cin >> n >> u >> T;
    // 1-index everything
    vector<int> w(n+1), p(n+1, 1), order(n+1);
    for(int i=1;i<=n;i++){
        cin>>w[i]>>p[i]; 
        order[i] = i;
    }
    
    vector<solution> sol;
    kernelComputation_coinchange_simple(n, u, w, p, order, T, sol);
    propagation(w, p, T, sol, order);
    // sol[t].value is -k where k is #coins used (we added with -1)
    for(int t=0; t<=T; t++){
        if(sol[t].size == 0 && t) {
            cout << -1 << "\n";
        } else {
            cout << -sol[t].value << "\n";
        }
    }
    return 0;
}
