#include "drawer.hpp"

#include <ek/util/locator.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/math/matrix_camera.hpp>
#include "draw2d_shader.h"
#include <ek/scenex/2d/Sprite.hpp>

using namespace ek::graphics;

namespace ek::draw2d {

void Context::setScissors(rect_i rc) {
    if (rc != curr.scissors) {
        stateChanged = true;
    }
    next.scissors = rc;
}

void Context::setBlendMode(BlendMode blending) {
    if (curr.blend != blending) {
        stateChanged = true;
    }
    next.blend = blending;
}

void Context::setTexture(sg_image texture) {
    if (curr.texture.id != texture.id) {
        stateChanged = true;
    }
    next.texture = texture;
}

void Context::setProgram(sg_shader shader, uint8_t numTextures) {
    if (curr.shader.id != shader.id) {
        stateChanged = true;
    }
    next.shader = shader;
    next.shaderTexturesCount = numTextures;
}

void Context::setNextState() {
    if (stateChanged) {
        curr = next;
        stateChanged = false;
    }
}

sg_pipeline createPipeline(sg_shader shader, bool useRenderTarget) {
    sg_pipeline_desc pip_desc{};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex2D);
    pip_desc.layout.attrs[0].offset = offsetof(Vertex2D, position);
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[1].offset = offsetof(Vertex2D, uv);
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[2].offset = offsetof(Vertex2D, cm);
    pip_desc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[3].offset = offsetof(Vertex2D, co);
    pip_desc.layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.shader = shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.blend.enabled = true;
    pip_desc.blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    pip_desc.blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_desc.blend.color_write_mask = SG_COLORMASK_RGB;
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    if (useRenderTarget) {
        pip_desc.blend.color_format = SG_PIXELFORMAT_RGBA8;
        pip_desc.blend.depth_format = SG_PIXELFORMAT_NONE;
    }
    pip_desc.depth_stencil.depth_write_enabled = false;
    pip_desc.depth_stencil.stencil_enabled = false;
    pip_desc.rasterizer.sample_count = 0;
    pip_desc.label = "draw2d-pipeline";
    return sg_make_pipeline(pip_desc);
}

sg_pipeline Context::getPipeline(sg_shader shader, bool useRenderTarget) {
    uint64_t key{shader.id};
    if (useRenderTarget) {
        key = key | (1ull << 32ull);
    }
    auto it = pipelines.find(key);
    if (it == pipelines.end()) {
        pipelines[key] = createPipeline(shader, useRenderTarget);
    }
    return pipelines[key];
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
    explicit BufferChain(BufferType type, uint32_t bufferSize) : type_{type} {
        // manual buffering
        // framesNum_ = 3; // triple-buffering
        buffersLimit_ = 0; // unlimited

        // orphaning
        framesNum_ = 1;
//        buffersLimit_ = 1;
        useOrphaning = true;
        maxSize = bufferSize;
    }

    Buffer* nextBuffer() {
        Buffer* buf;
        if (pos >= buffers_.size()) {
            buf = new Buffer(type_, Usage::Dynamic, maxSize);
            //buf->useDataOrphaning = useOrphaning;
            buffers_.push_back(buf);
        } else {
            buf = buffers_[pos];
        }
        ++pos;
        if (buffersLimit_ > 0 && pos >= buffersLimit_) {
            pos = 0;
        }
        return buf;
    }

    [[nodiscard]]
    uint32_t getUsedMemory() const {
        uint32_t mem = 0u;
        for (const auto* buffer : buffers_) {
            mem += buffer->getSize();
        }
        return mem;
    }

    void nextFrame() {
        ++frame;
        if (frame >= framesNum_) {
            frame = 0;
            pos = 0;
        }
    }

    void reset() {
        for (auto* b : buffers_) {
            delete b;
        }
        buffers_.resize(0);
    }

    ~BufferChain() {
        reset();
    }

private:
    BufferType type_;
    std::vector<Buffer*> buffers_;
    int pos = 0;
    int frame = 0;
    int buffersLimit_;
    int framesNum_;
    bool useOrphaning = true;
    uint32_t maxSize;
};

