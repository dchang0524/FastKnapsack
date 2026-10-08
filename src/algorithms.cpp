#include "algorithms.h"
#include "exact_convolution.h"
#include "peeling.h"
#include <numeric>
#include <stdexcept>

// Algorithm 1: Witness Propagation
/**
 * Standard Knapsack-DP given some set of solutions (called "kernels")
 * Gurantees optimal solution due to Optimal Substructure Combinatorial Property
 * For All-Target Unbounded Knapsack and All-Target Coinchange
 */
void propagation(
    const vector<int>& w, //weight of each coin
    const vector<int>& p, //the profit of each coin
    int t, //the bound for the range we want to compute solutions for
    vector<solution>& sol, //sol[c] <- properties related to solution for the weight sum = c
    const vector<int>& order //the lexicographical order of the coins
) {
    for (int j = 1; j <= t; j++) {
        if (sol[j].size == 0) continue;
        for (auto const C : sol[j].svec) { //use the svec instead of supp
            int x = C.first;
            int nxt = j + w[order[x]];
            if (nxt > t) continue;
            sol[j].svec[x]++;
            sol[j].size++;
            sol[j].value += p[order[x]];
            sol[j].weight += w[order[x]];
            if (sol[nxt].size == 0
                || sol[j].value > sol[nxt].value
                || (sol[j].value == sol[nxt].value && sol[j].lexCmp(sol[nxt]))) //lexCmp should take (log n)^2 time
            {
                sol[j].copy(sol[nxt]);
            }
            sol[j].svec[x]--;
            sol[j].size--;
            sol[j].value -= p[order[x]];
            sol[j].weight -= w[order[x]];
        }
    }
}

// Algorithm 2: Kernel Computation
/**
 * Computes the x-kernels for x in 1, ..., 2*logu + 1
 * A k-kernel is the set of solutions total of k (not necessarily distinct) coins
 * This specific implementation is only for All-Target Unbounded Knapsack
 * For All-Target CoinChange and Residue Table replace (max, +) convolutions with Boolean convolution, and use algorithm 4 for finding minimum witnesses
 * For CoinChange, the value of a solution is just the number of convolutions iterated
 * For Residue Table, the value of a solution is the sum itself
 */
