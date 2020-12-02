#pragma once

#include <vector>

namespace ek {

/**
 * Provides an indexed free list with constant-time removals from anywhere
 * in the list without invalidating indices. T must be trivially constructable
 * and destructible.
 */
template<typename T>
class FreeList {
public:
    /// Inserts an element to the free list and returns an index to it.
    int insert(const T& element);

    // emplace new element
    int emplace();

    // Removes the nth element from the free list.
    void erase(int idx);

    // Removes all elements from the free list.
    void clear();

    // Returns the range of valid indices.
    [[nodiscard]] int range() const;

    // Returns the nth element.
    T& operator[](int idx);

    // Returns the nth element.
    const T& operator[](int idx) const;

private:
    std::vector<T> data;
    std::vector<int> nextFreeIndex;
    int firstFreeIndex = -1;
};

template<typename T>
int FreeList<T>::insert(const T& element) {
    if (firstFreeIndex != -1) {
        const int index = firstFreeIndex;
        firstFreeIndex = nextFreeIndex[firstFreeIndex];
        data[index].element = element;
        return index;
    } else {
        data.push_back(element);
        nextFreeIndex.emplace_back();
        return static_cast<int>(data.size() - 1);
    }
}

template<typename T>
int FreeList<T>::emplace() {
    if (firstFreeIndex != -1) {
        const int index = firstFreeIndex;
        firstFreeIndex = nextFreeIndex[firstFreeIndex];
        return index;
    } else {
        data.emplace_back();
        nextFreeIndex.emplace_back();
        return static_cast<int>(data.size() - 1);
    }
}

template<typename T>
void FreeList<T>::erase(int idx) {
    nextFreeIndex[idx] = firstFreeIndex;
    firstFreeIndex = idx;
}

template<typename T>
void FreeList<T>::clear() {
    data.clear();
    nextFreeIndex.clear();
    firstFreeIndex = -1;
}

template<typename T>
int FreeList<T>::range() const {
    return static_cast<int>(data.size());
}

template<typename T>
T& FreeList<T>::operator[](int idx) {
    return data[idx];
}

template<typename T>
const T& FreeList<T>::operator[](int idx) const {
    return data[idx];
}

}