#pragma once

#include "MemoryProfiler.hpp"
#include "Widgets.hpp"
#include <ek/Allocator.hpp>

namespace ek {

static uint64_t MEMORY_PROFILER_BLOCK[4096];

inline double toMB(uint64_t bytes) {
    return static_cast<double>(bytes / 1024) / 1024.0;
}

inline double toKB(uint64_t bytes) {
    return static_cast<double>(bytes) / 1024.0;
}

void drawAllocatorMemorySpan(Allocator& allocator) {

    auto count = readAllocationMap(allocator, MEMORY_PROFILER_BLOCK, 4096);
    uint32_t i = 0;
    uint64_t min = 0xFFFFFFFFFFFFFFFFu;
    uint64_t max = 0;
    while (i < count) {
        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
        if (pos < min) {
            min = pos;
        }
        if (pos > max) {
            max = pos;
        }
    }
    i = 0;
    char BB[1024 * 128];
    memset(BB, '_', 1024 * 128);
    uint32_t len = (max - min) / 1024;
    if(len > 1024 * 128) {
        len = 1024 * 128;
    }

    while (i < count) {
        uint64_t pos = MEMORY_PROFILER_BLOCK[i++];
        uint64_t end = pos + MEMORY_PROFILER_BLOCK[i++];
        uint32_t i0 = (pos - min) / 1024;
        uint32_t i1 = (end - min) / 1024;
        for(uint32_t x = i0; x < i1; ++x) {
            BB[x] = '#';
        }
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    i = 0;
    const uint32_t NumKilosPerLine = 80;
    uint32_t skipped = 0;
    while(i < len) {
        uint32_t txtLen = 0;
        uint32_t occupied = 0;
        while(txtLen < NumKilosPerLine && (i + txtLen) < len) {
            if(BB[i + txtLen] == '#') {
                ++occupied;
            }
            ++txtLen;
        }
        if(occupied > 0) {
            if(skipped > 0) {
                ImGui::Text("-/- Skipped %0.2f MB -/-", (float)(skipped * NumKilosPerLine) / 1024.0f);
            }
            skipped = 0;
            ImGui::TextUnformatted(BB + i, BB + i + txtLen);
        }
        else {
            ++skipped;
        }
        i += txtLen;
    }
    if(skipped > 0) {
        ImGui::Text("-/- Skipped %0.2f MB -/-", (float)(skipped * NumKilosPerLine) / 1024.0f);
    }
    ImGui::PopFont();
}

void DrawMemoryBlock(Allocator& allocator) {
    auto stats = allocator.getStatistics();
    static const char* STATS_MODES[3] = {"Current", "Peak", "All Time"};
    if(ImGui::TreeNode("memory_map", "Memory Span: %0.1lf MB", toMB(stats.span))) {
        drawAllocatorMemorySpan(allocator);
        ImGui::TreePop();
    }
    if (ImGui::BeginTable("stats", 4)) {
        ImGui::TableSetupColumn("Measurement");
        ImGui::TableSetupColumn("# Allocations");
        ImGui::TableSetupColumn("Effective (MB)");
        ImGui::TableSetupColumn("Total (MB)");
        ImGui::TableHeadersRow();

        for (int i = 0; i < 3; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(STATS_MODES[i]);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%u", stats.allocations[i]);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%0.2lf", toMB(stats.memoryEffective[i]));
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%0.2lf", toMB(stats.memoryAllocated[i]));
        }
        ImGui::EndTable();
    }


}

void DrawAllocatorsTree(Allocator& allocator) {
    ImGui::PushID(&allocator);
    if (ImGui::TreeNode(allocator.label)) {
        DrawMemoryBlock(allocator);
        auto* it = allocator.children;
        while (it) {
            DrawAllocatorsTree(*it);
            it = it->next;
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void drawECSMemoryStats() {
    ImGui::Text("ECS World Struct Size: %lu", sizeof(ecs::World));
    uint32_t totalUsed = 0;
    uint32_t totalReserved = 0;
    uint32_t totalLookups = 0;
    for (uint32_t i = 0; i < ecs::COMPONENTS_MAX_COUNT; ++i) {
        auto* header = ecs::the_world.components[i];
        if (header) {
            const char* name = header->name;
            auto lookupSize = sizeof(ecs::EntityLookup);
            auto h2eSizeReserved = header->handleToEntity._capacity * sizeof(ecs::EntityIndex);
            auto h2eSizeUsed = header->handleToEntity.size() * sizeof(ecs::EntityIndex);
            auto dataSizeUsed = header->count() * header->storageElementSize;
            auto dataSizeReserved = dataSizeUsed;
            if (header->pDebugStorageCapacity) {
                dataSizeReserved = (*header->pDebugStorageCapacity) * header->storageElementSize;
            }
            totalLookups += lookupSize;
            totalUsed += lookupSize + h2eSizeUsed + dataSizeUsed;
            totalReserved += lookupSize + h2eSizeReserved + dataSizeReserved;
            if (ImGui::TreeNode(header, "#%u. %s | %0.2lf KB",
                                header->typeId,
                                name ? name : "Component",
                                toKB(h2eSizeReserved + dataSizeReserved))) {
                ImGui::Text("Handle-2-Entity Array: %u / %u | %0.2lf of %0.2lf KB", header->handleToEntity._size,
                            header->handleToEntity._capacity,
                            toKB(h2eSizeUsed), toKB(h2eSizeReserved));
                if (header->pDebugStorageCapacity) {
                    ImGui::Text("Data Array: %u elements | %0.2lf of %0.2lf KB", header->count(),
                                toKB(dataSizeUsed), toKB(dataSizeReserved));
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::Text("ECXX Lookups: %0.2lf KB", toKB(totalLookups));
    ImGui::Text("ECXX Used: %0.2lf MB", toMB(totalUsed));
    ImGui::Text("ECXX Reserved: %0.2lf MB", toMB(totalReserved));
}

void MemoryProfiler::onDraw() {
    if(ImGui::BeginTabBar("Memory Stats")) {
        if(ImGui::BeginTabItem("Allocators")) {
            DrawAllocatorsTree(memory::systemAllocator);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("ECS Memory")) {
            drawECSMemoryStats();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

}