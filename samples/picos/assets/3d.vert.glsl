#ifdef GL_ES
precision highp float;
#endif

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
attribute vec4 aColorMult;
attribute vec4 aColorOffset;

uniform mat4 uModelViewProjection;
uniform mat4 uModel;
uniform mat3 u_normal_matrix;
uniform mat4 u_depth_mvp;

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vFragPos;
varying vec4 vColorMult;
varying vec4 vColorOffset;

varying vec4 v_shadow_pos;

void main() {
    vNormal = normalize(u_normal_matrix * aNormal);
    vFragPos = vec3(uModel * vec4(aPosition, 1.0));
    vTexCoord = aTexCoord;
    vColorMult = aColorMult;
    vColorOffset = aColorOffset;

    v_shadow_pos = u_depth_mvp * vec4(aPosition, 1.0);
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}