void kernelComputation_knapsack(
    int n,                              // number of coins
    int u,                              // maximum coin weight
    const vector<int>& w,               // weights of the coins (1-indexed)
    const vector<int>& p,               // profits of the coins  (1-indexed)
    const vector<int>& order,           // lexicographical order σ[1..n]
    int t,                          // (unused) global target bound
    vector<solution>& sol,             // output: sol[c] for c∈[0..k·u]
    const MaxPlusBackend& convolve
) {
    if (u < 1 || n < 0 || static_cast<int>(order.size()) != n + 1 ||
        static_cast<int>(w.size()) <= n || static_cast<int>(p.size()) <= n)
        throw invalid_argument("invalid unbounded-knapsack dimensions");
    const int k = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    const int limit = k * u + 1;
    sol.assign(max(limit, t + 1), solution());

    vector<ll> best(limit, NEG_INF_LL), item(u + 1, NEG_INF_LL);
    vector<int> item_rank(u + 1, -1), seen(n + 1);
    best[0] = 0;
    for (int rank = 1; rank <= n; ++rank) {
        const int coin = order[rank];
        if (coin < 1 || coin > n || seen[coin]++ ||
            w[coin] < 1 || w[coin] > u)
            throw invalid_argument("invalid unbounded-knapsack item order");
        const int weight = w[coin];
        if (p[coin] > item[weight] ||
            (p[coin] == item[weight] &&
             (item_rank[weight] < 0 || rank < item_rank[weight]))) {
            item[weight] = p[coin];
            item_rank[weight] = rank;
        }
    }

    auto checked_scale = [n](ll value) -> ll {
        const __int128 scaled = static_cast<__int128>(n + 1) * value;
        if (scaled <= NEG_INF_LL || scaled >= LLONG_MAX)
            throw overflow_error("knapsack witness encoding overflows int64");
        return static_cast<ll>(scaled);
    };
    vector<ll> encoded_item(u + 1, NEG_INF_LL);
    for (int weight = 1; weight <= u; ++weight)
        if (item_rank[weight] > 0)
            encoded_item[weight] = checked_scale(item[weight]) - item_rank[weight];

    // Keep all solutions with at most the current number of items. This is
    // necessary when profits are negative: a new layer can be worse than a
    // solution using fewer items at the same sum.
    for (int iter = 1; iter <= k; ++iter) {
        vector<ll> encoded_best(limit, NEG_INF_LL);
        for (int sum = 0; sum < limit; ++sum)
            if (best[sum] != NEG_INF_LL)
                encoded_best[sum] = checked_scale(best[sum]);
        const auto encoded_result = convolve(encoded_best, encoded_item);
        if (encoded_result.size() != encoded_best.size() +
                                     encoded_item.size() - 1)
            throw runtime_error("max-plus backend returned wrong output length");
        const vector<solution> previous(sol.begin(), sol.begin() + limit);
        for (int sum = 1; sum < limit; ++sum) {
            const ll encoded = encoded_result[sum];
            if (encoded == NEG_INF_LL) continue;
            const ll candidate_value = encoded / (n + 1) +
                                       (encoded % (n + 1) > 0);
            if (candidate_value < best[sum]) continue;
            const ll remainder = (encoded % (n + 1) + n + 1) % (n + 1);
            const int rank = static_cast<int>((n + 1 - remainder) % (n + 1));
            if (rank < 1 || rank > n)
                throw runtime_error("invalid encoded knapsack witness");
            const int coin = order[rank], predecessor = sum - w[coin];
            if (predecessor < 0 || best[predecessor] == NEG_INF_LL)
                throw runtime_error("knapsack witness has no predecessor");
            solution candidate = previous[predecessor];
            candidate.addCoin(rank, w[coin], p[coin]);
            if (candidate.value != candidate_value)
                throw runtime_error("knapsack witness value mismatch");
            if (best[sum] == NEG_INF_LL || candidate.value > best[sum] ||
                (candidate.value == best[sum] && candidate.lexCmp(sol[sum]))) {
                sol[sum] = std::move(candidate);
                best[sum] = candidate_value;
            }
        }
    }
}

namespace {
void kernelComputation_coinchange_fixed_order(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol, bool random_order
) {
    (void)p;
    if (u < 1 || n < 0 || static_cast<int>(order.size()) != n + 1 ||
        static_cast<int>(w.size()) <= n)
        throw invalid_argument("invalid fixed-order coin-change dimensions");
    const int k = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    const int limit = k * u + 1;
    sol.assign(max(limit, t + 1), solution());
    vector<int> first(u + 1);
    vector<int> seen(n + 1);
    for (int rank = 1; rank <= n; ++rank) {
        const int coin = order[rank];
        if (coin < 1 || coin > n || seen[coin]++ ||
            w[coin] < 1 || w[coin] > u)
            throw invalid_argument("invalid fixed-order coin or weight");
        first[w[coin]] = 1;
    }
    if (random_order) {
        mt19937 generator(random_device{}());
        shuffle(order.begin() + 1, order.end(), generator);
    }

    vector<vector<int>> previous_rows;
    previous_rows.reserve(k);
    vector<int> previous(limit), first_layer(limit, -1);
    previous[0] = 1;
    first_layer[0] = 0;
    for (int iter = 1; iter <= k; ++iter) {
        previous_rows.push_back(std::move(previous));
        auto reachable = exact_boolean_convolution(first, previous_rows.back());
        previous.assign(limit, 0);
        for (int sum = 1; sum < limit; ++sum) {
            previous[sum] = reachable[sum];
            if (reachable[sum] && first_layer[sum] < 0)
                first_layer[sum] = iter;
        }
    }

    for (int iter = 1; iter <= k; ++iter) {
        auto witnesses = random_order
            ? minimum_witness_random(first, previous_rows[iter - 1], w, order)
            : minimum_witness_boolCnv_ordered(first, previous_rows[iter - 1],
                                              w, order);
        for (int sum = 1; sum < limit; ++sum) {
            if (first_layer[sum] != iter) continue;
            const int rank = witnesses[sum];
            if (rank < 1 || rank > n)
                throw runtime_error("fixed-order coin-change witness is missing");
            const int coin = order[rank];
            const int predecessor = sum - w[coin];
            if (predecessor < 0 || !previous_rows[iter - 1][predecessor])
                throw runtime_error("fixed-order coin-change predecessor is missing");
            sol[predecessor].copy(sol[sum]);
            sol[sum].addCoin(rank, w[coin], -1);
        }
    }
}
}

