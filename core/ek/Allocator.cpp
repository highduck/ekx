#include "Allocator.hpp"
#include "assert.hpp"
#include "util/StaticStorage.hpp"
#include <cstdlib>

#ifdef EK_ALLOCATION_TRACKER

#ifdef EK_ALLOCATION_TRACKER_STATS

#include "ds/Hash.hpp"

#endif // EK_ALLOCATION_TRACKER_STATS

#include <Tracy.hpp>

#ifdef TRACY_ENABLE
#define EK_TRACE_ALLOC(ptr,sz,name) TracyAllocNS(ptr,sz,3,name)
#define EK_TRACE_FREE(ptr,name)     TracyFreeNS(ptr,3,name)
#else
#define EK_TRACE_ALLOC(ptr, sz, name) ((void)0)
#define EK_TRACE_FREE(ptr, name)     ((void)0)
#endif // TRACY_ENABLE

#endif // EK_ALLOCATION_TRACKER

/**
 * standard allocator: malloc / free - no alignment, no tracking, just wraps system-dependent memory allocator,
 * aligned allocator(allocator): wraps any allocator and align blocks
 */

// std versions
//inline void* ek_aligned_alloc(size_t alignment, size_t size) {
//#if defined(__ANDROID__) || defined(__APPLE__)
//    // `aligned_alloc` is not available for iOS < 13
//    void* buffer = nullptr;
//    posix_memalign(&buffer, alignment, size);
//    return buffer;
//#else
//    return aligned_alloc(alignment, size);
//#endif
//}

// own general implementation to avoid memalign/aligned_alloc availability across platforms

// Number of bytes we're using for storing
// the aligned pointer offset
typedef uint16_t offset_t;
#define PTR_OFFSET_SZ sizeof(offset_t)


namespace ek {

inline static uint32_t upperPowerOfTwo(uint32_t v) {
    --v;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    return v;
}

// Aligns p to the specified alignment by moving it forward if necessary
// and returns the result.
inline void* alignForward(void* ptr, uint32_t align) {
    auto p = uintptr_t(ptr);
    const uint32_t mod = p % align;
    if (mod) {
        p = p + align - mod;
    }
    return (void*) p;
}

inline uint32_t sizeWithPadding(uint32_t size, uint32_t align) {
    return size + align;
}

void fillMemoryDebug(void* ptr, size_t sz) {
#ifndef NDEBUG
    auto* data = static_cast<uint8_t*>(ptr);
    for (size_t i = 0; i < sz; ++i) {
        data[i] = 0xCC;
    }
#endif
}

SystemAllocator::SystemAllocator(const char* label_) noexcept:
        label{label_} {

}

SystemAllocator::~SystemAllocator() = default;

void* SystemAllocator::alloc(uint32_t size, uint32_t align) {
    void* ptr = std::malloc(size);
#ifdef EK_ALLOCATION_TRACKER
    EK_TRACE_ALLOC(ptr, size, label);
#endif
    ek::fillMemoryDebug(ptr, size);
    return ptr;
}

void SystemAllocator::dealloc(void* ptr) {
#ifdef EK_ALLOCATION_TRACKER
    EK_TRACE_FREE(ptr, label);
#endif
    std::free(ptr);
}

#ifdef EK_ALLOCATION_TRACKER

class AllocationTracker {
public:
    const char* label;

    size_t currentAllocations = 0;
    size_t currentSizeRequested = 0; // only for stats
    size_t currentSizeTotal = 0; // only for stats

    size_t allAllocations = 0;
    size_t allSizeRequested = 0;
    size_t allSizeTotal = 0;

    int children = 0;
    bool alive = true;

#ifdef EK_ALLOCATION_TRACKER_STATS
    struct Rec {
        uint32_t sizeUsed = 0;
        uint32_t sizeTotal = 0;
    };

    inline static Rec Invalid{0xFFFFFFFF, 0x0};

    Hash<Rec> _map{memory::systemAllocator};
#endif // EK_ALLOCATION_TRACKER_STATS

    explicit AllocationTracker(const char* label_) : label{label_} {
    }

