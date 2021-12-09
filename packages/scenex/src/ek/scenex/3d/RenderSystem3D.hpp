#pragma once

#include <ecxx/ecxx.hpp>
#include "Material3D.hpp"
#include <ek/util/Type.hpp>

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
    void renderObjects(const Matrix4f& proj, const Matrix4f& view);

    Material3D defaultMaterial{};
    ShadowMapRes* shadows = nullptr;
    Main3DRes* main = nullptr;
    RenderSkyBoxRes* skybox= nullptr;

    Matrix4f cameraProjection;
    Matrix4f cameraView;

    ecs::EntityApi scene;
    ecs::EntityApi camera;
};

EK_DECLARE_TYPE(RenderSystem3D);

}