void kernelComputation_coinchange_simple(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol
) {
    kernelComputation_coinchange_fixed_order(n, u, w, p, order, t, sol, false);
}

void kernelComputation_coinchange_randomized(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol
) {
    kernelComputation_coinchange_fixed_order(n, u, w, p, order, t, sol, true);
}

void kernelComputation_coinchange(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol, bool randomized,
    bool optimized_peeling
) {
    (void)p; // A coin contributes value -1, independent of the profit input.
    if (u < 1 || n < 0 || static_cast<int>(order.size()) != n + 1)
        throw invalid_argument("invalid coin-change dimensions or order");
    const int kernel_size = static_cast<int>(floor(2.0 * log2(u) + 1.0));
    const int limit = kernel_size * u + 1;
    sol.assign(max(limit, t + 1), solution());

    vector<int> coin_at_weight(u + 1, -1);
    vector<int> weights;
    for (int rank = 1; rank <= n; ++rank) {
        int coin = order[rank];
        if (coin < 1 || coin > n || w[coin] < 1 || w[coin] > u)
            throw invalid_argument("coin order or weight out of range");
        if (coin_at_weight[w[coin]] == -1) {
            coin_at_weight[w[coin]] = coin;
            weights.push_back(w[coin]);
        }
    }
    vector<int> first(u + 1);
    for (int weight : weights) first[weight] = 1;

    // Each layer represents sums reachable with exactly `iter` coins. A
    // previous-layer witness is then a valid predecessor for reconstruction.
    vector<vector<int>> a_rows, b_rows;
    a_rows.reserve(kernel_size);
    b_rows.reserve(kernel_size);
    vector<int> previous(limit);
    previous[0] = 1;
    vector<int> first_layer(limit, -1);
    first_layer[0] = 0;
    for (int iter = 1; iter <= kernel_size; ++iter) {
        a_rows.push_back(first);
        b_rows.push_back(std::move(previous));
        auto reachable = exact_boolean_convolution(first, b_rows.back());
        previous.assign(limit, 0);
        for (int sum = 1; sum < limit; ++sum) {
            previous[sum] = reachable[sum];
            if (reachable[sum] && first_layer[sum] == -1)
                first_layer[sum] = iter;
        }
    }

    vector<int> weight_order;
    weight_order.reserve(u + 1);
    vector<unsigned char> weight_seen(u + 1);
    for (int coin_rank = 1; coin_rank <= n; ++coin_rank) {
        int weight = w[order[coin_rank]];
        if (!weight_seen[weight]++)
            weight_order.push_back(weight);
    }
    for (int weight = 0; weight <= u; ++weight)
        if (!first[weight]) weight_order.push_back(weight);

    const int outputs = limit + u;
    vector<vector<unsigned char>> requested(kernel_size,
        vector<unsigned char>(outputs));
    for (int sum = 1; sum < limit; ++sum)
        if (first_layer[sum] > 0)
            requested[first_layer[sum] - 1][sum] = 1;
    auto min_witness = adaptiveMinWitness(a_rows, b_rows, weight_order,
                                         randomized, &requested,
                                         optimized_peeling);
    vector<int> weight_rank(u + 1);
    for (int rank = 0; rank <= u; ++rank)
        weight_rank[weight_order[rank]] = rank;
    stable_sort(order.begin() + 1, order.end(), [&](int lhs, int rhs) {
        return weight_rank[w[lhs]] < weight_rank[w[rhs]];
    });
    vector<int> coin_rank(n + 1);
    for (int rank = 1; rank <= n; ++rank) coin_rank[order[rank]] = rank;

    for (int iter = 1; iter <= kernel_size; ++iter) {
        for (int sum = 1; sum < limit; ++sum) {
            if (first_layer[sum] != iter) continue;
            const int weight = min_witness[iter - 1][sum];
            if (weight <= 0 || weight > u || coin_at_weight[weight] < 0 ||
                sum - weight < 0 || !b_rows[iter - 1][sum - weight])
                throw runtime_error("adaptive witness failed coin-change reconstruction");
            int coin = coin_at_weight[weight];
            sol[sum - weight].copy(sol[sum]);
            sol[sum].addCoin(coin_rank[coin], weight, -1);
        }
    }
}

