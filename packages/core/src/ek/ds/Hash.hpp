#pragma once

#include <cstdint>
#include "Array.hpp"
#include "PodArray.hpp"
#include <ek/assert.h>

namespace ek {

// Hash data structure from
// - https://github.com/niklas-ourmachinery/bitsquid-foundation/blob/master

// TODO: make and test Move/Copy, also complete for non-POD values (New/Delete/Move/Copy)

/// Hash from an uint64_t to POD objects. If you want to use a generic key
/// object, use a hash function to map that object to an uint64_t.

/// The hash function stores its data in a "list-in-an-array" where
/// indices are used instead of pointers.

/// When items are removed, the array-list is repacked to always keep
/// it tightly ordered.

template<typename T>
struct Hash {
    struct Entry {
        uint64_t key;
        uint32_t next;
        T value;
    };

    PodArray <uint32_t> _hash;
    PodArray <Entry> _data;

    constexpr Hash() noexcept;
    ~Hash() noexcept {
        ek_core_dbg_dec(EK_CORE_DBG_HASH);
    }

    /// Returns true if the specified key exists in the hash.
    [[nodiscard]]
    bool has(uint64_t key) const;

    /// Returns the value stored for the specified key, or default if the key
    /// does not exist in the hash.
    const T& get(uint64_t key, const T& default_) const;

    const T* tryGet(uint64_t key) const;

    /// Sets the value for the key.
    void set(uint64_t key, const T& value);

    void set(uint64_t key, T&& value);

    /// Removes the key from the hash if it exists.
    void remove(uint64_t key);

    /// Resizes the hash lookup table to the specified size.
    /// (The table will grow automatically when 70 % full.)
    void reserve(uint32_t size);

    /// Remove all elements from the hash.
    void clear();

    /// Returns a pointer to the first entry in the hash table, can be used to
    /// efficiently iterate over the elements (in random order).
    const Entry* begin() const;

    const Entry* end() const;

    [[nodiscard]]
    uint32_t size() const {
        return _data.size();
    }

    [[nodiscard]]
    inline bool empty() const {
        return _data.empty();
    }

    constexpr Hash(Hash&& m) noexcept: _hash{std::move(m._hash)},
                             _data{std::move(m._data)} {
        ek_core_dbg_inc(EK_CORE_DBG_HASH);
    }

//    Hash(const Hash& m) noexcept = delete;
    Hash(const Hash& m) noexcept: _hash{m._hash}, _data{m._data} {
        ek_core_dbg_inc(EK_CORE_DBG_HASH);
    }

