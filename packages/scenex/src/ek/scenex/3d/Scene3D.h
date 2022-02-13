#pragma once

#include <ecx/ecx.hpp>

#include <ekx/app/game_display.h>
#include "Camera3D.hpp"
#include "StaticMesh.hpp"
#include "Light3D.hpp"
#include "Transform3D.hpp"
#include "RenderSystem3D.hpp"

namespace ek {

void initScene3D() {
    EK_ASSERT(!g_render_system_3d);

    ECX_COMPONENT(Transform3D);
    ECX_COMPONENT(Camera3D);
    ECX_COMPONENT(Light3D);
    ECX_COMPONENT(MeshRenderer);

    g_render_system_3d = new ek::RenderSystem3D();

    setup_res_material3d();
    setup_res_mesh3d();
}

// onPreRender
void preRenderScene3D() {
    if (g_render_system_3d) {
        update_world_transform3d();
        g_render_system_3d->prepare();
        g_render_system_3d->prerender();
    }
}

// onRenderSceneBefore
void renderScene3D(game_display_info* display_info) {
    if (g_render_system_3d) {
        g_render_system_3d->render(display_info->size.x, display_info->size.y);
    }
}

}