#ifdef GL_ES
precision mediump float;
#endif

varying vec3 vTexCoord;

uniform samplerCube uImage0;

void main() {

    gl_FragColor = textureCube(uImage0, vTexCoord);
    //gl_FragColor = vec4(vTexCoord.x, 1.0, 1.0, 1.0);

}