Context::Context() {

    using graphics::Texture;
    using graphics::Shader;

    emptyTexture = Texture::createSolid32(4, 4, 0xFFFFFFFFu);
    defaultShader = new Shader(draw2d_shader_desc());
    alphaMapShader = new Shader(draw2d_alpha_shader_desc());
    solidColorShader = new Shader(draw2d_color_shader_desc());

    Res<Texture>{"empty"}.reset(emptyTexture);
    auto* spr = new Sprite();
    spr->texture.setID("empty");
    Res<Sprite>{"empty"}.reset(spr);
    Res<Shader>{"draw2d"}.reset(defaultShader);
    Res<Shader>{"draw2d_alpha"}.reset(alphaMapShader);
    Res<Shader>{"draw2d_color"}.reset(solidColorShader);

    vertexBuffers_ = new BufferChain(BufferType::VertexBuffer, (MaxVertex + 1) * sizeof(Vertex2D));
    vertexData_ = new Vertex2D[MaxVertex + 1];
    vertexDataNext_ = vertexData_;

    indexBuffers_ = new BufferChain(BufferType::IndexBuffer, (MaxIndex + 1) * sizeof(uint16_t));
    indexData_ = new uint16_t[MaxIndex + 1];
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

void Context::drawBatch() {
    if (indicesCount_ == 0) {
        return;
    }

    auto* vb = vertexBuffers_->nextBuffer();
    auto* ib = indexBuffers_->nextBuffer();
    vb->update(vertexData_, verticesCount_ * sizeof(Vertex2D));
    ib->update(indexData_, indicesCount_ << 1u);

    auto pip = getPipeline(curr.shader, renderTarget != nullptr);
    if (pip.id != selectedPipeline.id) {
        selectedPipeline = pip;
        sg_apply_pipeline(pip);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &mvp, sizeof(mvp));
    }
    bind.vertex_buffers[0] = vb->buffer;
    bind.index_buffer = ib->buffer;
    bind.fs_images[0].id = curr.shaderTexturesCount == 1 ? curr.texture.id : SG_INVALID_ID;
    sg_apply_bindings(bind);

    auto scissors = curr.scissors;
    sg_apply_scissor_rect(scissors.x, scissors.y, scissors.width, scissors.height, true);

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

void Context::allocTriangles(int vertex_count, int index_count) {
    if (checkFlags != 0) {
        if (checkFlags & Context::Check_Texture) {
            setTexture(texture->image);
        }
        if (checkFlags & Context::Check_Shader) {
            setProgram(program->shader, program->numFSImages);
        }
        if (checkFlags & Context::Check_Scissors) {
            setScissors(rect_i{scissors});
        }
        checkFlags = 0;
    }

    if (stateChanged || (verticesCount_ + vertex_count) > 0xFFFF) {
        drawBatch();
        setNextState();
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

Context* state = nullptr;

void Context::finish() {
    // debug checks
    assert(scissorsStack.empty());
    assert(matrixStack.empty());
    assert(colorStack.empty());
    assert(programStack.empty());
    assert(textureStack.empty());
    assert(texCoordStack.empty());

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

Context& Context::set_texture_coords(float u0, float v0, float du, float dv) {
    uv.set(u0, v0, du, dv);
    return *this;
}

Context& Context::set_texture_coords(const rect_f& uv_rect) {
    uv = uv_rect;
    return *this;
}

Context& Context::restore_texture_coords() {
    uv = texCoordStack.back();
    texCoordStack.pop_back();
    return *this;
}

Context& Context::save_texture() {
    textureStack.push_back(texture);
    return *this;
}

Context& Context::set_empty_texture() {
    texture = emptyTexture;
    checkFlags |= Check_Texture;
    set_texture_coords(0, 0, 1, 1);
    return *this;
}

Context& Context::set_texture(const graphics::Texture* texture_) {
    texture = texture_;
    checkFlags |= Check_Texture;
    return *this;
}

Context& Context::set_texture_region(const graphics::Texture* texture_, const rect_f& region) {
    texture = texture_ != nullptr ? texture_ : emptyTexture;
    checkFlags |= Check_Texture;
    uv = region;
    return *this;
}

Context& Context::restore_texture() {
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

void beginNewFrame() {
    assert(!state->active);
    state->stats = {};
}

/*** drawings ***/
void begin(rect_i viewport, const matrix_2d& view, const graphics::Texture* renderTarget) {
    assert(!state->active);
    state->texture = state->emptyTexture;
    state->program = state->defaultShader;
    state->scissors = rect_f{viewport};
    state->checkFlags = 0;
    state->renderTarget = renderTarget;
    state->matrix.set_identity();
    state->color = {};
    state->uv.set(0, 0, 1, 1);
    state->active = true;

    state->curr = {};
    state->next.shader = state->program->shader;
    state->next.shaderTexturesCount = 1;
    state->next.texture = state->texture->image;
    state->next.scissors = viewport;
    state->selectedPipeline.id = SG_INVALID_ID;
    state->stateChanged = true;

    state->renderTarget = renderTarget;
    if (renderTarget) {
        state->mvp = ortho_2d<float>(viewport.x, viewport.bottom(), viewport.width, -viewport.height) * view;
    } else {
        state->mvp = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height) * view;
    }
}

void end() {
    assert(state->active);
    state->drawBatch();
    state->finish();
    state->active = false;
}

void write_index(uint16_t index) {
    *(state->indexDataPos_++) = state->baseVertex_ + index;
}

FrameStats getDrawStats() {
    return state->stats;
}

void triangles(int vertex_count, int index_count) {
    state->allocTriangles(vertex_count, index_count);
}

void quad(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state->color.scale;
    const auto co = state->color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t color) {
    triangles(4, 6);

    const auto cm = state->color.scale * color;
    const auto co = state->color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4) {
    triangles(4, 6);

    const auto cm = state->color.scale;
    const auto co = state->color.offset;
    write_vertex(x, y, 0, 0.0f, cm * c1, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm * c2, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm * c3, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm * c4, co);

    write_indices_quad();
}

void quad_rotated(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state->color.scale;
    const auto co = state->color.offset;
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

    const auto co = state->color.offset;
    auto inner_cm = state->color.scale * inner_color;
    auto outer_cm = state->color.scale * outer_color;
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
    const auto& m = state->matrix;
    const auto& uv = state->uv;

    auto* ptr = state->vertexDataPos_++;
    ptr->position.x = x * m.a + y * m.c + m.tx;
    ptr->position.y = x * m.b + y * m.d + m.ty;
    ptr->uv.x = uv.x + u * uv.width;
    ptr->uv.y = uv.y + v * uv.height;
    ptr->cm = cm;
    ptr->co = co;
}

void write_raw_vertex(const float2& pos, const float2& tex_coord, abgr32_t cm, abgr32_t co) {
    auto* ptr = state->vertexDataPos_++;
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
    const uint16_t index = state->baseVertex_ + baseVertex;
    *(state->indexDataPos_++) = index + i0;
    *(state->indexDataPos_++) = index + i1;
    *(state->indexDataPos_++) = index + i2;
    *(state->indexDataPos_++) = index + i2;
    *(state->indexDataPos_++) = index + i3;
    *(state->indexDataPos_++) = index + i0;
}

void write_indices(const uint16_t* source,
                   uint16_t count,
                   uint16_t baseVertex) {
    const uint16_t index = state->baseVertex_ + baseVertex;
    for (int i = 0; i < count; ++i) {
        *(state->indexDataPos_++) = *(source++) + index;
    }
}

/////

void draw_indexed_triangles(
        const std::vector<float2>& positions, const std::vector<abgr32_t>& colors,
        const std::vector<uint16_t>& indices, float2 offset, float2 scale) {

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
                state->color.scale * colors[i],
                state->color.offset
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

    auto m1 = state->color.scale * color1;
    auto m2 = state->color.scale * color2;
    auto co = state->color.offset;

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
    auto m1 = state->color.scale * color_inner;
    auto m2 = state->color.scale * color_outer;
    auto co = state->color.offset;
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

void endFrame() {
    state->vertexBuffers_->nextFrame();
    state->indexBuffers_->nextFrame();
}

void init() {
    assert(!state);
    state = new Context();
}

}