    ~AllocationTracker() {
        alive = false;

        // all proxy wrappers should be destructed first (wrong de-initialization order)
        EK_ASSERT(children == 0);

        // all allocations should be deallocated first (leak)
        EK_ASSERT(currentAllocations == 0);
        EK_ASSERT(currentSizeRequested == 0);
        EK_ASSERT(currentSizeTotal == 0);

#ifdef EK_ALLOCATION_TRACKER_STATS
        EK_ASSERT(_map.empty());
#endif // EK_ALLOCATION_TRACKER_STATS
    }

    void onAllocation(void* ptr, uint32_t size, uint32_t sizeTotal) {
        EK_ASSERT(ptr != nullptr);
        EK_ASSERT(size > 0);
        EK_ASSERT(sizeTotal > 0);
        EK_ASSERT(size <= sizeTotal);

        ++currentAllocations;
        ++allAllocations;
        allSizeRequested += size;
        allSizeTotal += sizeTotal;

#ifdef EK_ALLOCATION_TRACKER_STATS
        currentSizeRequested += size;
        currentSizeTotal += sizeTotal;

        // should not be tracked before
        const auto id = (uint64_t) ptr;

        /// address should not be occupied
        EK_ASSERT(!_map.has(id));

        _map.set(id, Rec{size, sizeTotal});

        /// check map implementation
        EK_ASSERT(_map.has(id));
        EK_ASSERT(_map.get(id, Invalid).sizeTotal == sizeTotal);

#endif // EK_ALLOCATION_TRACKER_STATS

        EK_TRACE_ALLOC(ptr, sizeTotal, label);
    }

    void onFree(void* ptr) {
        // some libraries just free nullptr and expect noop
        if (ptr == nullptr) {
            return;
        }
        EK_ASSERT_R2(currentAllocations > 0);

        --currentAllocations;

#ifdef EK_ALLOCATION_TRACKER_STATS
        const auto id = (uint64_t) ptr;
        /// address should not allocated before
        EK_ASSERT(_map.has(id));
        const auto& rec = _map.get(id, Invalid);
        currentSizeRequested -= rec.sizeUsed;
        currentSizeTotal -= rec.sizeTotal;
        _map.remove(id);
        /// something wrong with map implementation
        EK_ASSERT_R2(!_map.has(id));

#endif // EK_ALLOCATION_TRACKER_STATS

        EK_TRACE_FREE(ptr, label);
    }
};

#endif // EK_ALLOCATION_TRACKER

inline uintptr_t align_up(uintptr_t num, uintptr_t align) {
    return (((num) + ((align) - 1)) & ~((align) - 1));
}

AlignedAllocator::AlignedAllocator(Allocator& allocator_, const char* label_) noexcept:
        allocator{allocator_},
        label{label_} {
#ifdef EK_ALLOCATION_TRACKER
    tracker = new AllocationTracker(label_);
    if (allocator.tracker != nullptr) {
        ++(allocator.tracker->children);
    }
#endif // EK_ALLOCATION_TRACKER
}

AlignedAllocator::~AlignedAllocator() {
#ifdef EK_ALLOCATION_TRACKER
    if (allocator.tracker != nullptr) {
        --(allocator.tracker->children);
    }
    delete tracker;
#endif // EK_ALLOCATION_TRACKER
}

void* AlignedAllocator::alloc(uint32_t size, uint32_t align) {
    void* ptr = nullptr;

    // do we need min alignment or just 1?
    // const uint32_t MinAlign = EK_SIZEOF_U32(void**);

    EK_ASSERT(align > 0);

    // TODO: maybe contract requirement?
    if ((align & (align - 1)) != 0) {
        align = upperPowerOfTwo(align);
    }

    // We want it to be a power of two since
    // align_up operates on powers of two
    EK_ASSERT((align & (align - 1)) == 0);

    if (align && size) {
        /*
         * We know we have to fit an offset value
         * We also allocate extra bytes to ensure we
         * can meet the alignment
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        uint32_t sizeTotal = size + hdr_size;
        void* p = allocator.alloc(sizeTotal, 0);

        if (p) {
#ifdef EK_ALLOCATION_TRACKER
            ek::fillMemoryDebug(p, sizeTotal);
            tracker->onAllocation(p, size, sizeTotal);
#endif // EK_ALLOCATION_TRACKER

            /*
             * Add the offset size to malloc's pointer
             * (we will always store that)
             * Then align the resulting value to the
             * target alignment
             */
            ptr = (void*) align_up(((uintptr_t) p + PTR_OFFSET_SZ), align);

            // Calculate the offset and store it
            // behind our aligned pointer
            offset_t* hdr = ((offset_t*) ptr) - 1;
            *hdr = (offset_t) ((uintptr_t) ptr - (uintptr_t) p);
        } // else NULL, could not malloc
    } //else NULL, invalid arguments

