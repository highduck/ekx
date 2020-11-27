#ifdef GL_ES
precision mediump float;
#else
#define lowp
#define mediump
#define highp
#endif

varying vec2 vTexCoord;
varying lowp vec4 vColorMult;
#if defined(TEXTURE) && defined(COLOR_OFFSET)
varying lowp vec3 vColorOffset;
#endif

uniform lowp sampler2D uImage0;

void main() {
    #ifdef TEXTURE

    #ifdef TEXTURE_ALPHA
    lowp vec4 pixelColor = vColorMult * texture2D(uImage0, vTexCoord).r;
    #else
    lowp vec4 pixelColor = vColorMult * texture2D(uImage0, vTexCoord);
    #endif

    #ifdef COLOR_OFFSET
    gl_FragColor = pixelColor + vec4(vColorOffset * pixelColor.a, 0.0);
    #else
    gl_FragColor = pixelColor;
    #endif

    #else
    gl_FragColor = vColorMult;
    #endif
}