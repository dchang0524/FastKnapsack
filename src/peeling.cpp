#include "peeling.h"
#include "witness.h"
#include <stdexcept>

namespace {
vector<vector<int>> reconstruct(string& text, string& pat, int k,
                                 bool randomized) {
    if (pat.empty() || text.size() < pat.size()) return {};
    if (text.size() > 2 * pat.size()) {
        // Split long texts into overlapping windows of at most 2m-1
        // characters. Each alignment belongs to exactly one block.
        vector<vector<int>> result(text.size() - pat.size() + 1);
        for (size_t first = 0; first < result.size(); first += pat.size()) {
            string window = text.substr(first,
                std::min(text.size() - first, 2 * pat.size() - 1));
            auto local = reconstruct(window, pat, k, randomized);
            const size_t take = std::min(pat.size(), result.size() - first);
            for (size_t offset = 0; offset < take; ++offset)
                result[first + offset] = std::move(local[offset]);
        }
        return result;
    }
    vector<int> binary_text(text.size()), reversed(pat.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '0' && text[i] != '1')
            throw invalid_argument("text must be binary");
        binary_text[i] = text[i] - '0';
    }
    for (size_t j = 0; j < pat.size(); ++j) {
        if (pat[j] != '0' && pat[j] != '1')
            throw invalid_argument("pattern must be binary");
        reversed[pat.size() - 1 - j] = pat[j] - '0';
    }
    // Put the reversed pattern first. The peeling universe then has size m,
    // matching the O(n log m (k + log k log m)) bound of the aligned-ones
    // application, even when the text is much longer than the pattern.
    // Alignment i is convolution coefficient i + |pat| - 1.
    auto convolution_witnesses = randomized
        ? k_witnesses_boolean_optimized(reversed, binary_text, k)
        : k_witnesses_boolean_deterministic(reversed, binary_text, k);
    vector<vector<int>> result(text.size() - pat.size() + 1);
    for (size_t i = 0; i < result.size(); ++i) {
        for (int reversed_position : convolution_witnesses[i + pat.size() - 1])
            result[i].push_back(static_cast<int>(pat.size()) - 1 -
                                reversed_position);
        sort(result[i].begin(), result[i].end());
    }
    return result;
}
}

vector<vector<int>> k_reconstruct_randomized(string& text, string& pat, int k) {
    return reconstruct(text, pat, k, true);
}

vector<vector<int>> k_reconstruct_deterministic(string& text, string& pat, int k) {
    return reconstruct(text, pat, k, false);
}

vector<vector<int>> k_find_witnesses_randomized(
    vector<int>& a, vector<int>& b, int k
) {
    if (b.empty() || a.size() < b.size()) return {};
    string text, pattern;
    text.reserve(a.size());
    pattern.reserve(b.size());
    for (int value : a) text.push_back(value ? '1' : '0');
    for (int value : b) pattern.push_back(value ? '1' : '0');
    return k_reconstruct_randomized(text, pattern, k);
}

vector<vector<int>> k_find_witnesses_deterministic(
    vector<int>& a, vector<int>& b, int k
) {
    if (b.empty() || a.size() < b.size()) return {};
    string text, pattern;
    text.reserve(a.size());
    pattern.reserve(b.size());
    for (int value : a) text.push_back(value ? '1' : '0');
    for (int value : b) pattern.push_back(value ? '1' : '0');
    return k_reconstruct_deterministic(text, pattern, k);
}

vector<vector<int>> k_find_witnesses_knapsack(
    vector<int>& a, vector<int>& b, vector<int>& order,
    vector<int>& w, int k
) {
    return optimized_k_witness(a, b, k, w, order);
}
