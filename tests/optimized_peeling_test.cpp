#include "algorithms.h"
#include "witness.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <algorithm>
#include <cassert>
#include <vector>

namespace {
void check(const std::vector<int>& a, const std::vector<int>& b,
           int k, int output) {
    std::vector<unsigned char> requested(a.size() + b.size() - 1);
    requested[output] = 1;
    auto found = k_witnesses_boolean_optimized(a, b, k, &requested);
    std::vector<int> expected;
    for (int position = 0; position < static_cast<int>(a.size()); ++position)
        if (output - position >= 0 &&
            output - position < static_cast<int>(b.size()) &&
            a[position] && b[output - position])
            expected.push_back(position);
    assert(found[output].size() ==
           std::min<size_t>(k, expected.size()));
    std::sort(found[output].begin(), found[output].end());
    assert(std::unique(found[output].begin(), found[output].end()) ==
           found[output].end());
    for (int position : found[output])
        assert(std::binary_search(expected.begin(), expected.end(), position));
    for (size_t sum = 0; sum < found.size(); ++sum)
        if (sum != static_cast<size_t>(output)) assert(found[sum].empty());
}
}

int main() {
    std::vector<int> a(1200), b(7000, 1);
    for (int position : {0, 1, 2}) a[position] = 1;
    check(a, b, 2, 500); // bounded peeler: three witnesses
    for (int position = 3; position < 9; ++position) a[position] = 1;
    check(a, b, 2, 500); // separator: nine witnesses > 4k

    std::vector<std::vector<int>> rows_a{a}, rows_b{b};
    std::vector<std::vector<unsigned char>> requested(1,
        std::vector<unsigned char>(a.size() + b.size() - 1));
    requested[0][500] = 1;
    std::vector<int> order(a.size());
    for (size_t i = 0; i < order.size(); ++i) order[i] = static_cast<int>(i);
    auto minima = adaptiveMinWitness_optimized(rows_a, rows_b, order,
                                                &requested);
    std::vector<int> rank(order.size());
    for (size_t i = 0; i < order.size(); ++i) rank[order[i]] = static_cast<int>(i);
    int expected = -1;
    for (int position = 0; position < static_cast<int>(a.size()); ++position)
        if (a[position] && position <= 500 && b[500 - position] &&
            (expected < 0 || rank[position] < rank[expected]))
            expected = position;
    assert(minima[0][500] == expected);
}
