#ifdef GL_ES
precision highp float;
#endif

attribute vec2 aPosition;
attribute vec2 aTexCoord;
attribute vec4 aColorMult;
attribute vec4 aColorOffset;

uniform mat4 uModelViewProjection;

varying vec2 vTexCoord;
varying vec4 vColorMult;
varying vec4 vColorOffset;

void main() {
    vTexCoord = aTexCoord;
    vColorMult = aColorMult;
    vColorOffset = aColorOffset;
    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);
}