#include "Allocator.hpp"
#include "util/StaticStorage.hpp"
#include <cstdlib>
#include <cstring>
#include "debug.hpp"

#ifdef EK_ALLOCATION_TRACKER

#ifdef EK_ALLOCATION_TRACKER_STATS

#include "ds/Hash.hpp"

#endif // EK_ALLOCATION_TRACKER_STATS

#include <Tracy.hpp>

#ifdef TRACY_ENABLE
#define EK_TRACE_ALLOC(ptr, sz, name) TracyAllocNS(ptr,sz,3,name)
#define EK_TRACE_FREE(ptr, name)     TracyFreeNS(ptr,3,name)
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

#ifdef EK_ALLOCATION_TRACKER

void onDebugAllocatorDestroy(Allocator& allocator) {
    auto& stats = allocator.stats;
    // all proxy wrappers should be destructed first (wrong de-initialization order)
    EK_ASSERT(stats.children == nullptr);

    // all allocations should be deallocated first (leak)
    EK_ASSERT(stats.allocations[AllocatorStats::Current] == 0);
    EK_ASSERT(stats.memoryEffective[AllocatorStats::Current] == 0);
    EK_ASSERT(stats.memoryAllocated[AllocatorStats::Current] == 0);
}

void onDebugAllocation(Allocator& allocator, void* ptr, uint32_t size, uint32_t sizeTotal) {
    EK_ASSERT(ptr != nullptr);
    EK_ASSERT(size > 0);
    EK_ASSERT(sizeTotal > 0);
    EK_ASSERT(size <= sizeTotal);

    auto& stats = allocator.stats;
    stats.allocations[AllocatorStats::Current] += 1;
    stats.allocations[AllocatorStats::AllTime] += 1;
    stats.memoryEffective[AllocatorStats::AllTime] += size;
    stats.memoryAllocated[AllocatorStats::AllTime] += sizeTotal;

#ifdef EK_ALLOCATION_TRACKER_STATS
    stats.memoryEffective[AllocatorStats::Current] += size;
    stats.memoryAllocated[AllocatorStats::Current] += sizeTotal;

    if (stats.allocations[AllocatorStats::Current] > stats.allocations[AllocatorStats::Peak]) {
        stats.allocations[AllocatorStats::Peak] = stats.allocations[AllocatorStats::Current];
    }
    if (stats.memoryEffective[AllocatorStats::Current] > stats.memoryEffective[AllocatorStats::Peak]) {
        stats.memoryEffective[AllocatorStats::Peak] = stats.memoryEffective[AllocatorStats::Current];
    }
    if (stats.memoryAllocated[AllocatorStats::Current] > stats.memoryAllocated[AllocatorStats::Peak]) {
        stats.memoryAllocated[AllocatorStats::Peak] = stats.memoryAllocated[AllocatorStats::Current];
    }

    // should not be tracked before
    const auto id = (uint64_t) ptr;

    /// TODO: address should not be occupied
    //EK_ASSERT(!_map.has(id));

    AllocationInfo info;
    info.id = id;
    info.sizeUsed = size;
    info.sizeTotal = sizeTotal;
    for (unsigned i = 0; i < AllocationInfo::MaxStackDepth; ++i) {
        if(i < DebugRecords::labelsSize) {
            info.stack[i] = DebugRecords::labelsStack[DebugRecords::labelsSize - i - 1];
        }
        else {
            info.stack[i] = nullptr;
        }
    }
    allocator.records.add(info);

    /// TODO: check map implementation
//    EK_ASSERT(_map.has(id));
//    EK_ASSERT(_map.get(id, Invalid).sizeTotal == sizeTotal);
//    _freedMap.remove(id);

#endif // EK_ALLOCATION_TRACKER_STATS

    EK_TRACE_ALLOC(ptr, sizeTotal, stats.label);
}

void onDebugFree(Allocator& allocator, void* ptr) {
    // some libraries just free nullptr and expect noop
    if (ptr == nullptr) {
        return;
    }
    auto& stats = allocator.stats;
#ifdef EK_ALLOCATION_TRACKER_STATS
    const auto id = (uint64_t) ptr;
    // TODO: check double free
    //EK_ASSERT(!_freedMap.has(id));
    /// TODO:  address should not allocated before
    //EK_ASSERT(_map.has(id));
    auto* rec = allocator.records.find(id);
    if (rec) {
        stats.memoryEffective[AllocatorStats::Current] -= rec->sizeUsed;
        stats.memoryAllocated[AllocatorStats::Current] -= rec->sizeTotal;
        allocator.records.removeAt(rec - allocator.records.data);
        // TODO:
        // _freedMap.set(id, rec);
    }
        /// TODO: something wrong with map implementation
        //EK_ASSERT_R2(!_map.has(id));

#endif // EK_ALLOCATION_TRACKER_STATS

    EK_ASSERT_R2(stats.allocations[AllocatorStats::Current] > 0);
    stats.allocations[AllocatorStats::Current] -= 1;

    EK_TRACE_FREE(ptr, stats.label);
}

#endif // EK_ALLOCATION_TRACKER

SystemAllocator::SystemAllocator(const char* label_) noexcept:
        Allocator{label_} {

}

SystemAllocator::~SystemAllocator() = default;

void* SystemAllocator::alloc(uint32_t size, uint32_t align) {
    // TODO:
    (void)(align);

    void* ptr = std::malloc(size);
#ifdef EK_ALLOCATION_TRACKER
    EK_TRACE_ALLOC(ptr, size, stats.label);
    onDebugAllocation(*this, ptr, size, size);
#endif // EK_ALLOCATION_TRACKER

#ifdef EK_INIT_CC_MEMORY
    memset(ptr, 0xCC, size);
#endif // EK_INIT_CC_MEMORY

    return ptr;
}

