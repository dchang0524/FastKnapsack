#include <bits/stdc++.h>
#include "witness.h"
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(n), b(n), order(n);
    for (int i = 0; i < n; i++) cin >> a[i];
    for (int i = 0; i < n; i++) cin >> b[i];
    for (int i = 0; i < n; i++) cin >> order[i];

    auto start = chrono::high_resolution_clock::now();
    auto res = minimum_witness_boolCnv_ordered(a, b, order);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cerr << "Optimized witness took " << diff.count() << " s\n";

    for (int x : res) cout << x << ' ';
    cout << '\n';
    return 0;
}