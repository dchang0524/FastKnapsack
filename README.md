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

## Key Ideas
# Definitions
1. Let coins 1, ..., n have weights (w_1, ..., w_n) and profits (p_1, ..., p_n)
2. A solution to a sum c is represented by a vector (m_1, ..., m_n) denoting the multiplicities of each coin such that (m_1 * w_1) + ... + (m_n * w_n) = c and the value of the solution which is (m_1 * p_1) + ... + (m_n * p_n) is maximized.
3. sol(j, σ) denotes the solution (m_1, ..., m_n) that is minized under the lexicorgraphical order of σ, where σ is a permutation of (1, ..., n).
4. supp(sol(j, σ)) denotes support of sol(j,σ), which is the set of coins that have nonzero multiplicities in sol(j, σ).
# Combinatorial Properties
1. For any lexical order σ and any feasible target j, |supp(sol(j, σ))| ≤ log_2{u} + 1
2. For any lexical order σ, a feasible target j ∈ [1, t] and a “witness” x ∈ supp(sol(j, σ)), let sol(j, σ) = (u_1, · · · , u_n). Define (v_1, · · · , v_n) as follows:
v_k = (u_k if k != x, u_k − 1 if k = x). We have sol(j − w_x, σ) = v.
# Algorithm 1: Witness Propagation
By combinatorial property 2, to compute sol(c, σ), we only need to know sol(x, σ), where x = w_{a_1} + ... + w_{a_z}, where {a_1, ..., a_z} = supp(sol(c, σ)). We call the set of values the kernel.
Assuming we have the solutions to the kernel, we can extend our solutions to [1, ..., t] using the traditional knapsack algorithm, in push DP style (where we update future states based on the current state). The coins at each state in the DP correspond to the support of our solution.
This algorithm runs in O(t * log t).
# Algorithm 2: Kernel Computation
This algorithm computes the solutions to the kernel.
We simply use O(log_2{u}) convolutions to compute the values of each solutions. For Unbounded Knapsack, we use (min, +) convolution to do this, and for CoinChange and Residue Table, we use boolean convolutions which allows the use of FFT to optimize our runtime.
Then, to reconstruct the multiplicities of each solution, we find the minimum witness during each convoultion. 
For Unbounded Knapsack, we simply use a modular trick during our (max, +) convolution to do this.
For CoinChange and Residue Table, we apply Adaptive Minimum-Witness Finding (Algorithm 4) to our boolean convolutions.
# Algorithm 4: Adaptive Minimum Witness Finding


