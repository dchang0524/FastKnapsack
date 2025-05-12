#include "convolution.h"
#include <vector>
#include <climits>
using namespace std;

// Naïve Boolean‐count convolution
vector<int> boolCountConv(const vector<int>& a, const vector<int>& b) {
    int n = (int)a.size(), m = (int)b.size(), N = n + m - 1;
    vector<int> c(N, 0);
    for (int i = 0; i < n; i++) {
        if (!a[i]) continue;
        for (int j = 0; j < m; j++) {
            if (b[j]) c[i + j]++;
        }
    }
    return c;
}

// Naïve integer‐sum convolution
vector<long long> intSumConv(
    const vector<long long>& aSum,
    const vector<int>& b
) {
    int n = (int)aSum.size(), m = (int)b.size(), N = n + m - 1;
    vector<long long> s(N, 0);
    for (int i = 0; i < n; i++) {
        if (aSum[i] == 0) continue;
        for (int j = 0; j < m; j++) {
            if (b[j]) s[i + j] += aSum[i];
        }
    }
    return s;
}

// (max, +) convolution
vector<int> maxPlusCnv(const vector<int>& a, const vector<int>& b) {
    int n = (int)a.size(), m = (int)b.size(), N = n + m - 1;
    vector<int> c(N, INT_MIN);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            c[i + j] = max(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

// Boolean OR‐convolution
vector<int> boolCnv(const vector<int>& a, const vector<int>& b) {
    int n = (int)a.size(), m = (int)b.size(), N = n + m - 1;
    vector<int> c(N, 0);
    for (int i = 0; i < n; i++) {
        if (!a[i]) continue;
        for (int j = 0; j < m; j++) {
            if (b[j]) c[i + j] = 1;
        }
    }
    return c;
}