void SystemAllocator::dealloc(void* ptr) {
#ifdef EK_ALLOCATION_TRACKER
    EK_TRACE_FREE(ptr, stats.label);
    onDebugFree(*this, ptr);
#endif
    std::free(ptr);
}

inline uintptr_t align_up(uintptr_t num, uintptr_t align) {
    return (((num) + ((align) - 1)) & ~((align) - 1));
}

Allocator::Allocator(const char* label_) {
#ifdef EK_ALLOCATION_TRACKER
    stats.label = label_;
#endif
}

Allocator::~Allocator() = default;

void Allocator::addChild(Allocator& child) {
#ifdef EK_ALLOCATION_TRACKER
    child.stats.next = stats.children;
    stats.children = &child;
#endif
}

void Allocator::removeChild(Allocator& child) {
#ifdef EK_ALLOCATION_TRACKER
    auto* it = stats.children;
    if (it == &child) {
        stats.children = child.stats.next;
        return;
    }

    while (it) {
        if (it->stats.next == &child) {
            it->stats.next = it->stats.next->stats.next;
            return;
        }
    }
#endif
}

uint32_t Allocator::getAllocationsInfo(uint32_t maxCount, AllocationInfo* outData) const {
    if (!outData) {
        return 0;
    }
    uint32_t i = 0;
#ifdef EK_ALLOCATION_TRACKER
    while (i < maxCount && i < records.size) {
        outData[i] = records.data[i];
        ++i;
    }
#endif // EK_ALLOCATION_TRACKER
    return i;
}

void Allocator::pushDebugLabel(const char* label) {
#ifdef EK_ALLOCATION_TRACKER
    DebugRecords::pushDebugLabel(label);
#endif
}

void Allocator::popDebugLabel() {
#ifdef EK_ALLOCATION_TRACKER
    DebugRecords::popDebugLabel();
#endif
}

uint64_t Allocator::getSpanSize() const {
    uint64_t result = 0;
#ifdef EK_ALLOCATION_TRACKER
    // update span
    uint64_t min = 0xFFFFFFFFFFFFFFFFllu;
    uint64_t max = 0x0llu;
    auto* data = records.data;
    for (uint32_t i = 0; i < records.size; ++i) {
        auto& record = data[i];
        const uint64_t ptr = record.id;
        if (ptr < min) min = ptr;
        if (ptr > max) max = ptr;
    }
    result = max - min;
#endif
    return result;
}

AlignedAllocator::AlignedAllocator(Allocator& allocator_, const char* label_) noexcept:
        Allocator(label_),
        allocator{allocator_} {
#ifdef EK_ALLOCATION_TRACKER
    stats.label = label_;
#endif // EK_ALLOCATION_TRACKER
    allocator.addChild(*this);
}

AlignedAllocator::~AlignedAllocator() {
#ifdef EK_ALLOCATION_TRACKER
    onDebugAllocatorDestroy(*this);
#endif // EK_ALLOCATION_TRACKER
    allocator.removeChild(*this);
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
            memset(p, 0xCC, sizeTotal);
            onDebugAllocation(*this, p, size, sizeTotal);
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
        onDebugFree(*this, p);
#endif // EK_ALLOCATION_TRACKER

        allocator.dealloc(p);
    }
}

ProxyAllocator::ProxyAllocator(Allocator& allocator_, const char* label_) noexcept:
        Allocator(label_),
        allocator{allocator_} {
    allocator.addChild(*this);
}

ProxyAllocator::ProxyAllocator(const char* heapLabel) noexcept:
        ProxyAllocator{memory::systemAllocator, heapLabel} {

}

ProxyAllocator::~ProxyAllocator() {
    allocator.removeChild(*this);
}

void* ProxyAllocator::alloc(uint32_t size, uint32_t align) {
    void* ptr = allocator.alloc(size, align);
#ifdef EK_ALLOCATION_TRACKER
    onDebugAllocation(*this, ptr, size, size);
#endif // EK_ALLOCATION_TRACKER
    return ptr;
}

void ProxyAllocator::dealloc(void* ptr) {
#ifdef EK_ALLOCATION_TRACKER
    onDebugFree(*this, ptr);
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
    for (uint32_t i = 0; i < size; ++i) {
        d[i] = s[i];
    }
}

void clear(void* ptr, uint32_t size) {
    EK_ASSERT_R2(ptr != nullptr);
    memset(ptr, 0, size);
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

uint32_t readAllocationMap(Allocator& allocator, uint64_t* rle, uint32_t blockSize) {
    uint32_t p = 0;
#ifdef EK_ALLOCATION_TRACKER
    auto& records = allocator.records;
    auto* data = records.data;
    for (uint32_t i = 0; i < records.size; ++i) {
        auto& record = data[i];
        rle[p++] = record.id;
        rle[p++] = record.sizeTotal;
    }
#endif
    return p;
}

}

#ifdef EK_ALLOCATION_TRACKER

void* operator new(size_t sz) {
    if (sz == 0) {
        ++sz;
    }

    if (void* ptr = std::malloc(sz)) {
        EK_TRACE_ALLOC(ptr, sz, "c++ new");
        memset(ptr, 0xCC, sz);
        return ptr;
    }

    abort();
}

void operator delete(void* ptr) noexcept {
    EK_TRACE_FREE(ptr, "c++ new");
    std::free(ptr);
}

#endif // EK_ALLOCATION_TRACKER
