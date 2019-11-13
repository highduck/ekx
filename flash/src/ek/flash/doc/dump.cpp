#include <ek/logger.hpp>
#include "dump.h"

#include "flash_file.h"

namespace ek::flash {

void dump(const document_info& info) {
    EK_INFO("width: %i", info.width);
    EK_INFO("height: %i", info.height);
    EK_INFO("xflVersion: %f", info.xflVersion);
    EK_INFO("frameRate: %i", info.frameRate);
    EK_INFO("viewAngle3D: %f", info.viewAngle3D);
    EK_INFO("backgroundColor: #%x", info.backgroundColor);
    EK_INFO("buildNumber: %i", info.buildNumber);
    EK_INFO("currentTimeline: %i", info.currentTimeline);
    EK_INFO("majorVersion: %i", info.majorVersion);
}

void dump(const flash_file& doc) {
    dump(doc.info);

    EK_INFO << "Folders:";
    for (const auto& it: doc.folders) {
        EK_INFO("    DIR: %s  (%s)", it.item.name.c_str(), it.item.itemID.c_str());
    }

    EK_INFO << "Timelines:";
    for (const auto& it: doc.timelines) {
        EK_INFO("    TL: %s", it.name.c_str());
        for (const auto& layer: it.layers) {
            EK_INFO("          L: %s", layer.name.c_str());
        }
    }

    EK_INFO("Symbols:");
    for (const auto& it: doc.library) {
        EK_INFO("    @ %s", it.item.name.c_str());
        for (const auto& layer: it.timeline.layers) {
            EK_INFO("          L: %s", layer.name.c_str());
        }
    }
}

}