    return ptr;
}

void AlignedAllocator::dealloc(void* ptr) {
    if (ptr != nullptr) {

        /*
        * Walk backwards from the passed-in pointer
        * to get the pointer offset. We convert to an offset_t
        * pointer and rely on pointer math to get the data
        */
        offset_t offset = *((offset_t*) ptr - 1);

        /*
        * Once we have the offset, we can get our
        * original pointer and call free
        */
        void* p = (void*) ((uint8_t*) ptr - offset);

#ifdef EK_ALLOCATION_TRACKER
        tracker->onFree(p);
#endif // EK_ALLOCATION_TRACKER

        allocator.dealloc(p);
    }
}

ProxyAllocator::ProxyAllocator(Allocator& allocator_, const char* label_) noexcept:
        allocator{allocator_},
        label{label_} {

#ifdef EK_ALLOCATION_TRACKER
    tracker = new AllocationTracker(label_);
    if (allocator.tracker != nullptr) {
        ++(allocator.tracker->children);
    }
#endif // EK_ALLOCATION_TRACKER
}

ProxyAllocator::ProxyAllocator(const char* heapLabel) noexcept:
        ProxyAllocator{memory::systemAllocator, heapLabel} {

}

ProxyAllocator::~ProxyAllocator() {
#ifdef EK_ALLOCATION_TRACKER
    if (allocator.tracker != nullptr) {
        --(allocator.tracker->children);
    }
    delete tracker;
#endif // EK_ALLOCATION_TRACKER
}

void* ProxyAllocator::alloc(uint32_t size, uint32_t align) {
    void* ptr = allocator.alloc(size, align);
#ifdef EK_ALLOCATION_TRACKER
    tracker->onAllocation(ptr, size, size);
#endif // EK_ALLOCATION_TRACKER
    return ptr;
}

void ProxyAllocator::dealloc(void* ptr) {
#ifdef EK_ALLOCATION_TRACKER
    tracker->onFree(ptr);
#endif // EK_ALLOCATION_TRACKER
    allocator.dealloc(ptr);
}

namespace memory {

SystemAllocator systemAllocator{"system"};

class Globals {
public:
    ProxyAllocator std{systemAllocator, "global std"};
    AlignedAllocator aligned{systemAllocator, "global aligned"};
};

static StaticStorage<Globals> ssGlobals;
ProxyAllocator& stdAllocator = ssGlobals.ptr()->std;
AlignedAllocator& alignedAllocator = ssGlobals.ptr()->aligned;

void initialize() {
    ssGlobals.initialize();
}

void shutdown() {
    ssGlobals.shutdown();
}

void copy(void* dest, const void* src, uint32_t size) {
    EK_ASSERT_R2(dest != nullptr);
    EK_ASSERT_R2(src != nullptr);
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);
    for (size_t i = 0; i < size; ++i) {
        d[i] = s[i];
    }
}

void clear(void* ptr, uint32_t size) {
    EK_ASSERT_R2(ptr != nullptr);
    char* it = static_cast<char*>(ptr);
    const char* end = it + size;
    while (it != end) {
        *(it++) = 0;
    }
}

void* reallocate(Allocator& allocator, void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align) {
    void* ptrNew = allocator.alloc(newSize, align);
    if (ptr != nullptr) {
        copy(ptrNew, ptr, oldSizeToCopy);
        allocator.dealloc(ptr);
    }
    return ptrNew;
}

}

}

#ifdef EK_ALLOCATION_TRACKER

void* operator new(size_t sz) {
    if (sz == 0) {
        ++sz;
    }

    if (void* ptr = std::malloc(sz)) {
        EK_TRACE_ALLOC(ptr, sz, "c++ new");
        ek::fillMemoryDebug(ptr, sz);
        return ptr;
    }

    abort();
}

void operator delete(void* ptr) noexcept {
    EK_TRACE_FREE(ptr, "c++ new");
    std::free(ptr);
}

#endif // EK_ALLOCATION_TRACKER
