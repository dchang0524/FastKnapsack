#include "witness.h"
#include "exact_convolution.h"
#include <stdexcept>
namespace {
thread_local bool collect_randomized_stats = false;
thread_local RandomizedWitnessStats randomized_stats;
}

void reset_randomized_witness_stats(bool enabled) {
    collect_randomized_stats = enabled;
    randomized_stats = {};
}

RandomizedWitnessStats randomized_witness_stats() { return randomized_stats; }
/**
 * Computes the minimum witnesses of a boolean convolution for each result element
 * The `order` vector specifies the lexicographical order of the indices. 
 */
vector<int> minimum_witness_boolCnv_ordered(vector<int>& a, vector<int>& b, const vector<int>& w, vector<int>& order) {
    if (a.empty() || b.empty()) return {};
    const int n = static_cast<int>(order.size()) - 1;
    if (n < 0) throw invalid_argument("invalid ordered-witness permutation");
    vector<int> answer(a.size() + b.size() - 1, -1);
    if (n == 0) return answer;
    const int block = max(1, static_cast<int>(ceil(sqrt(double(n)))));
    ExactConvolver convolver(static_cast<int>(a.size()), b);
    for (int first = 1; first <= n; first += block) {
        const int last = min(n, first + block - 1);
        vector<int> restricted(a.size());
        for (int rank = first; rank <= last; ++rank) {
            const int coin = order[rank];
            if (coin < 1 || coin >= static_cast<int>(w.size()) ||
                w[coin] < 0 || w[coin] >= static_cast<int>(a.size()))
                throw invalid_argument("ordered witness coin or weight out of range");
            restricted[w[coin]] = a[w[coin]] != 0;
        }
        const auto counts = convolver.convolve(restricted);
        for (int sum = 0; sum < static_cast<int>(answer.size()); ++sum) {
            if (answer[sum] >= 0 || counts[sum] == 0) continue;
            for (int rank = first; rank <= last; ++rank) {
                const int weight = w[order[rank]];
                if (a[weight] && sum >= weight &&
                    sum - weight < static_cast<int>(b.size()) &&
                    b[sum - weight]) {
                    answer[sum] = rank;
                    break;
                }
            }
            if (answer[sum] < 0)
                throw runtime_error("group convolution had no ordered witness");
        }
    }
    return answer;
}

vector<vector<int>> k_minimum_witnesses_boolCnv_ordered(
    const vector<int>& a, const vector<int>& b,
    const vector<int>& order, int k
) {
    if (a.empty() || b.empty()) return {};
    if (order.size() != a.size())
        throw invalid_argument("ordered k-witness permutation length mismatch");
    vector<vector<int>> answer(a.size() + b.size() - 1);
    vector<unsigned char> seen(a.size());
    for (int position : order)
        if (position < 0 || position >= static_cast<int>(a.size()) ||
            seen[position]++)
            throw invalid_argument("ordered k-witness input is not a permutation");
    if (k <= 0) return answer;
    const int n = static_cast<int>(a.size());
    const int block = max(1, static_cast<int>(ceil(
        sqrt(double(n) / min(k, n)))));
    vector<int> binary_b(b.size());
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;
    ExactConvolver convolver(n, binary_b);
    for (int first = 0; first < n; first += block) {
        const int last = min(n, first + block);
        vector<int> restricted(n);
        for (int rank = first; rank < last; ++rank)
            restricted[order[rank]] = a[order[rank]] != 0;
        const auto counts = convolver.convolve(restricted);
        for (int sum = 0; sum < static_cast<int>(answer.size()); ++sum) {
            if (counts[sum] == 0 ||
                static_cast<int>(answer[sum].size()) >= k) continue;
            for (int rank = first; rank < last; ++rank) {
                const int position = order[rank];
                if (restricted[position] && sum >= position &&
                    sum - position < static_cast<int>(b.size()) &&
                    binary_b[sum - position]) {
                    answer[sum].push_back(position);
                    if (static_cast<int>(answer[sum].size()) == k) break;
                }
            }
        }
    }
    return answer;
}

