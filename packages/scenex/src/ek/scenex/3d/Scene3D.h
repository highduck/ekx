#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/scenex/app/GameDisplay.hpp>
#include "Camera3D.hpp"
#include "StaticMesh.hpp"
#include "Light3D.hpp"
#include "Transform3D.hpp"
#include "RenderSystem3D.hpp"

namespace ek {

void initScene3D() {
    ECX_COMPONENT(Transform3D);
    ECX_COMPONENT(Camera3D);
    ECX_COMPONENT(Light3D);
    ECX_COMPONENT(MeshRenderer);

    Locator::create<RenderSystem3D>();
}

// onPreRender
void preRenderScene3D() {
    auto* r3d = Locator::get<RenderSystem3D>();
    if (r3d) {
        Transform3D::updateAll();
        r3d->prepare();
        r3d->prerender();
    }
}

// onRenderSceneBefore
void renderScene3D(GameDisplay& display) {
    auto* r3d = Locator::get<RenderSystem3D>();
    if (r3d) {
        r3d->render(display.info.size.x, display.info.size.y);
    }
}

}