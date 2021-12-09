#pragma once

#include "MemoryProfiler.hpp"
#include "Widgets.hpp"

namespace ek {

inline double toMB(uint64_t bytes) {
    return static_cast<double>(bytes / 1024) / 1024.0;
}

inline double toKB(uint64_t bytes) {
    return static_cast<double>(bytes) / 1024.0;
}

void drawAllocationsList() {
    // TODO:
//    AllocationInfo data[10000];
//    const auto count = allocator.getAllocationsInfo(10000, data);
//    for (uint32_t i = 0; i < count; ++i) {
//        auto& info = data[i];
//        ImGui::Text("%0.2lf %0.2lf", toKB(info.sizeTotal), toKB(info.sizeUsed));
//        for(unsigned j = 0; j < AllocationInfo::MaxStackDepth; ++j) {
//            if(info.stack[j]) {
//                ImGui::BulletText("[%u] %s", j, info.stack[j]);
//            }
//        }
//    }
}

void drawAllocatorMemorySpan() {
    // TODO:
//
//    uint64_t MEMORY_PROFILER_BLOCK[4096];
//    auto count = readAllocationMap(allocator, MEMORY_PROFILER_BLOCK, 4096);
//    uint32_t i = 0;
//    uint64_t min = 0xFFFFFFFFFFFFFFFFu;
//    uint64_t max = 0;
//    while (i < count) {
//        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
//        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
//        if (pos < min) {
//            min = pos;
//        }
//        if (end > max) {
//            max = pos;
//        }
//    }
//    i = 0;
//    constexpr uint32_t MapSize = 1024 * 128;
//    char BB[MapSize];
//    memset(BB, '_', MapSize);
//    uint32_t len = (max - min) / 1024;
//    if (len > MapSize) {
//        len = MapSize;
//    }
//
//    while (i < count) {
//        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
//        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
//        uint32_t i0 = (pos - min) / 1024;
//        uint32_t i1 = (end - min) / 1024;
//        for (uint32_t x = i0; x < i1 && x < MapSize; ++x) {
//            BB[x] = '#';
//        }
//    }
//
//    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
//    i = 0;
//    const uint32_t NumKilosPerLine = 80;
//    uint32_t skipped = 0;
//    while (i < len) {
//        uint32_t txtLen = 0;
//        uint32_t occupied = 0;
//        while (txtLen < NumKilosPerLine && (i + txtLen) < len) {
//            if (BB[i + txtLen] == '#') {
//                ++occupied;
//            }
//            ++txtLen;
//        }
//        if (occupied > 0) {
//            if (skipped > 0) {
//                ImGui::Text("-/- Skipped %0.2f MB -/-", (float) (skipped * NumKilosPerLine) / 1024.0f);
//            }
//            skipped = 0;
//            ImGui::TextUnformatted(BB + i, BB + i + txtLen);
//        } else {
//            ++skipped;
//        }
//        i += txtLen;
//    }
//    if (skipped > 0) {
//        ImGui::Text("-/- Skipped %0.2f MB -/-", (float) (skipped * NumKilosPerLine) / 1024.0f);
//    }
//    ImGui::PopFont();
}

void DrawMemoryBlock() {

    // TODO:
//    auto* stats = allocator.getStats();
//    if (!stats) {
//        return;
//    }
//
//    static const char* STATS_MODES[3] = {"Current", "Peak", "All Time"};
//    if (ImGui::TreeNode("memory_map", "Memory Span: %0.1lf MB", toMB(allocator.getSpanSize()))) {
//        drawAllocatorMemorySpan(allocator);
//        ImGui::TreePop();
//    }
//    if (ImGui::TreeNode("allocations", "Allocations")) {
//        drawAllocationsList(allocator);
//        ImGui::TreePop();
//    }
//    if (ImGui::BeginTable("stats", 4)) {
//        ImGui::TableSetupColumn("Measurement");
//        ImGui::TableSetupColumn("# Allocations");
//        ImGui::TableSetupColumn("Effective (MB)");
//        ImGui::TableSetupColumn("Total (MB)");
//        ImGui::TableHeadersRow();
//
//        for (int i = 0; i < 3; ++i) {
//            ImGui::TableNextRow();
//            ImGui::TableSetColumnIndex(0);
//            ImGui::TextUnformatted(STATS_MODES[i]);
//            ImGui::TableSetColumnIndex(1);
//            ImGui::Text("%u", stats->allocations[i]);
//            ImGui::TableSetColumnIndex(2);
//            ImGui::Text("%0.2lf", toMB(stats->memoryEffective[i]));
//            ImGui::TableSetColumnIndex(3);
//            ImGui::Text("%0.2lf", toMB(stats->memoryAllocated[i]));
//        }
//        ImGui::EndTable();
//    }


}

void DrawAllocatorsTree() {
    // TODO:
//    ImGui::PushID(&allocator);
//    auto* info = allocator.getStats();
//    if (info) {
//        if (ImGui::TreeNode(info->label)) {
//            DrawMemoryBlock(allocator);
//            auto* it = info->children;
//            while (it) {
//                auto* childInfo = it->getStats();
//                if (childInfo) {
//                    DrawAllocatorsTree(*it);
//                    it = childInfo->next;
//                } else {
//                    break;
//                }
//            }
//            ImGui::TreePop();
//        }
//    } else {
//        ImGui::Text("Allocator is not debuggable");
//    }
//    ImGui::PopID();
}

void drawECSMemoryStats() {
    ImGui::Text("ECS World Struct Size: %lu", sizeof(ecs::World));
    uint32_t totalUsed = 0;
    uint32_t totalReserved = 0;
    uint32_t totalLookups = 0;
    for (auto* header : ecs::the_world.components) {
        if (header) {
            const char* name = header->name;
            auto lookupSize = sizeof(ecs::EntityLookup);
            auto h2eSizeReserved = header->handleToEntity.capacity() * sizeof(ecs::EntityIndex);
            auto h2eSizeUsed = header->handleToEntity.size() * sizeof(ecs::EntityIndex);
            auto dataSizeUsed = header->count() * header->storageElementSize;
            auto dataSizeReserved = dataSizeUsed;
            totalLookups += lookupSize;
            totalUsed += lookupSize + h2eSizeUsed + dataSizeUsed;
            totalReserved += lookupSize + h2eSizeReserved + dataSizeReserved;
            if (ImGui::TreeNode(header, "#%u. %s | %0.2lf KB",
                                header->typeId,
                                name ? name : "Component",
                                toKB(h2eSizeReserved + dataSizeReserved))) {
                ImGui::Text("Handle-2-Entity Array: %u / %u | %0.2lf of %0.2lf KB", header->handleToEntity.size(),
                            header->handleToEntity.capacity(),
                            toKB(h2eSizeUsed), toKB(h2eSizeReserved));
                ImGui::TreePop();
            }
        }
    }
    ImGui::Text("ECXX Lookups: %0.2lf KB", toKB(totalLookups));
    ImGui::Text("ECXX Used: %0.2lf MB", toMB(totalUsed));
    ImGui::Text("ECXX Reserved: %0.2lf MB", toMB(totalReserved));
}

void MemoryProfiler::onDraw() {
    if (ImGui::BeginTabBar("Memory Stats")) {
        if (ImGui::BeginTabItem("Allocators")) {
            // TODO:
            ImGui::Text("TODO");
            //DrawAllocatorsTree(memory::systemAllocator);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ECS Memory")) {
            drawECSMemoryStats();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}