/**
 * Uniformly samples a witness for each result element, in expected O(n log^2 n) time
 */
vector<int> randomized_witness_sampling(vector<int>& a, vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    vector<int> binary_a(a.size()), binary_b(b.size());
    for (size_t i = 0; i < a.size(); ++i) binary_a[i] = a[i] != 0;
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;
    ExactConvolver convolver(static_cast<int>(a.size()), binary_b);
    const auto original_counts = convolver.convolve(binary_a);
    vector<int> answer(original_counts.size(), -1);
    int unfinished = 0;
    for (int count : original_counts) unfinished += count > 0;
    if (!unfinished) return answer;

    mt19937_64 generator(random_device{}());
    bernoulli_distribution keep(0.5);
    const int stages = static_cast<int>(ceil(log2(max<size_t>(2, a.size())))) + 2;
    vector<int> active(a.size()), weighted(a.size());
    // Independent nested dilutions. Symmetry among the witnesses of any
    // output makes its first isolated witness uniform. Repeated rounds finish
    // all outputs in finite expected time (Lemma 5.4 of the main paper).
    while (unfinished) {
        active = binary_a;
        for (int stage = 0; stage < stages && unfinished; ++stage) {
            if (stage) for (int& bit : active) bit = bit && keep(generator);
            auto counts = convolver.convolve(active);
            bool isolated = false;
            for (size_t sum = 0; sum < answer.size(); ++sum)
                if (answer[sum] < 0 && counts[sum] == 1) {
                    isolated = true;
                    break;
                }
            if (!isolated) continue;
            for (size_t i = 0; i < active.size(); ++i)
                weighted[i] = active[i] ? static_cast<int>(i) + 1 : 0;
            auto weighted_sums = convolver.convolve(weighted);
            for (size_t sum = 0; sum < answer.size(); ++sum) {
                if (answer[sum] >= 0 || counts[sum] != 1) continue;
                const int position = weighted_sums[sum] - 1;
                if (position < 0 || position >= static_cast<int>(a.size()) ||
                    static_cast<int>(sum) - position < 0 ||
                    static_cast<int>(sum) - position >= static_cast<int>(b.size()) ||
                    !active[position] || !binary_b[sum - position])
                    throw runtime_error("random isolation returned an invalid witness");
                answer[sum] = position;
                --unfinished;
            }
        }
    }
    return answer;
}

vector<int> minimum_witness_random(vector<int>& a, vector<int>& b,
                                   const vector<int>& w, vector<int>& order) {
    if (a.empty() || b.empty()) return {};
    const int n = static_cast<int>(order.size()) - 1;
    if (n < 0) throw invalid_argument("invalid random-order permutation");
    const int outputs = static_cast<int>(a.size() + b.size() - 1);
    vector<int> answer(outputs, -1), rank_by_weight(a.size(), -1);
    vector<int> prefix(a.size());
    vector<int> binary_b(b.size());
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;
    vector<vector<int>> recovered(outputs);
    for (int rank = 1; rank <= n; ++rank) {
        const int coin = order[rank];
        if (coin < 1 || coin >= static_cast<int>(w.size()) ||
            w[coin] < 0 || w[coin] >= static_cast<int>(a.size()))
            throw invalid_argument("random-order coin or weight out of range");
        if (rank_by_weight[w[coin]] < 0) rank_by_weight[w[coin]] = rank;
    }
    for (int length = 1, previous = 0; previous < n;
         previous = length, length = min(n, length * 2)) {
        for (int rank = previous + 1; rank <= length; ++rank) {
            const int weight = w[order[rank]];
            prefix[weight] = a[weight] != 0;
        }
        ExactConvolver convolver(static_cast<int>(a.size()), binary_b);
        const auto counts = convolver.convolve(prefix);
        int pending = 0;
        for (int sum = 0; sum < outputs; ++sum)
            pending += answer[sum] < 0 && counts[sum] > 0;
        while (pending) {
            auto sample = randomized_witness_sampling(prefix, binary_b);
            for (int sum = 0; sum < outputs; ++sum) {
                if (answer[sum] >= 0 || counts[sum] == 0) continue;
                const int position = sample[sum];
                if (position < 0) throw runtime_error("random sampler missed an output");
                auto& witnesses = recovered[sum];
                if (find(witnesses.begin(), witnesses.end(), position) == witnesses.end())
                    witnesses.push_back(position);
                if (static_cast<int>(witnesses.size()) == counts[sum]) {
                    int best = n + 1;
                    for (int witness : witnesses)
                        best = min(best, rank_by_weight[witness]);
                    answer[sum] = best;
                    --pending;
                }
            }
        }
        if (length == n) break;
    }
    return answer;
}

