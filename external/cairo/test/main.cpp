#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <cairo.h>

int main(int argc, char* argv[]) {
    printf("cairo version: %s\n", cairo_version_string());

    uint8_t data[4 * 4 * 4];
    auto* surf = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,
                                                     4, 4, 4 * 4);
    auto status = cairo_surface_status(surf);
    if (status != CAIRO_STATUS_SUCCESS) {
        printf("fail to create surface: %s\n", cairo_status_to_string(status));
        return EXIT_FAILURE;
    }
    auto* cr = cairo_create(surf);
    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS) {
        printf("fail to create context: %s\n", cairo_status_to_string(status));
        return EXIT_FAILURE;
    }
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
    cairo_set_source_surface(cr, surf, 0, 0);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    return EXIT_SUCCESS;
}