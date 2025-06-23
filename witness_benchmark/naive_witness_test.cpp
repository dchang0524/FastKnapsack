#include <bits/stdc++.h>
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

    vector<int> inv(n);
    for (int i = 0; i < n; i++) inv[order[i]] = i;

    int m = 2*n - 1;
    vector<int> res(m, -1);
    for (int k = 0; k < m; k++) {
        int best = -1;
        for (int i = 0; i < n; i++) {
            int j = k - i;
            if (j < 0 || j >= n) continue;
            if (a[i] && b[j]) {
                int idx = inv[i];
                if (best == -1 || idx < best) best = idx;
            }
        }
        res[k] = best;
    }

    auto start = chrono::high_resolution_clock::now();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    cerr << "Naive witness took " << diff.count() << " s\n";

    for (int x : res) cout << x << ' ';
    cout << '\n';
    return 0;
}