#include <vector>
#include <pugixml.hpp>
#include "xfl/Doc.hpp"
#include <ek/scenex/data/SGFile.hpp>
#include <ek/debug/LogSystem.hpp>
#include "sxg/SGBuilder.hpp"
#include "sxg/RenderElement.hpp"
#include "ImageSet.hpp"

#include <stb/stb_image_write.h>

namespace ek {

void exportFlash(const char* xmlPath) {
    pugi::xml_document xml;
    if (!xml.load_file(xmlPath)) {
        EK_ERROR_F("error parse xml %s", xmlPath);
        return;
    }
    auto node = xml.first_child();
    const auto* path = node.attribute("path").as_string();
    const auto* output = node.attribute("output").as_string();
    const auto* outputImages = node.attribute("outputImages").as_string();

    ImageSet imageSet{};
    auto atlasNode = node.child("atlas");
    if (atlasNode) {
        imageSet.name = atlasNode.attribute("name").as_string();
        int i = 0;
        for (auto& resolutionNode: atlasNode.children("resolution")) {
            Resolution res{};
            res.index = i++;
            res.scale = resolutionNode.attribute("scale").as_float((float) i);
            imageSet.resolutions.push_back(res);
        }
    }

    xfl::Doc ff{path};
    xfl::SGBuilder fe{ff};
    fe.build_library();
    fe.build_sprites(imageSet);

    auto sg_data = fe.export_library();
    output_memory_stream out{100};
    IO io{out};
    io(sg_data);

    auto f = fopen(output, "wb");
    fwrite(out.data(), out.size(), 1, f);
    fclose(f);

    save(imageSet, outputImages);
}


///// prerender flash symbol

// INPUT SYMBOL [Scale WIDTH HEIGHT ALPHA TRIM OUTPUT_PATH]
void runFlashFilePrerender(int argc, char** argv) {
    using namespace ek;

    xfl::Doc ff{argv[0]};
    xfl::SGBuilder exporter{ff};
    auto& doc = exporter.doc;

    auto* item = doc.findLinkage(argv[1]);
    if (item) {
        int i = 2;
        while (i < argc) {
            const float scale = strtof(argv[i], nullptr);
            const int width = atoi(argv[i + 1]);
            const int height = atoi(argv[i + 2]);
            const bool alpha = atoi(argv[i + 3]) != 0;
            const bool trim = atoi(argv[i + 4]) != 0;
            const char* output = argv[i + 5];

            xfl::RenderElementOptions opts{scale, width, height, alpha, trim};
            auto spr = renderElement(doc, *item, opts);
            saveImagePNG(*spr.image, output, alpha);
            if (spr.image) {
                delete spr.image;
                spr.image = nullptr;
            }
            i += 6;
        }
    }
}

}


int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    if (strcmp("render", argv[1]) == 0) {
        ek::runFlashFilePrerender(argc - 2, argv + 2);
    } else if (strcmp("export", argv[1]) == 0) {
        if (argc > 2) {
            ek::exportFlash(argv[2]);
        } else {
            return 1;
        }
    }
    return 0;

}