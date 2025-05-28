#ifndef RLELIST_H
#define RLELIST_H

#include "common.h"

namespace aria {
// Run Length Coding
template<class T>
class RLEList
{
public:
    RLEList() = default;

    void insert(T value)
    {
        if (!encoded.empty() && encoded.back().first == value) {
            ++encoded.back().second;
        } else {
            encoded.emplace_back(value, 1);
        }
    }

    T operator[](const long index) const
    {
        long count = 0;
        for (const auto &[fst, snd] : encoded) {
            count += snd;
            if (count > index) {
                return fst;
            }
        }
        return -1;
    }

    List<Pair<T, int>> encoded; // Store compressed data (values, occurrences)
};
} // namespace aria

#endif // RLELIST_H
