#pragma sokol @vs vs_2d
uniform vs_params {
    mat4 uModelViewProjection;
};

in vec2 aPosition;
in vec2 aTexCoord;
in vec4 aColorMult;
in vec4 aColorOffset;

out vec2 vTexCoord;
out vec4 vColorMult;
out vec3 vColorOffset;

void main() {
    vTexCoord = aTexCoord;
    vColorMult = vec4(aColorMult.xyz * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);
    vColorOffset = aColorOffset.xyz;
    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);
}
#pragma sokol @end

#pragma sokol @vs vs_2d_color
uniform vs_params {
    mat4 uModelViewProjection;
};

in vec2 aPosition;
in vec2 aTexCoord;
in vec4 aColorMult;
in vec4 aColorOffset;

out vec4 vColorMult;

void main() {
    aTexCoord;
    vColorMult = vec4((aColorMult.xyz + aColorOffset.xyz) * aColorMult.a, (1.0 - aColorOffset.a) * aColorMult.a);
    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);
}
#pragma sokol @end

#pragma sokol @fs fs_2d_alpha

uniform lowp sampler2D uImage0;

in vec2 vTexCoord;
in lowp vec4 vColorMult;
in lowp vec3 vColorOffset;
out vec4 frag_color;

void main() {
    lowp vec4 pixelColor = vColorMult * texture(uImage0, vTexCoord).r;
    frag_color = pixelColor + vec4(vColorOffset * pixelColor.a, 0.0);
}

#pragma sokol @end

#pragma sokol @fs fs_2d

uniform lowp sampler2D uImage0;

in vec2 vTexCoord;
in lowp vec4 vColorMult;
in lowp vec3 vColorOffset;
out vec4 frag_color;

void main() {
    lowp vec4 pixelColor = vColorMult * texture(uImage0, vTexCoord);
    frag_color = pixelColor + vec4(vColorOffset * pixelColor.a, 0.0);
}
#pragma sokol @end

#pragma sokol @fs fs_2d_color

in lowp vec4 vColorMult;
out vec4 frag_color;

void main() {
    frag_color = vColorMult;
}

#pragma sokol @end

#pragma sokol @program canvas vs_2d fs_2d
#pragma sokol @program canvas_alpha vs_2d fs_2d_alpha
#pragma sokol @program canvas_color vs_2d_color fs_2d_color