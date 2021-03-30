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
    void render();
//    void renderShadowMap(const mat4f& cameraProjection, const mat4f& cameraView);
    void renderObjects(const mat4f& proj, const mat4f& view);

    Material3D defaultMaterial{};
    ShadowMapRes* shadows = nullptr;
    Main3DRes* main = nullptr;
    RenderSkyBoxRes* skybox= nullptr;

    mat4f cameraProjection;
    mat4f cameraView;

    ecs::EntityApi scene;
    ecs::EntityApi camera;
};



}

