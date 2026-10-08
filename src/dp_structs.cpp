#include "dp_structs.h"

// default constructor
solution::solution()
  : size(0), value(0), weight(0), svec()
{}

// add a single coin
void solution::addCoin(int i, int wgt, int prof) {
    svec[i]++;
    weight += wgt;
    value  += prof;
    size++;
}

// dump for debugging
void solution::dump() const {
    std::cout
        << "size="   << size
        << " value=" << value
        << " weight="<< weight << "\n";
    for (auto const& p : svec) {
        std::cout
            << "[" << p.first
            << " x "    << p.second
            << "]\n";
    }
}

// lexicographic compare
bool solution::lexCmp(const solution &s2) {
    auto it1 = svec.begin(), end1 = svec.end();
    auto it2 = s2.svec.begin(), end2 = s2.svec.end();

    // The first differing multiplicity in rank order decides the comparison.
    // The paper uses the order that prefers more copies of the earlier coin.
    while (it1 != end1 || it2 != end2) {
        if (it2 == end2 || (it1 != end1 && it1->first < it2->first))
            return it1->second > 0;
        if (it1 == end1 || it2->first < it1->first)
            return false;
        if (it1->second != it2->second)
            return it1->second > it2->second;
        ++it1;
        ++it2;
    }
    return false;
}

//copy s1 over to s2
void solution::copy(solution &s2) {
    s2.size = size;
    s2.value = value;
    s2.weight = weight;
    s2.svec.clear();
    for (const auto c : svec) {
        s2.svec[c.first] = c.second;
    }
}
