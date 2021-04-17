#include "drawer.hpp"

#include <ek/util/detect_platform.hpp>
#include <ek/math/matrix_camera.hpp>
#include "draw2d_shader.h"
#include <ek/Allocator.hpp>
#include <ek/util/StaticStorage.hpp>
#include <ek/util/logger.hpp>

using namespace ek::graphics;

namespace ek::draw2d {

StaticStorage<Context> context{};
Context& state = context.ref();

sg_layout_desc Vertex2D::layout() {
    sg_layout_desc layout{};
    layout.buffers[0].stride = sizeof(Vertex2D);
    layout.attrs[0].offset = offsetof(Vertex2D, position);
    layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[1].offset = offsetof(Vertex2D, uv);
    layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[2].offset = offsetof(Vertex2D, cm);
    layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
    layout.attrs[3].offset = offsetof(Vertex2D, co);
    layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    return layout;
}

void Context::setNextScissors(rect_i rc) {
    if (rc != curr.scissors) {
        stateChanged = true;
    }
    next.scissors = rc;
}

void Context::setNextBlending(BlendMode blending) {
    if (curr.blend != blending) {
        stateChanged = true;
    }
    next.blend = blending;
}

void Context::setNextTexture(sg_image nextTexture) {
    if (curr.texture.id != nextTexture.id) {
        stateChanged = true;
    }
    next.texture = nextTexture;
}

void Context::setNextShader(sg_shader shader, uint8_t numTextures) {
    if (curr.shader.id != shader.id) {
        stateChanged = true;
    }
    next.shader = shader;
    next.shaderTexturesCount = numTextures;
}

void Context::applyNextState() {
    if (stateChanged) {
        curr = next;
        stateChanged = false;
    }
}

sg_pipeline createPipeline(sg_shader shader, bool useRenderTarget, bool depthStencil) {
    sg_pipeline_desc pip_desc{};
    pip_desc.layout = Vertex2D::layout();
    pip_desc.shader = shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.colors[0].write_mask = SG_COLORMASK_RGB;
    pip_desc.colors[0].blend.enabled = true;
    pip_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    pip_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    if (useRenderTarget) {
        pip_desc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
        if(depthStencil) {
            pip_desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        }
        else {
            pip_desc.depth.pixel_format = SG_PIXELFORMAT_NONE;
        }
    }
    pip_desc.label = "draw2d-pipeline";
    return sg_make_pipeline(pip_desc);
}

sg_pipeline Context::getPipeline(sg_shader shader, bool useRenderTarget, bool depthStencilPass) {
    uint64_t key{shader.id};
    if (useRenderTarget) {
        key = key | (1ull << 32ull);
    }
    if (depthStencilPass) {
        key = key | (1ull << 33ull);
    }
    auto pip = pipelines.get(key, {0});
    if (pip.id == SG_INVALID_ID) {
        pip = createPipeline(shader, useRenderTarget, depthStencilPass);
        pipelines.set(key, pip);
    }
    return pip;
}

float getTriangleArea(const Vertex2D* vertices, const uint16_t* indices, int count) {
    float sum = 0.0f;
    for (int i = 0; i < count;) {
        const float2 a = vertices[indices[i++]].position;
        const float2 b = vertices[indices[i++]].position;
        const float2 c = vertices[indices[i++]].position;
        sum += (a.x * b.y + b.x * c.y + c.x * a.y - a.x * c.y - b.x * a.y - c.x * b.y) / 2.0f;
    }
    return sum;
}

class BufferChain {
public:
    BufferChain(BufferType type, uint32_t elementsMaxCount, uint32_t elementMaxSize) :
            type_{type},
            caps{0x10 * elementMaxSize,
                 0x100 * elementMaxSize,
                 0x400 * elementMaxSize,
                 elementsMaxCount * elementMaxSize} {
    }

    uint32_t calcSizeBucket(uint32_t requiredSize) {
        if (requiredSize < caps[0]) {
            return 0;
        } else if (requiredSize < caps[1]) {
            return 1;
        } else if (requiredSize < caps[2]) {
            return 2;
        }
        return 3;
    }

