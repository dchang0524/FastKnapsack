#include "probability_space.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    const int variables = 8, wise = 3;
    const double epsilon = 0.125;
    AlmostIndependentSpace space(variables, wise, epsilon);
    double worst = 0.0;
    int checked = 0;
    for (int subset = 1; subset < (1 << variables); ++subset) {
        int count = __builtin_popcount(static_cast<unsigned>(subset));
        if (count > wise) continue;
        std::vector<int> positions;
        for (int i = 0; i < variables; ++i)
            if (subset & (1 << i)) positions.push_back(i);
        std::vector<uint64_t> frequencies(1 << count);
        for (uint64_t point = 0; point < space.size(); ++point) {
            auto bits = space.sample(point);
            int pattern = 0;
            for (int j = 0; j < count; ++j)
                pattern |= int(bits[positions[j]]) << j;
            ++frequencies[pattern];
        }
        for (uint64_t frequency : frequencies) {
            double deviation = std::abs(double(frequency) / space.size() -
                                        1.0 / frequencies.size());
            worst = std::max(worst, deviation);
            assert(deviation <= epsilon + 1e-12);
            ++checked;
        }
    }
    std::cout << std::setprecision(8)
              << "c-wise epsilon-independence empirical check: variables="
              << variables << " c=" << wise << " epsilon=" << epsilon
              << " sample-space-size=" << space.size()
              << " checked-patterns=" << checked
              << " max-deviation=" << worst << '\n';
}
