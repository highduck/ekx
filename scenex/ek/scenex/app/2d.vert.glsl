#ifdef GL_ES
precision highp float;
#else
#define lowp
#define mediump
#define highp
#endif

attribute vec2 aPosition;
attribute mediump vec2 aTexCoord;
attribute lowp vec4 aColorMult;
attribute lowp vec4 aColorOffset;

uniform mat4 uModelViewProjection;

varying mediump vec2 vTexCoord;
varying lowp vec4 vColorMult;
#if defined(TEXTURE) && defined(COLOR_OFFSET)
varying lowp vec3 vColorOffset;
#endif

void main() {
    vTexCoord = aTexCoord;

    #if defined(COLOR_OFFSET)
    #if defined(TEXTURE)
    vColorMult = vec4(aColorMult.xyz * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);
    vColorOffset = aColorOffset.xyz;
    #else
    vColorMult = vec4((aColorMult.xyz + aColorOffset.xyz) * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);
    #endif
    #else
    vColorMult = vec4(aColorMult.xyz * aColorMult.a, aColorMult.a);
    #endif

    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);
}
