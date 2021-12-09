#include "RenderElement.hpp"

#include "../ImageSet.hpp"
#include "../xfl/renderer/Scanner.hpp"
#include "../xfl/renderer/CairoRenderer.hpp"
#include "../xfl/renderer/CairoHelpers.hpp"
#include "../xfl/renderer/RenderCommand.hpp"
#include "../xfl/Doc.hpp"
#include <cairo.h>
#include <ek/imaging/drawing.hpp>

namespace ek::xfl {

SpriteData renderMultiSample(const Rect2f& bounds,
                             const Array<RenderCommandsBatch>& batches,
                             const RenderElementOptions& options) {
    // x4 super-sampling
    const double upscale = 4.0;

    const double scale = options.scale;
    const bool fixed = options.width > 0 && options.height > 0;

    auto rc = bounds;
    if (!options.trim) {
        rc.x -= 1;
        rc.y -= 1;
        rc.width += 2;
        rc.height += 2;
    }

    image_t* img = nullptr;
    const auto w = static_cast<int>(fixed ? options.width : ceil(rc.width * scale));
    const auto h = static_cast<int>(fixed ? options.height : ceil(rc.height * scale));
    const int stride = w * 4;

    if (w > 0 && h > 0) {
        img = new image_t(w, h);

        auto surf = cairo_image_surface_create_for_data(img->data(),
                                                        CAIRO_FORMAT_ARGB32,
                                                        w, h, stride);
        auto cr = cairo_create(surf);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
        cairo_set_source_surface(cr, surf, 0, 0);

        const Vec2i up_scaled_size{static_cast<int>(w * upscale),
                                   static_cast<int>(h * upscale)};

        auto sub_surf = cairo_surface_create_similar(surf,
                                                     CAIRO_CONTENT_COLOR_ALPHA,
                                                     up_scaled_size.x,
                                                     up_scaled_size.y);
        auto sub_cr = cairo_create(sub_surf);
        cairo_set_antialias(sub_cr, CAIRO_ANTIALIAS_NONE);

        {
            CairoRenderer sub_renderer{sub_cr};

            cairo_scale(sub_cr, scale * upscale, scale * upscale);

            if (!fixed) {
                cairo_translate(sub_cr, -rc.x, -rc.y);
            }

            for (const auto& batch : batches) {
                sub_renderer.set_transform(batch.transform);
                for (const auto& cmd : batch.commands) {
                    sub_renderer.execute(cmd);
                }
            }

            cairo_surface_flush(sub_surf);

            blit_downsample(cr, sub_surf, up_scaled_size.x, up_scaled_size.y, upscale);
            cairo_surface_flush(surf);
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        cairo_destroy(sub_cr);
        cairo_surface_destroy(sub_surf);

        // convert ARGB to ABGR
        convert_image_bgra_to_rgba(*img, *img);
    }

    SpriteData data;
    data.rc = rc;
    data.source = {0, 0, w, h};
    data.image = img;

    return data;
}

SpriteData renderLowQuality(const Rect2f& bounds,
                            const Array<RenderCommandsBatch>& batches,
                            const RenderElementOptions& options) {
    const double scale = options.scale;
    const bool fixed = options.width > 0 && options.height > 0;

    auto rc = bounds;
    if (!options.trim) {
        rc.x -= 1;
        rc.y -= 1;
        rc.width += 2;
        rc.height += 2;
    }

    image_t* img = nullptr;
    const auto w = static_cast<int>(fixed ? options.width : ceil(rc.width * scale));
    const auto h = static_cast<int>(fixed ? options.height : ceil(rc.height * scale));
    const int stride = w * 4;

    if (w > 0 && h > 0) {
        img = new image_t(w, h);

        auto surf = cairo_image_surface_create_for_data(img->data(),
                                                        CAIRO_FORMAT_ARGB32,
                                                        w, h, stride);
        auto cr = cairo_create(surf);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
        cairo_set_source_surface(cr, surf, 0, 0);
        {
            CairoRenderer renderer{cr};

            cairo_scale(cr, scale, scale);

            if (!fixed) {
                cairo_translate(cr, -rc.x, -rc.y);
            }

            for (const auto& batch : batches) {
                renderer.set_transform(batch.transform);
                for (const auto& cmd : batch.commands) {
                    renderer.execute(cmd);
                }
            }
            cairo_surface_flush(surf);
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        // convert ARGB to ABGR
        convert_image_bgra_to_rgba(*img, *img);
    }

    SpriteData data;
    data.rc = rc;
    data.source = {0, 0, w, h};
    data.image = img;

    return data;
}

bool checkContainsOnlyBitmapOperations(const Array<RenderCommandsBatch>& batches) {
    for (const auto& batch : batches) {
        for (const auto& cmd : batch.commands) {
            if (cmd.op != RenderCommand::Operation::bitmap) {
                return false;
            }
        }
    }
    return true;
}

SpriteData renderElementBatches(const Rect2f& bounds,
                                const Array<RenderCommandsBatch>& batches,
                                const RenderElementOptions& options) {
    if (checkContainsOnlyBitmapOperations(batches)) {
        return renderLowQuality(bounds, batches, options);
    }
    return renderMultiSample(bounds, batches, options);
}

SpriteData renderElement(const Doc& doc, const Element& el, const RenderElementOptions& options) {
    Scanner scanner{};
    scanner.draw(doc, el);

    auto spr = renderElementBatches(scanner.bounds.rect(), scanner.batches, options);
    spr.name = el.item.name;
    return spr;
}

}