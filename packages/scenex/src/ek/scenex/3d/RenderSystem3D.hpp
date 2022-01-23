#pragma once

#include <ecxx/ecxx.hpp>
#include "Material3D.hpp"

namespace ek {

struct ShadowMapRes;
struct Main3DRes;
struct RenderSkyBoxRes;

class RenderSystem3D {
public:

    RenderSystem3D();
    ~RenderSystem3D();

    void prepare();
    void prerender();
    void render(float width, float height);
//    void renderShadowMap(const mat4f& cameraProjection, const mat4f& cameraView);
    void renderObjects(mat4_t proj, mat4_t view);

    Material3D defaultMaterial{};
    ShadowMapRes* shadows = nullptr;
    Main3DRes* main = nullptr;
    RenderSkyBoxRes* skybox= nullptr;

    mat4_t cameraProjection = mat4_identity();
    mat4_t cameraView = mat4_identity();

    ecs::EntityApi scene;
    ecs::EntityApi camera;
};

}

extern ek::RenderSystem3D* g_render_system_3d;
