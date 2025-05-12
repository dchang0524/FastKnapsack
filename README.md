# FastKnapsack

This repository implements the near-linear algorithms from  
**“On Problems Related to Unbounded SubsetSum: A Unified Combinatorial Approach”**  
(Chan, Deng, Mao, Zhong; arXiv:2202.13484) in C++.

It provides fast, practical code for three classic problems:

- **All-Target Unbounded Knapsack**  
  For each capacity within some range, compute the maximum profit using unlimited copies of each item.
- **All-Target CoinChange**  
  For each sum within some range, compute the minimum number of coins needed.
- **Residue Table**  
  Given coin denominations, for each residue, find the smallest achievable sum.

## Key Techniques

1. **Combinatorial kernels + witness propagation** (Algorithms 2 & 1)  
2. **Adaptive minimum-witness finding** (Algorithm 4)  
3. **Peeling for \(k\)-reconstruction** (Lemma 5.6)  
4. **Greedy hitting-set** (Lemma 5.7)  

All routines run in near-linear or sub-quadratic time, suitable for large \(T\) and \(u\).