    Hash& operator=(Hash&& m) noexcept {
        _hash = std::move(m._hash);
        _data = std::move(m._data);
        return *this;
    }

//    Hash& operator=(const Hash& m) noexcept  = delete;
    Hash& operator=(const Hash& m) noexcept {
        _hash = m._hash;
        _data = m.data;
        return *this;
    }
};

template<typename T>
constexpr Hash<T>::Hash() noexcept : _hash{}, _data{} {
    ek_core_dbg_inc(EK_CORE_DBG_HASH);
}

namespace hash_internal {

const uint32_t END_OF_LIST = 0xFFFFFFFF;

struct FindResult {
    uint32_t hash_i;
    uint32_t data_prev;
    uint32_t data_i;
};

// TODO: maybe wrong naming
inline uint32_t modKey(uint64_t key, uint32_t hashTableSize) {
    const auto hashTableSize64 = static_cast<uint64_t>(hashTableSize);
    const auto keyMod64 = key % hashTableSize64;
    return static_cast<uint32_t>(keyMod64);
}

template<typename T>
void erase(Hash<T>& h, const FindResult& fr);

template<typename T>
FindResult find(const Hash<T>& h, uint64_t key);

template<typename T>
uint32_t add_entry(Hash<T>& h, uint64_t key) {
    typename Hash<T>::Entry e;
    e.key = key;
    e.next = END_OF_LIST;
    uint32_t ei = h._data.size();
    h._data.push_back(e);
    return ei;
}

template<typename T>
void debugValidateList(Hash<T>& h, const FindResult& fr) {
    auto it = h._hash[fr.hash_i];
    while (it != END_OF_LIST) {
        auto next = h._data[it].next;
        EK_ASSERT(next == END_OF_LIST || next < h._data.size());
        it = next;
    }
}

template<typename T>
void erase(Hash<T>& h, const FindResult& fr) {
    if (fr.data_prev == END_OF_LIST) {
        h._hash[fr.hash_i] = h._data[fr.data_i].next;
    } else {
        h._data[fr.data_prev].next = h._data[fr.data_i].next;
    }

    if (fr.data_i + 1 == h._data.size()) {
        h._data.pop_back();
        debugValidateList(h, fr);
        return;
    }

    FindResult last = hash_internal::find(h, h._data[h._data.size() - 1].key);
    if (last.data_prev == END_OF_LIST) {
        h._hash[last.hash_i] = fr.data_i;
    } else {
        EK_ASSERT(h._data[last.data_prev].next == last.data_i);
        h._data[last.data_prev].next = fr.data_i;
    }

    h._data[fr.data_i] = std::move(h._data[h._data.size() - 1]);
    h._data.pop_back();

    debugValidateList(h, fr);
}

template<typename T>
FindResult find(const Hash<T>& h, uint64_t key) {
    FindResult fr;
    fr.hash_i = END_OF_LIST;
    fr.data_prev = END_OF_LIST;
    fr.data_i = END_OF_LIST;

    if (h._hash.empty()) {
        return fr;
    }

    fr.hash_i = modKey(key, h._hash.size());
    fr.data_i = h._hash[fr.hash_i];
    while (fr.data_i != END_OF_LIST) {
        if (h._data[fr.data_i].key == key) {
            return fr;
        }
        fr.data_prev = fr.data_i;
        fr.data_i = h._data[fr.data_i].next;
        EK_ASSERT_R2(fr.data_i < h._data.size() || fr.data_i == END_OF_LIST);
    }
    return fr;
}

template<typename T>
FindResult find(const Hash<T>& h, const typename Hash<T>::Entry* e) {
    FindResult fr;
    fr.hash_i = END_OF_LIST;
    fr.data_prev = END_OF_LIST;
    fr.data_i = END_OF_LIST;

    if (h._hash.empty()) {
        return fr;
    }

    fr.hash_i = modKey(e->key, h._hash.size());
    fr.data_i = h._hash[fr.hash_i];
    while (fr.data_i != END_OF_LIST) {
        if (&h._data[fr.data_i] == e) {
            return fr;
        }
        fr.data_prev = fr.data_i;
        fr.data_i = h._data[fr.data_i].next;
        EK_ASSERT_R2(fr.data_i == END_OF_LIST || fr.data_i < h._data.size());
    }
    return fr;
}

template<typename T>
uint32_t find_or_fail(const Hash<T>& h, uint64_t key) {
    return hash_internal::find(h, key).data_i;
}

template<typename T>
uint32_t find_or_make(Hash<T>& h, uint64_t key) {
    const FindResult fr = hash_internal::find(h, key);
    if (fr.data_i != END_OF_LIST) {
        EK_ASSERT_R2(fr.data_i < h._data.size());
        return fr.data_i;
    }

    uint32_t i = add_entry(h, key);
    EK_ASSERT_R2(i < h._data.size());
    if (fr.data_prev == END_OF_LIST) {
        h._hash[fr.hash_i] = i;
    } else {
        h._data[fr.data_prev].next = i;
    }
    return i;
}

template<typename T>
uint32_t make(Hash<T>& h, uint64_t key) {
    const FindResult fr = find(h, key);
    EK_ASSERT_R2(fr.data_i == END_OF_LIST);
    const uint32_t i = add_entry(h, key);

    if (fr.data_prev == END_OF_LIST) {
        h._hash[fr.hash_i] = i;
    } else {
        h._data[fr.data_prev].next = i;
    }

    h._data[i].next = fr.data_i;
    return i;
}

template<typename T>
void find_and_erase(Hash<T>& h, uint64_t key) {
    const FindResult fr = find(h, key);
    if (fr.data_i != END_OF_LIST) {
        erase(h, fr);
    }
}

template<typename T>
void insertMulti(Hash<T>& h, uint64_t key, const T& value);

template<typename T>
void rehash(Hash<T>& h, uint32_t new_size) {
    Hash<T> nh{};
    nh._hash.resize(new_size);
    nh._data.reserve(h._data.size());
    for (uint32_t i = 0; i < new_size; ++i) {
        nh._hash[i] = END_OF_LIST;
    }
    for (uint32_t i = 0; i < h._data.size(); ++i) {
        const typename Hash<T>::Entry& e = h._data[i];
        insertMulti(nh, e.key, e.value);
    }

    h = std::move(nh);
//    log_trace("HASH growing to %u", new_size);
//    log_trace("HASH growing to hash.size = %u", h._hash.size());
//    log_trace("HASH growing to data.size = %u", h._data.size());
}

template<typename T>
bool full(const Hash<T>& h) {
    const uint32_t hash_size = h._hash.size();
    const uint32_t data_size_min = (uint32_t)(0.7f * (float)hash_size);
    const uint32_t data_size = h._data.size();
    return data_size >= data_size_min;
//    if(UNLIKELY(data_size >= data_size_min)) {
//        log_trace("hash_size: %u", hash_size);
//        log_trace("data_size_min: %u", data_size_min);
//        log_trace("data_size: %u", data_size);
//        return true;
//    }
//    else {
//        return false;
//    }
}

template<typename T>
void grow(Hash<T>& h) {
    const uint32_t new_size = h._data.size() * 2 + 10;
    rehash(h, new_size);
}

template<typename T>
void insertMulti(Hash<T>& h, uint64_t key, const T& value) {
    if (h._hash.empty()) {
        grow(h);
    }
    const uint32_t i = hash_internal::make(h, key);
    h._data[i].value = value;
    if (full(h)) {
        grow(h);
    }
}
}

template<typename T>
bool Hash<T>::has(uint64_t key) const {
    return hash_internal::find_or_fail(*this, key) != hash_internal::END_OF_LIST;
}

template<typename T>
const T& Hash<T>::get(uint64_t key, const T& default_) const {
    const uint32_t i = hash_internal::find_or_fail(*this, key);
    return i == hash_internal::END_OF_LIST ? default_ : _data[i].value;
}

template<typename T>
const T* Hash<T>::tryGet(uint64_t key) const {
    const uint32_t i = hash_internal::find_or_fail(*this, key);
    return i != hash_internal::END_OF_LIST ? &_data[i].value : nullptr;
}

template<typename T>
void Hash<T>::set(uint64_t key, const T& value) {
    if (_hash.empty()) {
        hash_internal::grow(*this);
    }

    const uint32_t i = hash_internal::find_or_make(*this, key);
    _data[i].value = value;
    if (hash_internal::full(*this)) {
        hash_internal::grow(*this);
    }
}

template<typename T>
void Hash<T>::set(uint64_t key, T&& value) {
    if (_hash.empty()) {
        hash_internal::grow(*this);
    }

    const uint32_t i = hash_internal::find_or_make(*this, key);
    _data[i].value = (T&&) value;
    if (hash_internal::full(*this)) {
        hash_internal::grow(*this);
    }
}

template<typename T>
inline void Hash<T>::remove(uint64_t key) {
    hash_internal::find_and_erase(*this, key);
}

template<typename T>
inline void Hash<T>::reserve(uint32_t size) {
    hash_internal::rehash(*this, size);
}

template<typename T>
void Hash<T>::clear() {
    _data.clear();
    for(uint32_t i = 0; i < _hash.size(); ++i) {
        _hash.buffer[i] = hash_internal::END_OF_LIST;
    }
}

template<typename T>
inline const typename Hash<T>::Entry* Hash<T>::begin() const {
    return _data.begin();
}

template<typename T>
inline const typename Hash<T>::Entry* Hash<T>::end() const {
    return _data.end();
}

}