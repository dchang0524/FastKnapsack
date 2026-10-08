#include "algorithms.h"
#include "witness.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <numeric>

int main() {
    {
        solution early, late, equal;
        early.addCoin(2, 2, 1);
        late.addCoin(5, 5, 1);
        equal.addCoin(2, 2, 1);
        assert(early.lexCmp(late));
        assert(!late.lexCmp(early));
        assert(!early.lexCmp(equal));
        early.addCoin(5, 5, 1);
        assert(early.lexCmp(equal));
    }
    mt19937 generator(20261007);
    for (int trial = 0; trial < 60; ++trial) {
        const int n = 1 + generator() % 7;
        const int u = 2 + generator() % 14;
        const int t = 2 * u * u + 3;
        vector<int> weight(n + 1), profit(n + 1), order(n + 1);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin() + 1, order.end(), generator);
        for (int coin = 1; coin <= n; ++coin) {
            weight[coin] = 1 + generator() % u;
            profit[coin] = static_cast<int>(generator() % 25) - 12;
        }
        vector<ll> expected(t + 1, NEG_INF_LL);
        vector<vector<int>> expected_counts(t + 1, vector<int>(n + 1));
        vector<int> rank(n + 1);
        for (int position = 1; position <= n; ++position)
            rank[order[position]] = position;
        expected[0] = 0;
        for (int sum = 1; sum <= t; ++sum)
            for (int coin = 1; coin <= n; ++coin)
                if (weight[coin] <= sum &&
                    expected[sum - weight[coin]] != NEG_INF_LL) {
                    auto counts = expected_counts[sum - weight[coin]];
                    ++counts[rank[coin]];
                    const ll value = expected[sum - weight[coin]] + profit[coin];
                    if (value > expected[sum] ||
                        (value == expected[sum] &&
                         lexicographical_compare(counts.begin() + 1, counts.end(),
                             expected_counts[sum].begin() + 1,
                             expected_counts[sum].end(), greater<int>()))) {
                        expected[sum] = value;
                        expected_counts[sum] = std::move(counts);
                    }
                }
        vector<solution> found;
        int backend_calls = 0;
        MaxPlusBackend backend = [&](const vector<ll>& left,
                                     const vector<ll>& right) {
            ++backend_calls;
            return maxPlusCnv(left, right);
        };
        kernelComputation_knapsack(n, u, weight, profit, order, t, found,
                                   backend);
        assert(backend_calls == static_cast<int>(floor(2.0 * log2(u) + 1.0)));
        propagation(weight, profit, t, found, order);
        for (int sum = 1; sum <= t; ++sum) {
            if (expected[sum] == NEG_INF_LL) {
                assert(found[sum].size == 0);
                continue;
            }
            assert(found[sum].value == expected[sum]);
            assert(found[sum].weight == sum);
            ll recomputed_weight = 0, recomputed_profit = 0, item_count = 0;
            for (auto [rank, count] : found[sum].svec) {
                assert(rank >= 1 && rank <= n && count > 0);
                recomputed_weight += count * weight[order[rank]];
                recomputed_profit += count * profit[order[rank]];
                item_count += count;
            }
            assert(recomputed_weight == sum &&
                   recomputed_profit == expected[sum] &&
                   item_count == found[sum].size);
            for (int position = 1; position <= n; ++position) {
                const auto it = found[sum].svec.find(position);
                assert((it == found[sum].svec.end() ? 0 : it->second) ==
                       expected_counts[sum][position]);
            }
        }
    }

    for (int trial = 0; trial < 24; ++trial) {
        const int n = 1 + generator() % 8;
        const int u = 2 + generator() % 12;
        const int t = 5 * u;
        vector<int> weight(n + 1), profit(n + 1, -1), order(n + 1);
        iota(order.begin(), order.end(), 0);
        for (int coin = 1; coin <= n; ++coin)
            weight[coin] = 1 + generator() % u;
        vector<int> expected(t + 1, 1000000);
        expected[0] = 0;
        for (int sum = 1; sum <= t; ++sum)
            for (int coin = 1; coin <= n; ++coin)
                if (weight[coin] <= sum)
                    expected[sum] = min(expected[sum],
                        expected[sum - weight[coin]] + 1);
        for (int method = 0; method < 3; ++method) {
            auto actual_order = order;
            vector<solution> found;
            if (method == 2)
                kernelComputation_coinchange_optimized(n, u, weight, profit,
                    actual_order, t, found);
            else if (method == 1)
                kernelComputation_coinchange_randomized(n, u, weight, profit,
                    actual_order, t, found);
            else
                kernelComputation_coinchange_simple(n, u, weight, profit,
                    actual_order, t, found);
            propagation(weight, profit, t, found, actual_order);
            for (int sum = 1; sum <= t; ++sum)
                if (expected[sum] == 1000000)
                    assert(found[sum].size == 0);
                else
                    assert(found[sum].size == expected[sum] &&
                           found[sum].weight == sum &&
                           found[sum].value == -expected[sum]);
        }
    }

    for (int trial = 0; trial < 20; ++trial) {
        const int n = 2 + generator() % 8, u = 4 + generator() % 18;
        vector<int> first(u + 1), second(2 * u + 1),
                    weights(n + 1), order(n + 1);
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin() + 1, order.end(), generator);
        for (int coin = 1; coin <= n; ++coin) {
            weights[coin] = 1 + generator() % u;
            first[weights[coin]] = 1;
        }
        for (int& bit : second) bit = generator() % 2;
        auto fixed = minimum_witness_boolCnv_ordered(first, second,
                                                      weights, order);
        auto random_order = minimum_witness_random(first, second,
                                                  weights, order);
        for (int sum = 0; sum < static_cast<int>(fixed.size()); ++sum) {
            int expected = -1;
            for (int rank = 1; rank <= n; ++rank) {
                const int weight = weights[order[rank]];
                if (sum >= weight && sum - weight < static_cast<int>(second.size()) &&
                    second[sum - weight]) {
                    expected = rank;
                    break;
                }
            }
            assert(fixed[sum] == expected);
            assert(random_order[sum] == expected);
        }
    }

    for (int trial = 0; trial < 35; ++trial) {
        const int n = 1 + generator() % 24;
        const int m = 1 + generator() % 24;
        const int k = 1 + generator() % 8;
        vector<int> a(n), b(m), order(n);
        for (int& bit : a) bit = generator() % 2;
        for (int& bit : b) bit = generator() % 2;
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), generator);
        auto found = k_minimum_witnesses_boolCnv_ordered(a, b, order, k);
        for (int sum = 0; sum < n + m - 1; ++sum) {
            vector<int> expected;
            for (int position : order)
                if (sum >= position && sum - position < m &&
                    a[position] && b[sum - position])
                    expected.push_back(position);
            expected.resize(min(k, static_cast<int>(expected.size())));
            assert(found[sum] == expected);
        }
    }

    vector<int> a{1, 0, 1, 1, 0, 1}, b{1, 1, 0, 1};
    for (int iteration = 0; iteration < 20; ++iteration) {
        auto sampled = randomized_witness_sampling(a, b);
        for (int sum = 0; sum < static_cast<int>(sampled.size()); ++sum) {
            int count = 0;
            for (int position = 0; position < static_cast<int>(a.size()); ++position)
                if (sum - position >= 0 && sum - position < static_cast<int>(b.size()) &&
                    a[position] && b[sum - position]) ++count;
            assert((count == 0 && sampled[sum] == -1) ||
                   (count > 0 && sampled[sum] >= 0 &&
                    a[sampled[sum]] && b[sum - sampled[sum]]));
        }
    }
}
