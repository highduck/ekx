#ifdef GL_ES
precision mediump float;
#endif

varying vec2 vTexCoord;
varying vec4 vColorMult;
varying vec4 vColorOffset;

uniform sampler2D uImage0;

void main() {
    vec4 pixel_color = texture2D(uImage0, vTexCoord);
    gl_FragColor = vec4(pixel_color.x, pixel_color.y, pixel_color.z, 1.0);
}