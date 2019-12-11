#include "render_to_sprite.hpp"

#include "dom_scanner.hpp"
#include "cairo_renderer.hpp"
#include "cairo_utility.hpp"
#include <ek/flash/doc/flash_file.hpp>
#include <cairo.h>
#include <cstring>
#include <ek/imaging/drawing.hpp>

namespace ek::flash {

using spritepack::sprite_t;

sprite_t render(const rect_f& bounds,
                const std::vector<render_batch>& batches,
                const renderer_options_t& options,
                const std::string& name) {

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

        const int2 up_scaled_size{static_cast<int>(w * upscale),
                                  static_cast<int>(h * upscale)};

        auto sub_surf = cairo_surface_create_similar(surf,
                                                     CAIRO_CONTENT_COLOR_ALPHA,
                                                     up_scaled_size.x,
                                                     up_scaled_size.y);
        auto sub_cr = cairo_create(sub_surf);
        cairo_set_antialias(sub_cr, CAIRO_ANTIALIAS_NONE);

        {
            cairo_renderer renderer{cr};
            cairo_renderer sub_renderer{sub_cr};

            cairo_scale(cr, scale, scale);
            cairo_scale(sub_cr, scale * upscale, scale * upscale);

            if (!fixed) {
                cairo_translate(cr, -rc.x, -rc.y);
                cairo_translate(sub_cr, -rc.x, -rc.y);
            }

            for (const auto& batch : batches) {
                renderer.set_transform(batch.transform);
                if (batch.bitmap) {
                    renderer.draw_bitmap(batch.bitmap);
                    cairo_surface_flush(surf);
                }

//                for (const auto& cmd : batch.commands) {
//                    renderer.execute(cmd);
//                }

                clear(sub_cr);
                sub_renderer.set_transform(batch.transform);
                for (const auto& cmd : batch.commands) {
                    sub_renderer.execute(cmd);
                }
                cairo_surface_flush(sub_surf);

                blit_downsample(cr, sub_surf, up_scaled_size.x, up_scaled_size.y, upscale);
                cairo_surface_flush(surf);
            }
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surf);

        cairo_destroy(sub_cr);
        cairo_surface_destroy(sub_surf);

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