    Buffer* nextBuffer(uint32_t requiredSize) {
        Buffer* buf;
        const auto index = calcSizeBucket(requiredSize);
        auto& v = buffers_[index];
        auto position = pos[index];
        if (position >= v.size()) {
            buf = new Buffer(type_, Usage::Stream, caps[index]);
            v.push_back(buf);
        } else {
            buf = v[position];
        }
        ++position;
        pos[index] = position;
        return buf;
    }

    [[nodiscard]]
    uint32_t getUsedMemory() const {
        uint32_t mem = 0u;
        for (auto& v : buffers_) {
            for (const auto* buffer : v) {
                mem += buffer->getSize();
            }
        }
        return mem;
    }

    void nextFrame() {
        resetPositions();
    }

    void resetPositions() {
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;
        pos[3] = 0;
    }

    void disposeBuffers() {
        for (auto& v : buffers_) {
            for (auto* b : v) {
                delete b;
            }
            v.resize(0);
        }
    }

    void reset() {
        disposeBuffers();
        resetPositions();
    }

    ~BufferChain() {
        disposeBuffers();
    }

private:
    BufferType type_;
    Array<Buffer*> buffers_[4]{};
    uint16_t pos[4] = {0, 0, 0, 0};
    // each bucket buffer size
    uint32_t caps[4];
};

void Context::drawUserBatch(sg_pipeline pip, uint32_t numTextures) {
    if (indicesCount_ == 0) {
        return;
    }

    const uint32_t vertexDataSize = verticesCount_ * static_cast<uint32_t>(sizeof(Vertex2D));
    const uint32_t indexDataSize = indicesCount_ << 1u;
    auto* vb = vertexBuffers_->nextBuffer(vertexDataSize);
    auto* ib = indexBuffers_->nextBuffer(indexDataSize);
    vb->update(vertexData_, vertexDataSize);
    ib->update(indexData_, indexDataSize);

    // reset current pipeline
    selectedPipeline = pip;

    bind.vertex_buffers[0] = vb->buffer;
    bind.index_buffer = ib->buffer;
    bind.fs_images[0].id = numTextures == 1 ? curr.texture.id : SG_INVALID_ID;
    sg_apply_bindings(bind);

    {
        const auto rc = curr.scissors;
        sg_apply_scissor_rect(rc.x, rc.y, rc.width, rc.height, true);
    }

    sg_draw(0, (int) indicesCount_, 1);

#ifndef NDEBUG
    stats.fillArea += getTriangleArea(vertexData_, indexData_, indicesCount_);
#endif
    stats.triangles += indicesCount_ / 3;
    ++stats.drawCalls;

    indicesCount_ = 0;
    verticesCount_ = 0;

    vertexDataNext_ = vertexData_;
    indexDataNext_ = indexData_;

    // just verify that we alloc before write
    vertexDataPos_ = nullptr;
    indexDataPos_ = nullptr;
}

void Context::drawBatch() {
    if (indicesCount_ == 0) {
        return;
    }

    const uint32_t vertexDataSize = verticesCount_ * static_cast<uint32_t>(sizeof(Vertex2D));
    const uint32_t indexDataSize = indicesCount_ << 1u;
    auto* vb = vertexBuffers_->nextBuffer(vertexDataSize);
    auto* ib = indexBuffers_->nextBuffer(indexDataSize);
    vb->update(vertexData_, vertexDataSize);
    ib->update(indexData_, indexDataSize);

    const auto* fbColor = renderTarget != nullptr ? renderTarget : framebufferColor;
    const auto* fbDepthStencil = renderDepthStencil != nullptr ? renderDepthStencil : framebufferDepthStencil;

    auto pip = getPipeline(curr.shader, fbColor != nullptr, fbDepthStencil != nullptr);
    if (pip.id != selectedPipeline.id) {
        selectedPipeline = pip;
        sg_apply_pipeline(pip);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(mvp));
    }
    bind.vertex_buffers[0] = vb->buffer;
    bind.index_buffer = ib->buffer;
    bind.fs_images[0].id = curr.shaderTexturesCount == 1 ? curr.texture.id : SG_INVALID_ID;
    sg_apply_bindings(bind);

    {
        const auto rc = curr.scissors;
        sg_apply_scissor_rect(rc.x, rc.y, rc.width, rc.height, true);
    }

    sg_draw(0, (int) indicesCount_, 1);

#ifndef NDEBUG
    stats.fillArea += getTriangleArea(vertexData_, indexData_, indicesCount_);
#endif
    stats.triangles += indicesCount_ / 3;
    ++stats.drawCalls;

