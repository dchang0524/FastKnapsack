#ifndef EXACT_CONVOLUTION_H
#define EXACT_CONVOLUTION_H

#include <vector>

// Exact integer convolution modulo 998244353. Binary-input coefficients are
// exact counts when the first input has fewer than 998244353 entries.
class ExactConvolver {
public:
    static constexpr int modulus = 998244353;
    ExactConvolver(int left_size, const std::vector<int>& right);
    std::vector<int> convolve(const std::vector<int>& left) const;
    int output_size() const;
private:
    int left_size_, output_size_, transform_size_;
    std::vector<int> right_transform_;
};

std::vector<int> exact_boolean_convolution(
    const std::vector<int>& a, const std::vector<int>& b);

#endif
