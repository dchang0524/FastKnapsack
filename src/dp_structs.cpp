#include "dp_structs.h"
#include <vector>
using namespace std;
// Compare lexicographically: return true if s1 < s2 (σ-order assumed global)
bool lexCmp(const solution& s1, const solution& s2) {
    int n = (int)s1.svec.size();
    for (int i = 0; i < n; i++) {
        if (s1.svec[i] != s2.svec[i])
            return s1.svec[i] > s2.svec[i];
    }
    return false;
}
