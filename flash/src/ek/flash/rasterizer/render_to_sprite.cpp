#include "render_to_sprite.h"

#include "dom_scanner.h"
#include "cairo_renderer.h"
#include <ek/flash/doc/flash_file.h>
#include <cairo.h>
#include <cstring>
#include <ek/imaging/drawing.hpp>

namespace ek::flash {

using spritepack::sprite_t;

sprite_t render(const rect_f& bounds,
                const std::vector<render_batch>& batches,
                const renderer_options_t& options,
                const std::string& name) {

    const int width = options.width;
    const int height = options.height;
    const float scale = options.scale;
    const bool fixed = width > 0 && height > 0;

    auto rc = bounds;
    if (!options.trim) {
        rc.x -= 1;
        rc.y -= 1;
        rc.width += 2;
        rc.height += 2;
    }

    image_t* img = nullptr;
    const auto w = static_cast<int>(fixed ? width : ceil(rc.width * scale));
    const auto h = static_cast<int>(fixed ? height : ceil(rc.height * scale));
    const int stride = w * 4;

    if (w > 0 && h > 0) {
        img = new image_t(w, h);

        auto surf = cairo_image_surface_create_for_data(img->data(), CAIRO_FORMAT_ARGB32, w, h, stride);
        auto cr = cairo_create(surf);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
        cairo_set_source_surface(cr, surf, 0, 0);

        cairo_save(cr);
        {
            cairo_scale(cr, scale, scale);

            if (!fixed) {
                cairo_translate(cr, -rc.x, -rc.y);
            }

            cairo_renderer renderer{cr};
            for (const auto& batch : batches) {
                renderer.set_transform(batch.transform);
                if (batch.bitmap) {
                    renderer.draw_bitmap(batch.bitmap);
                    cairo_surface_flush(surf);
                }
                for (const auto& cmd : batch.commands) {
                    renderer.execute(cmd);
                }
            }
        }
        cairo_restore(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        // convert ARGB to ABGR
        convert_image_bgra_to_rgba(*img, *img);
    }

    sprite_t data;
    data.name = name;
    data.rc = rc;
    data.source = {0, 0, w, h};
    data.image = img;

    return data;
}

sprite_t render(const flash_file& doc, const element_t& el, const renderer_options_t& options) {
    dom_scanner scanner{doc};
    scanner.scan(el);
    return render(
            scanner.output.bounds.rect(),
            scanner.output.batches,
            options,
            el.item.name
    );
}

}