cmake_minimum_required(VERSION 3.19)
project(cairo)

add_library(${PROJECT_NAME} STATIC
        src/cairo.c
        src/cairo-atomic.c
        src/cairo-error.c
        src/cairo-font-face.c
        src/cairo-font-options.c
        src/cairo-version.c
        src/cairo-surface.c
        src/cairo-pattern.c
        src/cairo-observer.c
        src/cairo-clip.c
        src/cairo-array.c
        src/cairo-scaled-font.c
        src/cairo-raster-source-pattern.c
        src/cairo-path-fixed.c
        src/cairo-matrix.c
        src/cairo-hash.c
        src/cairo-debug.c
        src/cairo-region.c
        src/cairo-toy-font-face.c
        src/cairo-surface-subsurface.c
        src/cairo-cache.c
        src/cairo-color.c
        src/cairo-boxes.c
        src/cairo-clip-region.c
        src/cairo-clip-boxes.c
        src/cairo-clip-polygon.c
        src/cairo-clip-surface.c
        src/cairo-image-surface.c
        src/cairo-image-compositor.c
        src/cairo-composite-rectangles.c
        src/cairo-compositor.c
        src/cairo-spans-compositor.c
        src/cairo-spans.c
        src/cairo-tor-scan-converter.c
        src/cairo-rectangular-scan-converter.c
        src/cairo-mono-scan-converter.c
        src/cairo-mesh-pattern-rasterizer.c
        src/cairo-bentley-ottmann-rectangular.c
        src/cairo-bentley-ottmann.c
        src/cairo-bentley-ottmann-rectilinear.c
        src/cairo-no-compositor.c
        src/cairo-hull.c
        src/cairo-path-bounds.c

        src/cairo-analysis-surface.c
        src/cairo-arc.c
        src/cairo-path.c
        src/cairo-boxes-intersect.c
        src/cairo-damage.c
        src/cairo-font-face-twin.c
        src/cairo-font-face-twin-data.c
        src/cairo-mask-compositor.c
        src/cairo-shape-mask-compositor.c

        src/cairo-path-fixed.c
        src/cairo-path-in-fill.c
        src/cairo-contour.c
        src/cairo-surface-offset.c
        src/cairo-clip-tor-scan-converter.c
        src/cairo-tor22-scan-converter.c
        src/cairo-shape-mask-compositor.c
        src/cairo-polygon.c
        src/cairo-polygon-reduce.c
        src/cairo-polygon-intersect.c
        src/cairo-freed-pool.c
        src/cairo-freelist.c
        src/cairo-rectangle.c
        src/cairo-traps.c
        src/cairo-traps-compositor.c
        src/cairo-scaled-font-subsets.c
        src/cairo-device.c
        src/cairo-mutex.c
        src/cairo-surface-snapshot.c
        src/cairo-misc.c
        src/cairo-line.c
        src/cairo-wideint.c
        src/cairo-unicode.c
        src/cairo-path-stroke-traps.c
        src/cairo-path-stroke-boxes.c
        src/cairo-path-stroke-polygon.c
        src/cairo-path-stroke-tristrip.c
        src/cairo-path-stroke.c
        src/cairo-image-source.c
        src/cairo-user-font.c
        src/cairo-default-context.c
        src/cairo-gstate.c
        src/cairo-path-fill.c
        src/cairo-stroke-style.c
        src/cairo-stroke-dash.c
        src/cairo-spline.c
        src/cairo-slope.c
        src/cairo-pen.c
        src/cairo-recording-surface.c
        src/cairo-surface-wrapper.c
        src/cairo-tristrip.c


        src/pixman.c
        src/pixman-access.c
        src/pixman-access-accessors.c
        src/pixman-bits-image.c
        src/pixman-combine32.c
        src/pixman-combine-float.c
        src/pixman-conical-gradient.c
        src/pixman-filter.c
        src/pixman-x86.c
        src/pixman-mips.c
        src/pixman-arm.c
        src/pixman-ppc.c
        src/pixman-edge.c
        src/pixman-edge-accessors.c
        src/pixman-fast-path.c
        src/pixman-glyph.c
        src/pixman-general.c
        src/pixman-gradient-walker.c
        src/pixman-image.c
        src/pixman-implementation.c
        src/pixman-linear-gradient.c
        src/pixman-matrix.c
        src/pixman-noop.c
        src/pixman-radial-gradient.c
        src/pixman-region16.c
        src/pixman-region32.c
        src/pixman-solid-fill.c
        src/pixman-timer.c
        src/pixman-trap.c
        src/pixman-utils.c

        )

target_include_directories(${PROJECT_NAME} PUBLIC
        src
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 99
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_compile_definitions(${PROJECT_NAME}
            PUBLIC CAIRO_WIN32_STATIC_BUILD)
else()
    target_link_libraries(${PROJECT_NAME} -pthread)
endif()

add_library(ekx::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (EKX_BUILD_EXTERNAL_TESTS)
    add_subdirectory(test)
endif ()
