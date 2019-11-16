#ifdef GL_ES
precision highp float;
#endif

attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
attribute vec4 aColorMult;
attribute vec4 aColorOffset;

uniform mat4 uModelViewProjection;

void main() {
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}