vector<vector<int>> k_witnesses_boolean_randomized(
    const vector<int>& a, const vector<int>& b, int k,
    const vector<unsigned char>* requested) {
    if (a.empty() || b.empty()) return {};
    vector<vector<int>> out(a.size() + b.size() - 1);
    if (requested && requested->size() != out.size())
        throw invalid_argument("requested witness length mismatch");
    if (k <= 0) return out;
    if (collect_randomized_stats) randomized_stats.max_k = max(randomized_stats.max_k, k);
    vector<int> binary_a(a.size()), binary_b(b.size());
    for (size_t i = 0; i < a.size(); ++i) binary_a[i] = a[i] != 0;
    for (size_t i = 0; i < b.size(); ++i) binary_b[i] = b[i] != 0;

    // Algorithm 4 usually requests only a small subset of the convolution
    // outputs in each row. Estimate that work directly instead of treating a
    // long, mostly irrelevant b array as dense work.
    constexpr unsigned long long direct_limit = 8000000ULL;
    if (requested) {
        unsigned long long requested_work = 0;
        for (size_t s = 0; s < requested->size() && requested_work <= direct_limit; ++s) {
            if (!(*requested)[s]) continue;
            const size_t first = s >= b.size() ? s - b.size() + 1 : 0;
            const size_t last = min(s, a.size() - 1);
            if (first <= last) requested_work += last - first + 1;
        }
        if (requested_work <= direct_limit) {
            if (collect_randomized_stats) ++randomized_stats.requested_enumeration_calls;
            for (size_t s = 0; s < requested->size(); ++s) if ((*requested)[s]) {
                const size_t first = s >= b.size() ? s - b.size() + 1 : 0;
                const size_t last = min(s, a.size() - 1);
                if (first > last) continue;
                for (size_t i = first; i <= last &&
                     static_cast<int>(out[s].size()) < k; ++i)
                    if (binary_a[i] && binary_b[s - i])
                        out[s].push_back(static_cast<int>(i));
            }
            return out;
        }
    }

    // On small dense instances, direct enumeration is much cheaper than
    // repeated transforms and gives the exact answer without sampling variance.
    if (static_cast<unsigned long long>(a.size()) * b.size() <= direct_limit) {
        if (collect_randomized_stats) ++randomized_stats.pair_enumeration_calls;
        for (size_t i = 0; i < a.size(); ++i) if (binary_a[i])
            for (size_t j = 0; j < b.size(); ++j) if (binary_b[j]) {
                size_t s = i + j;
                if ((!requested || (*requested)[s]) &&
                    static_cast<int>(out[s].size()) < k)
                    out[s].push_back(static_cast<int>(i));
            }
        return out;
    }

    if (collect_randomized_stats) ++randomized_stats.sampling_calls;
    ExactConvolver convolver(static_cast<int>(a.size()), binary_b);
    const auto counts = convolver.convolve(binary_a);
    vector<int> target(counts.size());
    int unfinished = 0;
    for (size_t s = 0; s < counts.size(); ++s) {
        target[s] = (!requested || (*requested)[s])
            ? min(k, max(0, counts[s])) : 0;
        unfinished += target[s] != 0;
    }
    if (!unfinished) return out;

    mt19937_64 rng(random_device{}());
    bernoulli_distribution keep(0.5);
    const int levels = static_cast<int>(ceil(log2(max<size_t>(1, a.size()))));
    vector<int> sampled(a.size()), weighted(a.size());

    // Run independent nested dilution chains until every requested output is
    // complete. An output with q unknown witnesses is processed at level
    // ceil(log2(q)), where every unknown survives with probability 2^-level.
    // This isolates one of them with constant probability. Recomputing the
    // level from q after every chain avoids the coupon-collector tail caused
    // by continuing to sample at the output's original density.
    while (unfinished) {
        vector<vector<int>> outputs_at_level(levels + 1);
        if (collect_randomized_stats) ++randomized_stats.dilution_chains;
        for (size_t s = 0; s < target.size(); ++s) {
            if (static_cast<int>(out[s].size()) >= target[s]) continue;
            const int unknown = counts[s] - static_cast<int>(out[s].size());
            if (unknown <= 0)
                throw runtime_error("randomized k-witness lost an unfinished output");
            int level = 0;
            while ((size_t(1) << level) < static_cast<size_t>(unknown)) ++level;
            outputs_at_level[level].push_back(static_cast<int>(s));
        }

        sampled = binary_a;
        for (int level = 0; level <= levels && unfinished; ++level) {
            if (level) {
                bool any_selected = false;
                for (size_t i = 0; i < sampled.size(); ++i) {
                    sampled[i] = sampled[i] && keep(rng);
                    any_selected = any_selected || sampled[i];
                }
                if (!any_selected) break;
            }
            if (outputs_at_level[level].empty()) continue;

            for (size_t i = 0; i < sampled.size(); ++i)
                weighted[i] = sampled[i] ? static_cast<int>(i) + 1 : 0;
            const auto sums = convolver.convolve(weighted);
            if (collect_randomized_stats) ++randomized_stats.weighted_convolutions;
            for (int s : outputs_at_level[level]) {
                int sum = sums[s];
                for (int known : out[s]) if (sampled[known]) {
                    sum -= known + 1;
                    if (sum < 0) sum += ExactConvolver::modulus;
                }
                const int candidate = sum - 1;
                // A single selected unknown always decodes to its position.
                // More than one can occasionally decode to another valid
                // unknown modulo the NTT prime; accepting that candidate is
                // also safe because all witness conditions are checked here.
                if (candidate < 0 || candidate >= static_cast<int>(a.size()) ||
                    s - candidate < 0 || s - candidate >= static_cast<int>(b.size()) ||
                    !sampled[candidate] || !binary_b[s - candidate] ||
                    find(out[s].begin(), out[s].end(), candidate) != out[s].end())
                    continue;
                out[s].push_back(candidate);
                if (static_cast<int>(out[s].size()) == target[s]) --unfinished;
            }
        }
    }
    return out;
}