    indicesCount_ = 0;
    verticesCount_ = 0;

    vertexDataNext_ = vertexData_;
    indexDataNext_ = indexData_;

    // just verify that we alloc before write
    vertexDataPos_ = nullptr;
    indexDataPos_ = nullptr;
}

void Context::allocTriangles(uint32_t vertex_count, uint32_t index_count) {
    if (checkFlags != 0) {
        if (checkFlags & Context::Check_Texture) {
            setNextTexture(texture->image);
        }
        if (checkFlags & Context::Check_Shader) {
            setNextShader(program->shader, program->numFSImages);
        }
        if (checkFlags & Context::Check_Scissors) {
            setNextScissors(rect_i{scissors});
        }
        checkFlags = 0;
    }

    if (stateChanged || (verticesCount_ + vertex_count) > 0xFFFF) {
        drawBatch();
        applyNextState();
    }

    indexDataPos_ = indexDataNext_;
    indexDataNext_ = indexDataPos_ + index_count;
    indicesCount_ += index_count;

    baseVertex_ = verticesCount_;
    vertexDataPos_ = vertexDataNext_;
    vertexDataNext_ = vertexDataPos_ + vertex_count;
    verticesCount_ += vertex_count;
}

uint32_t Context::getUsedMemory() const {
    return indexBuffers_->getUsedMemory() + vertexBuffers_->getUsedMemory();
}


Context::Context() {
    using graphics::Texture;
    using graphics::Shader;

    vertexBuffers_ = new BufferChain(BufferType::VertexBuffer, MaxVertex + 1, EK_SIZEOF_U32(Vertex2D));
    indexBuffers_ = new BufferChain(BufferType::IndexBuffer, MaxIndex + 1, EK_SIZEOF_U32(uint16_t));

    EK_TRACE << "draw2d: allocate memory buffers";
    vertexData_ = memory::systemAllocator.allocBufferForArray<Vertex2D>(MaxVertex + 1);
    indexData_ = memory::systemAllocator.allocBufferForArray<uint16_t>(MaxIndex + 1);

    vertexDataNext_ = vertexData_;
    indexDataNext_ = indexData_;
}

Context::~Context() {
    Res<graphics::Texture>{"empty"}.reset(nullptr);
    Res<Shader>{"draw2d"}.reset(nullptr);
    Res<Shader>{"draw2d_alpha"}.reset(nullptr);
    Res<Shader>{"draw2d_color"}.reset(nullptr);

    delete vertexBuffers_;
    delete[] vertexData_;

    delete indexBuffers_;
    delete[] indexData_;
}

void Context::finish() {
    // debug checks
    EK_ASSERT(scissorsStack.empty());
    EK_ASSERT(matrixStack.empty());
    EK_ASSERT(colorStack.empty());
    EK_ASSERT(programStack.empty());
    EK_ASSERT(textureStack.empty());
    EK_ASSERT(texCoordStack.empty());

    scissorsStack.clear();
    matrixStack.clear();
    colorStack.clear();
    programStack.clear();
    textureStack.clear();
    texCoordStack.clear();
}

/** Scissors **/

Context& Context::saveScissors() {
    scissorsStack.push_back(scissors);
    return *this;
}

void Context::setScissors(const rect_f& rc) {
    scissors = rc;
    checkFlags |= Check_Scissors;
}

void Context::pushClipRect(const rect_f& rc) {
    saveScissors();
    setScissors(clamp_bounds(scissors, rc));
}

Context& Context::popClipRect() {
    scissors = scissorsStack.back();
    scissorsStack.pop_back();
    checkFlags |= Check_Scissors;
    return *this;
}

/** Matrix Transform **/

Context& Context::save_matrix() {
    matrixStack.push_back(matrix);
    return *this;
}

Context& Context::save_transform() {
    save_matrix();
    save_color();
    return *this;
}

Context& Context::restore_transform() {
    restore_matrix();
    restore_color();
    return *this;
}

Context& Context::translate(float tx, float ty) {
    matrix.translate(tx, ty);
    return *this;
}

Context& Context::translate(const float2& v) {
    matrix.translate(v);
    return *this;
}

Context& Context::scale(float sx, float sy) {
    matrix.scale(sx, sy);
    return *this;
}

