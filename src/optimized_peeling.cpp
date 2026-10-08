#include "witness.h"
#include "exact_convolution.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <map>
#include <random>
#include <stdexcept>

namespace {
int ceil_log2(uint64_t value) {
    int exponent = 0;
    uint64_t power = 1;
    while (power < value) { power <<= 1; ++exponent; }
    return exponent;
}
uint64_t pack(int count, int sum) {
    return (uint64_t(static_cast<uint32_t>(count)) << 32) |
           static_cast<uint32_t>(sum);
}
int count_of(uint64_t pair) { return static_cast<int>(pair >> 32); }
int sum_of(uint64_t pair) { return static_cast<int>(pair & 0xffffffffu); }
bool valid(int position, int output,
           const std::vector<int>& a, const std::vector<int>& b) {
    const int paired = output - position;
    return position >= 0 && position < static_cast<int>(a.size()) &&
           paired >= 0 && paired < static_cast<int>(b.size()) &&
           a[position] && b[paired];
}
}

std::vector<std::vector<int>> k_witnesses_boolean_optimized(
    const std::vector<int>& a, const std::vector<int>& b, int k,
    const std::vector<unsigned char>* requested
) {
    if (a.empty() || b.empty()) return {};
    const int outputs = static_cast<int>(a.size() + b.size() - 1);
    if (requested && static_cast<int>(requested->size()) != outputs)
        throw std::invalid_argument("requested optimized-witness length mismatch");
    std::vector<std::vector<int>> empty(outputs);
    if (k <= 0) return empty;
    std::vector<int> binary_a(a.size()), binary_b(b.size());
    for (size_t i = 0; i < a.size(); ++i) binary_a[i] = a[i] != 0;
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;
    if (uint64_t(a.size()) * b.size() <= 8000000ULL)
        return k_witnesses_boolean_deterministic(binary_a, binary_b, k,
                                                 requested);

    ExactConvolver convolver(static_cast<int>(a.size()), binary_b);
    const auto original = convolver.convolve(binary_a);
    const int threshold = static_cast<int>(std::min<uint64_t>(
        uint64_t(4) * k, a.size()));
    std::vector<int> small;
    std::map<int, std::vector<int>> large_by_level;
    for (int sum = 0; sum < outputs; ++sum) {
        if ((requested && !(*requested)[sum]) || original[sum] == 0) continue;
        if (original[sum] <= threshold) small.push_back(sum);
        else large_by_level[ceil_log2(original[sum])].push_back(sum);
    }
    if (small.empty() && large_by_level.empty()) return empty;

    // Theorem 9: bounded 4k-peeler. F(0) contains the whole universe, and
    // F(j) has r_j independent 2^-j masks. The constants below are the
    // paper's explicit success-probability bounds.
    constexpr double alpha = 1.0 / (2.0 * 2.71828182845904523536);
    const double denominator = std::log(1.0 / (1.0 - alpha));
    std::vector<double> peel_probabilities{1.0};
    for (int level = 1; level <= ceil_log2(std::max(1, threshold)); ++level) {
        const int scale = 1 << level;
        const int trials = static_cast<int>(std::ceil(
            (double(scale) + 3.0 * std::log2(double(outputs) * threshold)) /
            denominator));
        peel_probabilities.insert(peel_probabilities.end(), trials,
                                  1.0 / double(scale));
    }
    const size_t masks = peel_probabilities.size();
    // A memory-bounded implementation can use the generic Las Vegas
    // routine; materializing the full peeler table is unnecessary then.
    if (small.size() && masks > 64000000ULL / small.size())
        return k_witnesses_boolean_randomized(binary_a, binary_b, k,
                                              requested);

    std::mt19937_64 generator(std::random_device{}());
    std::vector<int> selected(a.size()), weighted(a.size());
    for (;;) {
        auto answer = empty;
        std::vector<std::vector<int>> incidence(a.size());
        std::vector<uint64_t> peel_data(small.size() * masks);
        for (size_t mask = 0; mask < masks && !small.empty(); ++mask) {
            std::bernoulli_distribution choose(peel_probabilities[mask]);
            for (size_t position = 0; position < a.size(); ++position) {
                selected[position] = binary_a[position] &&
                    (mask == 0 || choose(generator));
                weighted[position] = selected[position]
                    ? static_cast<int>(position) + 1 : 0;
                if (selected[position]) incidence[position].push_back(
                    static_cast<int>(mask));
            }
            const auto sizes = mask == 0 ? original : convolver.convolve(selected);
            const auto sums = convolver.convolve(weighted);
            for (size_t row = 0; row < small.size(); ++row)
                peel_data[row * masks + mask] = pack(sizes[small[row]],
                                                     sums[small[row]]);
        }

        for (size_t row = 0; row < small.size(); ++row) {
            const int output = small[row];
            const int target = std::min(k, original[output]);
            std::deque<int> singleton;
            for (size_t mask = 0; mask < masks; ++mask)
                if (count_of(peel_data[row * masks + mask]) == 1)
                    singleton.push_back(static_cast<int>(mask));
            while (!singleton.empty() &&
                   static_cast<int>(answer[output].size()) < target) {
                const int mask = singleton.front();
                singleton.pop_front();
                auto& slot = peel_data[row * masks + mask];
                if (count_of(slot) != 1) continue;
                const int position = sum_of(slot) - 1;
                if (!valid(position, output, binary_a, binary_b))
                    throw std::runtime_error("peeling produced invalid witness");
                if (std::find(answer[output].begin(), answer[output].end(),
                              position) != answer[output].end())
                    throw std::runtime_error("peeling repeated a witness");
                answer[output].push_back(position);
                for (int affected : incidence[position]) {
                    auto& other = peel_data[row * masks + affected];
                    int count = count_of(other);
                    if (count == 0) continue;
                    int sum = sum_of(other) - (position + 1);
                    if (sum < 0) sum += ExactConvolver::modulus;
                    other = pack(count - 1, sum);
                    if (count == 2) singleton.push_back(affected);
                }
            }
        }

        // Theorem 10: large-set k-separator. A singleton from a sample is
        // one independent uniformly selected witness of that output.
        for (const auto& [level, sums_at_level] : large_by_level) {
            const double base = 16.0 / alpha * std::log(2.0 * outputs);
            const double extra = (2.0 / alpha) * level * k /
                (level - 1.0 - std::log2(double(k)));
            const int trials = static_cast<int>(std::ceil(base + extra));
            std::bernoulli_distribution choose(std::ldexp(1.0, -level));
            for (int trial = 0; trial < trials; ++trial) {
                bool pending = false;
                for (int output : sums_at_level)
                    pending |= static_cast<int>(answer[output].size()) < k;
                if (!pending) break;
                for (size_t position = 0; position < a.size(); ++position) {
                    selected[position] = binary_a[position] && choose(generator);
                    weighted[position] = selected[position]
                        ? static_cast<int>(position) + 1 : 0;
                }
                const auto sizes = convolver.convolve(selected);
                bool isolated = false;
                for (int output : sums_at_level)
                    if (sizes[output] == 1 &&
                        static_cast<int>(answer[output].size()) < k)
                        isolated = true;
                if (!isolated) continue;
                const auto sums = convolver.convolve(weighted);
                for (int output : sums_at_level) {
                    if (sizes[output] != 1 ||
                        static_cast<int>(answer[output].size()) >= k) continue;
                    const int position = sums[output] - 1;
                    if (!valid(position, output, binary_a, binary_b))
                        throw std::runtime_error("separator produced invalid witness");
                    if (std::find(answer[output].begin(), answer[output].end(),
                                  position) == answer[output].end())
                        answer[output].push_back(position);
                }
            }
        }

        bool complete = true;
        for (int output : small)
            complete &= static_cast<int>(answer[output].size()) ==
                std::min(k, original[output]);
        for (const auto& [level, positions] : large_by_level)
            for (int output : positions)
                complete &= static_cast<int>(answer[output].size()) == k;
        if (complete) return answer;
    }
}
