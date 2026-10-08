#include "algorithms.h"
#include "peeling.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <numeric>

int main() {
    mt19937 rng(20261006);
    for (int trial = 0; trial < 100; ++trial) {
        int n = 1 + rng() % 22, m = 1 + rng() % 22;
        int k = 1 + rng() % 10;
        vector<int> a(n), b(m);
        for (int& x : a) x = rng() % 2;
        for (int& x : b) x = rng() % 2;
        for (auto method : {false, true}) {
            auto found = method ? k_witnesses_boolean_randomized(a, b, k)
                                : k_witnesses_boolean_deterministic(a, b, k);
            assert(static_cast<int>(found.size()) == n + m - 1);
            for (int s = 0; s < n + m - 1; ++s) {
                vector<int> expected;
                for (int i = 0; i < n; ++i)
                    if (s - i >= 0 && s - i < m && a[i] && b[s - i])
                        expected.push_back(i);
                auto actual = found[s];
                sort(actual.begin(), actual.end());
                assert(unique(actual.begin(), actual.end()) == actual.end());
                assert(static_cast<int>(actual.size()) == min(k, static_cast<int>(expected.size())));
                for (int i : actual)
                    assert(binary_search(expected.begin(), expected.end(), i));
            }
        }
    }

    for (int trial = 0; trial < 60; ++trial) {
        int n = 2 + rng() % 18, p = 1 + rng() % 3;
        vector<vector<int>> a(p, vector<int>(n)), b(p, vector<int>(n));
        for (auto& row : a) for (int& x : row) x = rng() % 2;
        for (auto& row : b) for (int& x : row) x = rng() % 2;
        for (bool randomized : {false, true}) {
            vector<int> order(n);
            iota(order.begin(), order.end(), 0);
            shuffle(order.begin(), order.end(), rng);
            auto witnesses = adaptiveMinWitness(a, b, order, randomized);
            vector<int> rank(n);
            for (int i = 0; i < n; ++i) rank[order[i]] = i;
            for (int row = 0; row < p; ++row)
                for (int s = 0; s < 2*n - 1; ++s) {
                    int expected = -1;
                    for (int i = 0; i < n; ++i)
                        if (s-i >= 0 && s-i < n && a[row][i] && b[row][s-i] &&
                            (expected < 0 || rank[i] < rank[expected]))
                            expected = i;
                    assert(witnesses[row][s] == expected);
                }
        }
    }

    // Dense outputs force the hitting-set branch of Algorithm 4.
    {
        const int n = 96;
        vector<vector<int>> a(1, vector<int>(n, 1));
        vector<vector<int>> b(1, vector<int>(n, 1));
        for (bool randomized : {false, true}) {
            vector<int> order(n);
            iota(order.begin(), order.end(), 0);
            auto witnesses = adaptiveMinWitness(a, b, order, randomized);
            vector<int> rank(n);
            for (int i = 0; i < n; ++i) rank[order[i]] = i;
            for (int s = 0; s < 2*n-1; ++s) {
                int expected = -1;
                for (int i = 0; i < n; ++i)
                    if (s-i >= 0 && s-i < n &&
                        (expected < 0 || rank[i] < rank[expected]))
                        expected = i;
                assert(witnesses[0][s] == expected);
            }
        }
    }

    for (int trial = 0; trial < 50; ++trial) {
        int u = 2 + rng() % 28, n = 1 + rng() % 12, t = 3*u;
        vector<int> w(n+1), p(n+1,-1), order(n+1);
        for (int i = 1; i <= n; ++i) {
            w[i] = 1 + rng() % u;
            order[i] = i;
        }
        vector<int> dp(t+1, 1000000);
        dp[0] = 0;
        for (int s = 1; s <= t; ++s) {
            for (int i = 1; i <= n; ++i) if (s >= w[i])
                dp[s] = min(dp[s], dp[s-w[i]] + 1);
        }
        for (bool randomized : {false, true}) {
            vector<int> current_order = order;
            vector<solution> sol;
            kernelComputation_coinchange(n, u, w, p, current_order, t, sol,
                                         randomized);
            propagation(w, p, t, sol, current_order);
            for (int s = 1; s <= t; ++s) {
                bool correct = (dp[s] == 1000000 && sol[s].size == 0) ||
                    (dp[s] < 1000000 && sol[s].size == dp[s] &&
                     sol[s].weight == s && sol[s].value == -dp[s]);
                if (!correct) {
                    cerr << "trial=" << trial << " sum=" << s << " u=" << u
                         << " randomized=" << randomized
                         << " dp=" << dp[s] << " size=" << sol[s].size
                         << " weight=" << sol[s].weight
                         << " value=" << sol[s].value << " weights:";
                    for (int i = 1; i <= n; ++i) cerr << ' ' << w[i];
                    cerr << '\n';
                }
                assert(correct);
            }
        }
    }

    string text = "101101", pattern = "10111";
    auto aligned = k_reconstruct_randomized(text, pattern, 2);
    auto aligned_deterministic = k_reconstruct_deterministic(text, pattern, 2);
    assert(aligned_deterministic == aligned);
    for (size_t start = 0; start < aligned.size(); ++start) {
        vector<int> expected;
        for (size_t j = 0; j < pattern.size(); ++j)
            if (text[start+j] == '1' && pattern[j] == '1')
                expected.push_back(static_cast<int>(j));
        assert(aligned[start].size() == min<size_t>(2, expected.size()));
        for (int position : aligned[start])
            assert(binary_search(expected.begin(), expected.end(), position));
    }

    // A long text is partitioned into overlapping windows of length < 2m.
    {
        string long_text(173, '0'), short_pattern = "1011011";
        for (size_t i = 0; i < long_text.size(); ++i)
            long_text[i] = (i % 4 != 0) ? '1' : '0';
        auto found = k_reconstruct_randomized(long_text, short_pattern, 3);
        auto deterministic = k_reconstruct_deterministic(long_text,
                                                          short_pattern, 3);
        assert(found == deterministic);
        for (size_t start = 0; start < found.size(); ++start) {
            vector<int> expected;
            for (size_t j = 0; j < short_pattern.size(); ++j)
                if (short_pattern[j] == '1' && long_text[start + j] == '1')
                    expected.push_back(static_cast<int>(j));
            assert(found[start].size() == min<size_t>(3, expected.size()));
            for (int position : found[start])
                assert(binary_search(expected.begin(), expected.end(),
                                     position));
        }
    }

    // The legacy coin API uses coin IDs, weights, and 1-based order ranks.
    {
        const int n = 18, k = 5;
        vector<int> a(n, 1), b(n), w(n), order(n);
        a[0] = 0;
        for (int i = 1; i < n; ++i) {
            b[i] = i % 3 != 0;
            w[i] = i;
            order[i] = i;
        }
        shuffle(order.begin() + 1, order.end(), rng);
        auto found = randomized_k_witness(a, b, k, w, order);
        auto peeled = k_find_witnesses_knapsack(a, b, order, w, k);
        for (const auto* result : {&found, &peeled})
            for (int s = 0; s < 2*n-1; ++s) {
                int count = 0;
                for (int coin = 1; coin < n; ++coin)
                    if (s-w[coin] >= 0 && s-w[coin] < n &&
                        a[coin] && b[s-w[coin]]) ++count;
                assert(static_cast<int>((*result)[s].size()) == min(k, count));
                for (int rank : (*result)[s]) {
                    assert(rank > 0 && rank < n);
                    int coin = order[rank];
                    assert(s-w[coin] >= 0 && s-w[coin] < n && b[s-w[coin]]);
                }
            }
    }
}