namespace {
vector<vector<int>> legacy_k_witness(vector<int>& a, vector<int>& b,
    int k, const vector<int>& w, vector<int>& order, bool optimized) {
    vector<int> by_weight(a.size()), rank(a.size(), -1);
    // The legacy API indexes a by coin ID and b by weight; order[0] is a
    // sentinel and order[1..] are coin IDs.
    for (int i = 1; i < static_cast<int>(order.size()); ++i) {
        int coin = order[i];
        if (coin >= 0 && coin < static_cast<int>(w.size()) &&
            coin < static_cast<int>(a.size()) && w[coin] >= 0 &&
            w[coin] < static_cast<int>(a.size()) && a[coin]) {
            by_weight[w[coin]] = 1;
            if (rank[w[coin]] < 0) rank[w[coin]] = i;
        }
    }
    auto positions = optimized
        ? k_witnesses_boolean_optimized(by_weight, b, k)
        : k_witnesses_boolean_randomized(by_weight, b, k);
    for (auto& row : positions) {
        for (int& position : row) position = rank[position];
        row.erase(remove(row.begin(), row.end(), -1), row.end());
    }
    return positions;
}
}

vector<vector<int>> randomized_k_witness(vector<int>& a, vector<int>& b,
    int k, const vector<int>& w, vector<int>& order) {
    return legacy_k_witness(a, b, k, w, order, false);
}

vector<vector<int>> optimized_k_witness(vector<int>& a, vector<int>& b,
    int k, const vector<int>& w, vector<int>& order) {
    return legacy_k_witness(a, b, k, w, order, true);
}
