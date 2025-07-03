#include <bits/stdc++.h>
#include "peeling.h"    // for k_find_witnesses_randomized
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    // input: n k
    // then a[0..n-1], b[0..n-1], w[0..n-1], order[0..n-1]
    cin >> n >> k;
    vector<int> a(n), b(n), w(n), order(n);
    for(int i = 0; i < n; i++) cin >> a[i];
    for(int i = 0; i < n; i++) cin >> b[i];
    for(int i = 0; i < n; i++) cin >> w[i];
    for(int i = 0; i < n; i++) cin >> order[i];

    // compute k witnesses
    auto witnesses = k_find_witnesses_knapsack(a, b, w, order, k);

    // print: for each sum i from 0 to 2n-2, first the count then the positions
    int R = (int)witnesses.size();
    for(int i = 0; i < R; i++){
        cout << witnesses[i].size();
        for(int idx : witnesses[i]){
            cout << " " << idx;
        }
        cout << "\n";
    }
    return 0;
}
