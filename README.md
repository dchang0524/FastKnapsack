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

# Key Ideas
## Definitions
1. Let coins 1, ..., n have weights (w_1, ..., w_n) and profits (p_1, ..., p_n)
2. A solution to a sum c is represented by a vector (m_1, ..., m_n) denoting the multiplicities of each coin such that (m_1 * w_1) + ... + (m_n * w_n) = c and the value of the solution which is (m_1 * p_1) + ... + (m_n * p_n) is maximized.
3. sol(j, σ) denotes the solution (m_1, ..., m_n) that is minized under the lexicorgraphical order of σ, where σ is a permutation of (1, ..., n).
4. supp(sol(j, σ)) denotes support of sol(j,σ), which is the set of coins that have nonzero multiplicities in sol(j, σ).
5. A "witness" of a convolution is a term that contributed to the nonzero term in the result convolution.
## Combinatorial Properties
1. For any lexical order σ and any feasible target j, |supp(sol(j, σ))| ≤ log_2{u} + 1
2. For any lexical order σ, a feasible target j ∈ [1, t] and a “witness” x ∈ supp(sol(j, σ)), let sol(j, σ) = (u_1, · · · , u_n). Define (v_1, · · · , v_n) as follows:
v_k = (u_k if k != x, u_k − 1 if k = x). We have sol(j − w_x, σ) = v.
## Algorithm 1: Witness Propagation
By combinatorial property 2, to compute sol(c, σ), we only need to know sol(x, σ), where x = w_{a_1} + ... + w_{a_z}, where {a_1, ..., a_z} = supp(sol(c, σ)). We call the set of values the kernel.
Assuming we have the solutions to the kernel, we can extend our solutions to [1, ..., t] using the traditional knapsack algorithm, in push DP style (where we update future states based on the current state). The coins at each state in the DP correspond to the support of our solution.
This algorithm runs in O(t * log u).
## Algorithm 2: Kernel Computation
This algorithm computes the solutions to the kernel.
We simply use O(log_2{u}) convolutions to compute the values of each solutions. For Unbounded Knapsack, we use (min, +) convolution to do this, and for CoinChange and Residue Table, we use boolean convolutions which allows the use of FFT to optimize our runtime.
Then, to reconstruct the multiplicities of each solution, we find the minimum witness during each convoultion. 
For Unbounded Knapsack, we simply use a modular trick during our (max, +) convolution to do this.
For CoinChange and Residue Table, we apply Adaptive Minimum-Witness Finding (Algorithm 4) to our boolean convolutions.
## Algorithm 4: Adaptive Minimum Witness Finding
Let us have p boolean convolutions: c_i = boolCnv(a_i, b_i) for i = [1, p]. a_i represents the boolean array of our coins, while b_i represents the boolean arrays of our results after making i convolutions during the Kernel Computation.
We use the following two blackbox algorithms from other papers:
1. For some c = boolCnv(a,b), for every index i where c[i] > 0, let x[i] = {witness a[j] | a[j]^b[i-j] = 1}. We can compute min(|x[i]|, k) elements in x[i] in O~(k(n+m)) time.
2. Given sets S_1, ..., S_u, where for i in 1,...,u S_i is a subset of {1, ..., n} and |S_i| >= R, we can compute a "hitting set" of size <= (n/R)*log(u) so that the hitting set contains at least one element in each S_i. This can be done in O~((n+u)R) time.
With the p boolean convolutions, instead of trying to find the minimum witness of every convolution, we fix an arbitrary witness for each convolution, then find an ordering that makes the fixed witness the minimum witness.
Let n be the length of our permutation. For our p boolean convolutions, we apply (1) to find at most k witnesses for each entry in each convolution. We choose k = 2log(p * t). If an entry has less than k convolutions, we can ignore it and compute its minimum witness among its set of witnesses after fully determining our permutation. Now, every element has k elements, so we apply (2) to find a hitting set of set of size <= (n/2log(p * t))log(p * t) = n/2. If we let the first min(n/2, |hitting set|) elements be some permutation of the hitting set, all the convolutions will have their minimum witness within the first min(n/2, |hitting set|) elements, so we can order the latter part of the permutation in any way. Now, we change the permutation we need to compute be the hitting set, reducing the size of the permutation to less than or equal to n/2. We change a_i by getting rid of the coins not in this first part of the permutation, then recompute c_i. Now, we apply this algorithm again on this new permutation, until the size of our new permutation is <= 1.
This gives us the lexical ordering and the corresponding minimum witnesses of our convolutions under that ordering in O~(n).c 
