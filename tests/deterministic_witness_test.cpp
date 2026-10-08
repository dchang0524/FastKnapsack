#include "algorithms.h"
#include "witness.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace {
void check(const std::vector<int>& a, const std::vector<int>& b, int k,
           const std::vector<unsigned char>* requested = nullptr,
           bool randomized = false) {
    auto got = randomized
        ? k_witnesses_boolean_randomized(a, b, k, requested)
        : k_witnesses_boolean_deterministic(a, b, k, requested);
    assert(got.size() == a.size() + b.size() - 1);
    for (size_t sum = 0; sum < got.size(); ++sum) {
        int expected = 0;
        for (size_t i = 0; i < a.size(); ++i)
            if (i <= sum && sum - i < b.size() && a[i] && b[sum - i])
                ++expected;
        auto positions = got[sum];
        std::sort(positions.begin(), positions.end());
        assert(std::unique(positions.begin(), positions.end()) == positions.end());
        assert(positions.size() == static_cast<size_t>(
            requested && !(*requested)[sum] ? 0 : std::min(k, expected)));
        for (int position : positions)
            assert(position >= 0 && position < static_cast<int>(a.size()) &&
                   a[position] && sum >= static_cast<size_t>(position) &&
                   sum - position < b.size() && b[sum - position]);
    }
}
}

int main() {
    // The product exceeds the dense direct-enumeration threshold. The first
    // check exercises the explicit almost-independent family and exact NTT.
    std::vector<int> a(1200), b(7000);
    for (int i = 0; i < 1200; i += 11) a[i] = 1;
    for (int j = 0; j < 7000; j += 17) b[j] = 1;
    check(a, b, 7);
    std::vector<unsigned char> sparse_requested(a.size() + b.size() - 1);
    for (int sum : {100, 1000, 3500, 7000}) sparse_requested[sum] = 1;
    check(a, b, 4, &sparse_requested, true);
    const auto sparse_a = a, sparse_b = b;

    std::fill(a.begin(), a.end(), 1);
    std::fill(b.begin(), b.end(), 1);
    // Exercise the large randomized branch on the density range that exposed
    // the fixed-budget fallback: these outputs have 32, 35, 36, and 64
    // witnesses, while k is 35.
    std::vector<unsigned char> randomized_dense_requested(a.size() + b.size() - 1);
    for (int sum : {31, 34, 35, 63}) randomized_dense_requested[sum] = 1;
    check(a, b, 35, &randomized_dense_requested, true);
    // With no request mask, the same dimensions exceed both direct-work
    // thresholds and exercise the adaptive randomized dilution loop itself.
    check(a, b, 1, nullptr, true);

    std::vector<unsigned char> requested(a.size() + b.size() - 1);
    for (int sum : {0, 500, 1200, 4000, 7000, 8198}) requested[sum] = 1;
    check(a, b, 4, &requested);

    // Algorithm 4 must use the large-instance witness primitive and return
    // minima under the permutation it constructs.
    std::vector<std::vector<int>> rows_a{sparse_a}, rows_b{sparse_b};
    std::vector<std::vector<unsigned char>> request_rows{sparse_requested};
    std::vector<int> order(sparse_a.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = static_cast<int>(i);
    auto minima = adaptiveMinWitness(rows_a, rows_b, order, false,
                                     &request_rows);
    std::vector<int> rank(order.size());
    for (size_t i = 0; i < order.size(); ++i) rank[order[i]] = static_cast<int>(i);
    for (size_t sum = 0; sum < sparse_requested.size(); ++sum) {
        if (!sparse_requested[sum]) {
            assert(minima[0][sum] == -1);
            continue;
        }
        int expected = -1;
        for (size_t position = 0; position < sparse_a.size(); ++position)
            if (position <= sum && sum - position < sparse_b.size() &&
                sparse_a[position] && sparse_b[sum - position] &&
                (expected < 0 || rank[position] < rank[expected]))
                expected = static_cast<int>(position);
        assert(minima[0][sum] == expected);
    }
    std::cout << "deterministic large-branch witness checks passed\n";
}
