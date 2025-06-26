#include <bits/stdc++.h>
#include "witness.h"
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(n), b(n);
    for (int i = 0; i < n; i++) cin >> a[i];
    for (int i = 0; i < n; i++) cin >> b[i];

    auto t0 = chrono::high_resolution_clock::now();
    vector<int> witness = randomized_witness_sampling(a, b);
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cerr << "Sampling took " << secs << " s\n";

    // output witness
    for (int x : witness) cout << x << " ";
    cout << "\n";
    return 0;
}