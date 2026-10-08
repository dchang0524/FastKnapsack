#include "witness.h"
#include "exact_convolution.h"
#include "probability_space.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

// Lemma 5.6 of Chan--Deng--Mao--Zhong reduces k-witness finding to the
// peeling/k-reconstruction primitive of Aumann--Lewenstein--Lewenstein--Tsur.
// This implementation uses the Alon--Naor deterministic witness routine:
// isolate all sets of at most c candidates and repeatedly dilute larger
// sets, enumerating an explicit c-wise epsilon-almost-independent family.
// The family is constructed in probability_space.cpp from a small-bias seed
// (Naor--Naor / Alon--Goldreich--Hastad--Peralta) and a polynomial generator.

namespace {
int unknown_count(int sum, const std::vector<int>& convolution_count,
                  const std::vector<unsigned char>& selected,
                  const std::vector<std::vector<int>>& known) {
    int count = convolution_count[sum];
    for (int position : known[sum])
        count -= selected[position] != 0;
    return count;
}

void find_one_new_witness(
    const std::vector<int>& a, const std::vector<int>& b,
    const std::vector<int>& total, const std::vector<int>& target,
    const ExactConvolver& convolver, AlmostIndependentSpace& space,
    std::vector<std::vector<int>>& found
) {
    const int n = static_cast<int>(a.size());
    const int outputs = static_cast<int>(total.size());
    std::vector<int> unresolved;
    for (int sum = 0; sum < outputs; ++sum)
        if (static_cast<int>(found[sum].size()) < target[sum])
            unresolved.push_back(sum);
    if (unresolved.empty()) return;
    const int phase_limit = static_cast<int>(
        std::ceil(1.0 + 3.0 * std::log(double(n + outputs)) /
                            std::log(4.0 / 3.0)));
    const int c = space.wise();
    const double alpha = 8.0 / double(uint64_t(1) << c);
    int outer_round = 0;
    while (!unresolved.empty()) {
        const int before = static_cast<int>(unresolved.size());
        std::vector<unsigned char> active(n), alive(outputs), resolved(outputs);
        for (int i = 0; i < n; ++i) active[i] = a[i] != 0;
        std::vector<int> current(outputs);
        for (int sum : unresolved) {
            current[sum] = total[sum] - static_cast<int>(found[sum].size());
            alive[sum] = 1;
        }
        for (int phase = 0; phase < phase_limit; ++phase) {
            std::vector<int> phase_outputs, small_outputs;
            int large_count = 0;
            uint64_t total_current = 0;
            for (int sum : unresolved) if (alive[sum]) {
                phase_outputs.push_back(sum);
                total_current += current[sum];
                if (current[sum] <= c) small_outputs.push_back(sum);
                else ++large_count;
            }
            if (phase_outputs.empty()) break;
            int small_left = static_cast<int>(small_outputs.size());
            bool have_good_dilution = false;
            std::vector<unsigned char> good_active;
            std::vector<int> good_counts;
            // Enumerating the entire almost-independent space is the
            // deterministic worst-case guarantee. In typical instances we
            // stop as soon as every small set is isolated and a good dilution
            // is found, often after only a few candidates.
            for (uint64_t point = 0; point < space.size(); ++point) {
                auto sample = space.sample(point);
                std::vector<unsigned char> diluted(n);
                std::vector<int> binary(n);
                for (int i = 0; i < n; ++i) {
                    diluted[i] = active[i] && sample[i];
                    binary[i] = diluted[i];
                }
                auto sizes = convolver.convolve(binary);
                std::vector<int> next(outputs);
                uint64_t total_next = 0;
                int lost_large = 0;
                bool isolate_small = false;
                for (int sum : phase_outputs) {
                    next[sum] = unknown_count(sum, sizes, diluted, found);
                    if (next[sum] < 0 || next[sum] > current[sum])
                        throw std::runtime_error("invalid exact witness count");
                    total_next += next[sum];
                    if (current[sum] > c && next[sum] == 0) ++lost_large;
                    if (current[sum] <= c && !resolved[sum] && next[sum] == 1)
                        isolate_small = true;
                }
                if (isolate_small) {
                    std::vector<int> weighted(n);
                    for (int i = 0; i < n; ++i)
                        if (diluted[i]) weighted[i] = i + 1;
                    auto sums = convolver.convolve(weighted);
                    for (int sum : small_outputs) {
                        if (resolved[sum] || next[sum] != 1) continue;
                        int value = sums[sum];
                        for (int position : found[sum]) if (diluted[position]) {
                            value -= position + 1;
                            if (value < 0) value += ExactConvolver::modulus;
                        }
                        int position = value - 1;
                        int paired = sum - position;
                        if (position < 0 || position >= n || !diluted[position] ||
                            paired < 0 || paired >= static_cast<int>(b.size()) ||
                            !b[paired] ||
                            std::find(found[sum].begin(), found[sum].end(), position)
                                != found[sum].end())
                            throw std::runtime_error("isolation produced an invalid witness");
                        found[sum].push_back(position);
                        resolved[sum] = 1;
                        alive[sum] = 0;
                        --small_left;
                    }
                }
                const bool progress = 4 * total_next <= 3 * total_current;
                const bool few_lost = large_count == 0 ||
                    double(lost_large) <= alpha * large_count + 1e-12;
                if (!have_good_dilution && progress && few_lost) {
                    good_active = std::move(diluted);
                    good_counts = std::move(next);
                    have_good_dilution = true;
                }
                if (have_good_dilution && small_left == 0) break;
            }
            if (!have_good_dilution || small_left)
                throw std::runtime_error("almost-independent space did not isolate or dilute");
            active.swap(good_active);
            for (int sum : phase_outputs) if (!resolved[sum]) {
                current[sum] = good_counts[sum];
                if (current[sum] == 0) alive[sum] = 0;
            }
        }
        std::vector<int> next_unresolved;
        for (int sum : unresolved) if (!resolved[sum])
            next_unresolved.push_back(sum);
        if (static_cast<int>(next_unresolved.size()) > before / 2)
            throw std::runtime_error("deterministic witness phase made insufficient progress");
        unresolved.swap(next_unresolved);
        if (++outer_round > 2 + static_cast<int>(std::ceil(std::log2(outputs + 1))))
            throw std::runtime_error("deterministic witness outer loop exceeded bound");
    }
}
}