Context& Context::scale(const float2& v) {
    matrix.scale(v);
    return *this;
}

Context& Context::rotate(float radians) {
    matrix.rotate(radians);
    return *this;
}

Context& Context::concat(const matrix_2d& r) {
    matrix = matrix * r;
    return *this;
}

Context& Context::restore_matrix() {
    matrix = matrixStack.back();
    matrixStack.pop_back();
    return *this;
}

/** Color Transform **/

Context& Context::save_color() {
    colorStack.push_back(color);
    return *this;
}

Context& Context::restore_color() {
    color = colorStack.back();
    colorStack.pop_back();
    return *this;
}

Context& Context::scaleAlpha(float alpha) {
    auto a = (uint8_t) ((color.scale.a * ((int) (alpha * 255)) * 258u) >> 16u);
    color.scale.a = a;
    return *this;
}

Context& Context::scaleColor(abgr32_t multiplier) {
    color.scale = color.scale * multiplier;
    return *this;
}

Context& Context::concat(abgr32_t scale, abgr32_t offset) {
    using details::clamp_255;

    if (offset.abgr != 0) {
        color.offset = abgr32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }

    if (scale.abgr != 0xFFFFFFFF) {
        color.scale = color.scale * scale;
    }

    return *this;
}

Context& Context::concat(ColorMod32 colorMod) {
    return concat(colorMod.scale, colorMod.offset);
}

Context& Context::offset_color(abgr32_t offset) {
    using details::clamp_255;
    if (offset.abgr != 0) {
        color.offset = abgr32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }
    return *this;
}

/** STATES **/

Context& Context::save_texture_coords() {
    texCoordStack.push_back(uv);
    return *this;
}

Context& Context::setTextureCoords(float u0, float v0, float du, float dv) {
    uv.set(u0, v0, du, dv);
    return *this;
}

Context& Context::setTextureCoords(const rect_f& uv_rect) {
    uv = uv_rect;
    return *this;
}

Context& Context::restore_texture_coords() {
    uv = texCoordStack.back();
    texCoordStack.pop_back();
    return *this;
}

Context& Context::saveTexture() {
    textureStack.push_back(texture);
    return *this;
}

Context& Context::setEmptyTexture() {
    texture = emptyTexture;
    checkFlags |= Check_Texture;
    setTextureCoords(0, 0, 1, 1);
    return *this;
}

Context& Context::setTexture(const graphics::Texture* texture_) {
    texture = texture_;
    checkFlags |= Check_Texture;
    return *this;
}

Context& Context::setTextureRegion(const graphics::Texture* texture_, const rect_f& region) {
    texture = texture_ != nullptr ? texture_ : emptyTexture;
    checkFlags |= Check_Texture;
    uv = region;
    return *this;
}

Context& Context::restoreTexture() {
    texture = textureStack.back();
    checkFlags |= Check_Texture;
    textureStack.pop_back();
    return *this;
}

Context& Context::pushProgram(const char* name) {
    programStack.push_back(program);
    Res<graphics::Shader> pr{name};
    program = pr.empty() ? defaultShader : pr.get();
    checkFlags |= Check_Shader;
    return *this;
}

Context& Context::setProgram(const graphics::Shader* program_) {
    program = program_ ? program_ : defaultShader;
    checkFlags |= Check_Shader;
    return *this;
}

Context& Context::saveProgram() {
    programStack.push_back(program);
    return *this;
}

Context& Context::restoreProgram() {
    program = programStack.back();
    checkFlags |= Check_Shader;
    programStack.pop_back();
    return *this;
}

void Context::createDefaultResources() {
    EK_TRACE << "draw2d: create default resources";
    const auto backend = sg_query_backend();
    emptyTexture = Texture::createSolid32(4, 4, 0xFFFFFFFFu);
    defaultShader = new Shader(draw2d_shader_desc(backend));
    alphaMapShader = new Shader(draw2d_alpha_shader_desc(backend));
    solidColorShader = new Shader(draw2d_color_shader_desc(backend));
    Res<Texture>{"empty"}.reset(emptyTexture);
    Res<Shader>{"draw2d"}.reset(defaultShader);
    Res<Shader>{"draw2d_alpha"}.reset(alphaMapShader);
    Res<Shader>{"draw2d_color"}.reset(solidColorShader);
}

