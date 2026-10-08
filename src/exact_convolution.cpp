#include "exact_convolution.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace {
int power(int base, int exponent) {
    int64_t result = 1, x = base;
    while (exponent) {
        if (exponent & 1) result = result * x % ExactConvolver::modulus;
        x = x * x % ExactConvolver::modulus;
        exponent >>= 1;
    }
    return static_cast<int>(result);
}
void ntt(std::vector<int>& values, bool inverse) {
    const int n = static_cast<int>(values.size());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(values[i], values[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int root = power(3, (ExactConvolver::modulus - 1) / len);
        if (inverse) root = power(root, ExactConvolver::modulus - 2);
        for (int start = 0; start < n; start += len) {
            int64_t factor = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = values[start + j];
                int v = static_cast<int>(factor * values[start + j + len / 2]
                    % ExactConvolver::modulus);
                int sum = u + v;
                if (sum >= ExactConvolver::modulus) sum -= ExactConvolver::modulus;
                int difference = u - v;
                if (difference < 0) difference += ExactConvolver::modulus;
                values[start + j] = sum;
                values[start + j + len / 2] = difference;
                factor = factor * root % ExactConvolver::modulus;
            }
        }
    }
    if (inverse) {
        int factor = power(n, ExactConvolver::modulus - 2);
        for (int& value : values)
            value = static_cast<int>(int64_t(value) * factor % ExactConvolver::modulus);
    }
}
}

ExactConvolver::ExactConvolver(int left_size, const std::vector<int>& right)
    : left_size_(left_size), output_size_(0), transform_size_(1) {
    if (left_size < 1 || right.empty() || left_size >= modulus)
        throw std::invalid_argument("invalid exact convolution dimensions");
    output_size_ = left_size + static_cast<int>(right.size()) - 1;
    while (transform_size_ < output_size_) transform_size_ <<= 1;
    if (transform_size_ > (1 << 23))
        throw std::invalid_argument("exact convolution exceeds NTT root order");
    right_transform_.assign(transform_size_, 0);
    for (size_t i = 0; i < right.size(); ++i) {
        if (right[i] < 0 || right[i] >= modulus)
            throw std::invalid_argument("right coefficient outside NTT field");
        right_transform_[i] = right[i];
    }
    ntt(right_transform_, false);
}
std::vector<int> ExactConvolver::convolve(const std::vector<int>& left) const {
    if (static_cast<int>(left.size()) != left_size_)
        throw std::invalid_argument("left length mismatch");
    std::vector<int> transformed(transform_size_);
    for (int i = 0; i < left_size_; ++i) {
        if (left[i] < 0 || left[i] >= modulus)
            throw std::invalid_argument("left coefficient outside NTT field");
        transformed[i] = left[i];
    }
    ntt(transformed, false);
    for (int i = 0; i < transform_size_; ++i)
        transformed[i] = static_cast<int>(int64_t(transformed[i]) *
            right_transform_[i] % modulus);
    ntt(transformed, true);
    transformed.resize(output_size_);
    return transformed;
}
int ExactConvolver::output_size() const { return output_size_; }
std::vector<int> exact_boolean_convolution(
    const std::vector<int>& a, const std::vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    auto counts = ExactConvolver(static_cast<int>(a.size()), b).convolve(a);
    for (int& count : counts) count = count != 0;
    return counts;
}