std::vector<std::vector<int>> k_witnesses_boolean_deterministic(
    const std::vector<int>& a, const std::vector<int>& b, int k,
    const std::vector<unsigned char>* requested
) {
    if (a.empty() || b.empty()) return {};
    const int outputs = static_cast<int>(a.size() + b.size() - 1);
    std::vector<std::vector<int>> found(outputs);
    if (requested && static_cast<int>(requested->size()) != outputs)
        throw std::invalid_argument("requested witness length mismatch");
    if (k <= 0) return found;
    std::vector<int> binary_a(a.size()), binary_b(b.size());
    for (size_t i = 0; i < a.size(); ++i) binary_a[i] = a[i] != 0;
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;
    if (static_cast<uint64_t>(a.size()) * b.size() <= 8000000ULL) {
        for (int i = 0; i < static_cast<int>(a.size()); ++i) if (binary_a[i])
            for (int j = 0; j < static_cast<int>(b.size()); ++j) if (binary_b[j]) {
                int sum = i + j;
                if ((!requested || (*requested)[sum]) &&
                    static_cast<int>(found[sum].size()) < k)
                    found[sum].push_back(i);
            }
        return found;
    }
    ExactConvolver convolver(static_cast<int>(a.size()), binary_b);
    auto total = convolver.convolve(binary_a);
    std::vector<int> target(outputs);
    for (int sum = 0; sum < outputs; ++sum)
        target[sum] = (!requested || (*requested)[sum]) ? std::min(k, total[sum]) : 0;
    int phase_limit = static_cast<int>(std::ceil(
        1.0 + 3.0 * std::log(double(a.size() + outputs)) /
                  std::log(4.0 / 3.0)));
    int c = static_cast<int>(std::ceil(std::log2(32.0 * phase_limit)));
    c = std::min(c, static_cast<int>(a.size()));
    double atom_error = std::ldexp(1.0, -(c + 1));
    AlmostIndependentSpace space(static_cast<int>(a.size()), c, atom_error);
    for (int round = 0; round < k; ++round)
        find_one_new_witness(binary_a, binary_b, total, target,
                             convolver, space, found);
    return found;
}