void beginNewFrame() {
    EK_ASSERT(!state.active);
    state.stats = {};
}

/*** drawings ***/
void begin(rect_f viewport, const matrix_2d& view, const graphics::Texture* renderTarget, const graphics::Texture* depthStencilTarget) {
    EK_ASSERT(!state.active);
    state.texture = state.emptyTexture;
    state.program = state.defaultShader;
    state.scissors = viewport;
    state.checkFlags = 0;
    state.matrix.set_identity();
    state.color = {};
    state.uv.set(0, 0, 1, 1);
    state.active = true;

    state.curr = {};
    state.next.shader = state.program->shader;
    state.next.shaderTexturesCount = 1;
    state.next.texture = state.texture->image;
    state.next.scissors = rect_i{viewport};
    state.selectedPipeline.id = SG_INVALID_ID;
    state.stateChanged = true;

    state.renderTarget = renderTarget;
    state.renderDepthStencil = depthStencilTarget;

#if EK_IOS || EK_MACOS
    state.mvp = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height) * view;
#else
    if (state.renderTarget) {
        state.mvp = ortho_2d<float>(viewport.x, viewport.bottom(), viewport.width, -viewport.height) * view;
    } else {
        state.mvp = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height) * view;
    }
#endif
}

void end() {
    EK_ASSERT(state.active);
    state.drawBatch();
    state.finish();
    state.active = false;
}

void write_index(uint16_t index) {
    *(state.indexDataPos_++) = state.baseVertex_ + index;
}

FrameStats getDrawStats() {
    return state.stats;
}

void triangles(int vertex_count, int index_count) {
    state.allocTriangles(vertex_count, index_count);
}

void quad(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t color) {
    triangles(4, 6);

    const auto cm = state.color.scale * color;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm * c1, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm * c2, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm * c3, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm * c4, co);

    write_indices_quad();
}

void quad_rotated(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 1, cm, co);
    write_vertex(x + w, y, 0, 0, cm, co);
    write_vertex(x + w, y + h, 1, 0, cm, co);
    write_vertex(x, y + h, 1, 1, cm, co);

    write_indices_quad();
}

// This function should be moved to the dedicated `indexed draw` mode
void fill_circle(const circle_f& circle, abgr32_t inner_color, abgr32_t outer_color, int segments) {
    triangles(1 + segments, 3 * segments);

    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    const auto co = state.color.offset;
    auto inner_cm = state.color.scale * inner_color;
    auto outer_cm = state.color.scale * outer_color;
    write_vertex(x, y, 0.0f, 0.0f, inner_cm, co);

    const float da = math::pi2 / segments;
    float a = 0.0f;
    while (a < math::pi2) {
        write_vertex(x + r * cosf(a), y + r * sinf(a), 1, 1, outer_cm, co);
        a += da;
    }

    const uint16_t end = segments - 1;
    for (uint16_t i = 0; i < end; ++i) {
        write_index(0u);
        write_index(i + 1u);
        write_index(i + 2u);
    }
    write_index(0u);
    write_index(segments);
    write_index(1u);
}

void write_vertex(float x, float y, float u, float v, abgr32_t cm, abgr32_t co) {
    // could be cached before draw2d
    const auto& m = state.matrix;
    const auto& uv = state.uv;

    auto* ptr = state.vertexDataPos_++;
    ptr->position.x = x * m.a + y * m.c + m.tx;
    ptr->position.y = x * m.b + y * m.d + m.ty;
    ptr->uv.x = uv.x + u * uv.width;
    ptr->uv.y = uv.y + v * uv.height;
    ptr->cm = cm;
    ptr->co = co;
}

void write_raw_vertex(const float2& pos, const float2& tex_coord, abgr32_t cm, abgr32_t co) {
    auto* ptr = state.vertexDataPos_++;
    ptr->position = pos;
    ptr->uv = tex_coord;
    ptr->cm = cm;
    ptr->co = co;
}

void write_indices_quad(const uint16_t i0,
                        const uint16_t i1,
                        const uint16_t i2,
                        const uint16_t i3,
                        const uint16_t baseVertex) {
    const uint16_t index = state.baseVertex_ + baseVertex;
    *(state.indexDataPos_++) = index + i0;
    *(state.indexDataPos_++) = index + i1;
    *(state.indexDataPos_++) = index + i2;
    *(state.indexDataPos_++) = index + i2;
    *(state.indexDataPos_++) = index + i3;
    *(state.indexDataPos_++) = index + i0;
}

