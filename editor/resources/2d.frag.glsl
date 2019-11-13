#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexCoord;
varying vec4 vColorMult;
varying vec4 vColorOffset;

uniform sampler2D uImage0;

void main() {
    vec4 pixelColor = texture2D(uImage0, vTexCoord);
    pixelColor *= vColorMult;
    gl_FragColor = pixelColor + vColorOffset * pixelColor.wwww;
}