void kernelComputation_coinchange_optimized(
    int n, int u, const vector<int>& w, const vector<int>& p,
    vector<int>& order, int t, vector<solution>& sol
) {
    kernelComputation_coinchange(n, u, w, p, order, t, sol, true, true);
}

vector<vector<int>> adaptiveMinWitness(
    const vector<vector<int>>& a, const vector<vector<int>>& b,
    vector<int>& order, bool randomized,
    const vector<vector<unsigned char>>* requested,
    bool optimized_peeling
) {
    if (a.size() != b.size()) throw invalid_argument("convolution row count mismatch");
    if (a.empty()) return {};
    const int n = static_cast<int>(a[0].size());
    if (n == 0 || static_cast<int>(order.size()) != n)
        throw invalid_argument("invalid adaptive witness order");
    vector<int> seen(n);
    for (int position : order) {
        if (position < 0 || position >= n || seen[position]++)
            throw invalid_argument("adaptive witness order is not a permutation");
    }
    int outputs = static_cast<int>(a[0].size() + b[0].size() - 1);
    if (requested && requested->size() != a.size())
        throw invalid_argument("requested witness row count mismatch");
    for (size_t row = 0; row < a.size(); ++row)
        if (static_cast<int>(a[row].size()) != n ||
            static_cast<int>(a[row].size() + b[row].size() - 1) != outputs || b[row].empty())
            throw invalid_argument("adaptive witness arrays have inconsistent lengths");

    vector<vector<int>> answer(a.size(), vector<int>(outputs, -1));
    vector<vector<unsigned char>> pending(a.size(), vector<unsigned char>(outputs));
    int remaining = 0;
    for (size_t row = 0; row < a.size(); ++row) {
        if (requested && static_cast<int>((*requested)[row].size()) != outputs)
            throw invalid_argument("requested witness output length mismatch");
        auto result = exact_boolean_convolution(a[row], b[row]);
        for (int s = 0; s < outputs; ++s) {
            pending[row][s] = result[s] != 0 &&
                (!requested || (*requested)[row][s]);
            remaining += pending[row][s];
        }
    }
    int active_size = n;
    while (active_size > 1 && remaining) {
        const int k = 2 * static_cast<int>(ceil(log2(max(2, remaining)))) + 5;
        vector<unsigned char> active(n);
        for (int i = 0; i < active_size; ++i) active[order[i]] = 1;
        vector<vector<pair<int, vector<int>>>> small_candidates(a.size());
        vector<vector<int>> large_sets;
        for (size_t row = 0; row < a.size(); ++row) {
            vector<int> restricted(n);
            for (int i = 0; i < n; ++i) restricted[i] = active[i] && a[row][i];
            auto found = optimized_peeling
                ? k_witnesses_boolean_optimized(restricted, b[row], k,
                                                &pending[row])
                : randomized
                    ? k_witnesses_boolean_randomized(restricted, b[row], k,
                                                     &pending[row])
                    : k_witnesses_boolean_deterministic(restricted, b[row], k,
                                                        &pending[row]);
            for (int s = 0; s < outputs; ++s) if (pending[row][s]) {
                if (static_cast<int>(found[s].size()) >= k)
                    large_sets.push_back(std::move(found[s]));
                else
                    small_candidates[row].emplace_back(s, std::move(found[s]));
            }
        }

        // The greedy cover has the Lemma 5.7 size bound for sets of size k.
        // Fill the remainder of the prefix arbitrarily to get exactly ceil(m/2).
        const int prefix_size = (active_size + 1) / 2;
        auto hit = computeHittingSet(large_sets, static_cast<int>(large_sets.size()), k, n);
        if (static_cast<int>(hit.size()) > prefix_size)
            throw runtime_error("hitting set exceeded half of the active universe");
        vector<unsigned char> in_prefix(n);
        for (int position : hit) in_prefix[position] = 1;
        for (int i = 0, count = static_cast<int>(hit.size());
             i < active_size && count < prefix_size; ++i) {
            if (!in_prefix[order[i]]) {
                in_prefix[order[i]] = 1;
                ++count;
            }
        }
        vector<int> revised;
        revised.reserve(n);
        for (int i = 0; i < active_size; ++i)
            if (in_prefix[order[i]]) revised.push_back(order[i]);
        for (int i = 0; i < active_size; ++i)
            if (!in_prefix[order[i]]) revised.push_back(order[i]);
        revised.insert(revised.end(), order.begin() + active_size, order.end());
        order.swap(revised);

        vector<int> rank(n);
        for (int i = 0; i < n; ++i) rank[order[i]] = i;
        for (size_t row = 0; row < a.size(); ++row) {
            for (const auto& candidate : small_candidates[row]) {
                int s = candidate.first;
                const auto& found = candidate.second;
                bool in_first_half = false;
                for (int position : found)
                    in_first_half |= in_prefix[position];
                if (in_first_half) continue;
                if (found.empty())
                    throw runtime_error("positive convolution output has no witness");
                answer[row][s] = *min_element(found.begin(), found.end(),
                    [&](int lhs, int rhs) { return rank[lhs] < rank[rhs]; });
                pending[row][s] = 0;
                --remaining;
            }
        }
        active_size = prefix_size;
    }
    if (remaining) {
        for (size_t row = 0; row < a.size(); ++row)
            for (int s = 0; s < outputs; ++s) if (pending[row][s]) {
                for (int i = 0; i < active_size; ++i) {
                    int position = order[i], paired = s - position;
                    if (a[row][position] && paired >= 0 &&
                        paired < static_cast<int>(b[row].size()) && b[row][paired]) {
                        answer[row][s] = position;
                        break;
                    }
                }
                if (answer[row][s] < 0)
                    throw runtime_error("active convolution output has no witness");
            }
    }
    return answer;
}

vector<vector<int>> adaptiveMinWitness_optimized(
    const vector<vector<int>>& a, const vector<vector<int>>& b,
    vector<int>& order, const vector<vector<unsigned char>>* requested
) {
    return adaptiveMinWitness(a, b, order, true, requested, true);
}

vector<vector<int>> adaptiveMinWitness_randomized(
    vector<vector<int>>& a, vector<vector<int>>& b, vector<vector<int>>& c,
    vector<int>& w, vector<int>& order
) {
    (void)c;
    // Legacy benchmark API: `order` is a permutation of first-array positions.
    // The witness itself is a position in `a`, regardless of the separate w array.
    (void)w;
    return adaptiveMinWitness(a, b, order, true);
}