void write_indices(const uint16_t* source,
                   uint16_t count,
                   uint16_t baseVertex) {
    const uint16_t index = state.baseVertex_ + baseVertex;
    for (int i = 0; i < count; ++i) {
        *(state.indexDataPos_++) = *(source++) + index;
    }
}

/////

void draw_indexed_triangles(const Array<float2>& positions, const Array<abgr32_t>& colors,
                            const Array<uint16_t>& indices, float2 offset, float2 scale) {

    int verticesTotal = static_cast<int>(positions.size());
    triangles(verticesTotal, indices.size());
    float2 loc_uv;

    for (int i = 0; i < verticesTotal; ++i) {
        float2 local_position = positions[i] * scale + offset;
        write_vertex(
                local_position.x,
                local_position.y,
                loc_uv.x,
                loc_uv.y,
                state.color.scale * colors[i],
                state.color.offset
        );
    }
    write_indices(indices.data(), indices.size());
}

void line(const float2& start, const float2& end, abgr32_t color1, abgr32_t color2, float lineWidth1,
          float lineWidth2) {
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float sn = 0.5f * sinf(angle);
    float cs = 0.5f * cosf(angle);
    float t2sina1 = sn * lineWidth1;
    float t2cosa1 = cs * lineWidth1;
    float t2sina2 = sn * lineWidth2;
    float t2cosa2 = cs * lineWidth2;

    triangles(4, 6);

    auto m1 = state.color.scale * color1;
    auto m2 = state.color.scale * color2;
    auto co = state.color.offset;

    write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    write_indices_quad();
}

void line(const float2& start,
          const float2& end,
          abgr32_t color,
          float lineWidth) {
    line(start, end, color, color, lineWidth, lineWidth);
}

void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              abgr32_t color_inner, abgr32_t color_outer) {
    auto pi2 = static_cast<float>(math::pi2);
    float da = pi2 / float(segments);
    float a0 = angle_from;
    auto m1 = state.color.scale * color_inner;
    auto m2 = state.color.scale * color_outer;
    auto co = state.color.offset;
    auto hw = line_width / 2.0f;
    auto r0 = r - hw;
    auto r1 = r + hw;
    while (a0 < angle_to) {

        triangles(4, 6);
        float a1 = fminf(angle_to, a0 + da);
        float cs0 = cosf(a0);
        float sn0 = sinf(a0);
        float cs1 = cosf(a1);
        float sn1 = sinf(a1);

        write_vertex(x + r1 * cs0, y + r1 * sn0, 0, 0, m2, co);
        write_vertex(x + r1 * cs1, y + r1 * sn1, 1, 0, m2, co);
        write_vertex(x + r0 * cs1, y + r0 * sn1, 1, 1, m1, co);
        write_vertex(x + r0 * cs0, y + r0 * sn0, 0, 1, m1, co);

        write_indices_quad();

        a0 += da;
    }
}

void strokeRect(const rect_f& rc, abgr32_t color, float lineWidth) {
    line({rc.x, rc.y}, {rc.right(), rc.y}, color, lineWidth);
    line({rc.right(), rc.y}, {rc.right(), rc.bottom()}, color, lineWidth);
    line({rc.right(), rc.bottom()}, {rc.x, rc.bottom()}, color, lineWidth);
    line({rc.x, rc.bottom()}, {rc.x, rc.y}, color, lineWidth);
}

void strokeCircle(const circle_f& circle, abgr32_t color, float lineWidth, int segments) {
    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    const float da = math::pi2 / segments;
    float a = 0.0f;
    float2 pen{x, y - r};
    while (a < math::pi2) {
        float2 next{x + r * cosf(a), y + r * sinf(a)};
        line(pen, next, color, lineWidth);
        pen = next;
        a += da;
    }
    line(pen, {x, y - r}, color, lineWidth);
}

void endFrame() {
    state.vertexBuffers_->nextFrame();
    state.indexBuffers_->nextFrame();
}

void initialize() {
    EK_TRACE << "draw2d initialize";
    context.initialize();
    EK_TRACE << "draw2d initialized";
}

void shutdown() {
    EK_TRACE << "draw2d shutdown";
    context.shutdown();
}

}