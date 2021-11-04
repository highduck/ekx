#pragma sokol @vs vs_shadow_map
uniform vs_shadow_map_params {
    mat4 uModelViewProjection;
};

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aColorMult;
in vec4 aColorOffset;
out vec2 projZW;

void main() {
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
    projZW = gl_Position.zw;
}
#pragma sokol @end

#pragma sokol @fs fs_shadow_map
in vec2 projZW;
out vec4 fragColor;

vec4 encodeDepth(float v) {
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
    enc = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
    return enc;
}

void main() {
    float depth = projZW.x / projZW.y;
    fragColor = encodeDepth(depth);
}
    #pragma sokol @end

    #pragma sokol @program render3d_shadow_map vs_shadow_map fs_shadow_map

    #pragma sokol @vs vs
uniform vs_params {
    mat4 uModelViewProjection;
    mat4 uModel;
    mat4 u_normal_matrix;
    mat4 u_depth_mvp;
};

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aColorMult;
in vec4 aColorOffset;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vFragPos;
out vec4 vColorMult;
out vec4 vColorOffset;
out vec4 v_shadow_pos;

void main() {
    vNormal = vec3(normalize(u_normal_matrix * vec4(aPosition, 1.0)));
    vFragPos = mat3(uModel) * aPosition;
    vTexCoord = aTexCoord;
    vColorMult = aColorMult;
    vColorOffset = aColorOffset;

    v_shadow_pos = u_depth_mvp * vec4(aPosition, 1.0);
    gl_Position = uModelViewProjection * vec4(aPosition, 1.0);
}

#pragma sokol @end

#pragma sokol @fs fs

#define PI 3.14159265

uniform material_params {
    vec3 mat_ambient;
    vec3 mat_diffuse;
    vec3 mat_specular;
    vec3 mat_emission;
    float mat_shininess;
    float mat_roughness;
};

// for Directional Light
uniform light_params {
    // light direction actually
    vec3 light_position;
    vec3 light_ambient;
    vec3 light_diffuse;
    vec3 light_specular;
};

// for Point Light
uniform light2_params {
    vec3 light2_position;
    float light2_radius;
    vec3 light2_ambient;
    float light2_falloff;
    vec3 light2_diffuse;
    vec3 light2_specular;
};

uniform fs_params {
    vec4 u_time;
    vec4 u_resolution;
    vec3 uViewPos;
};

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 vColorMult;
in vec4 vColorOffset;
in vec4 v_shadow_pos;

uniform sampler2D uImage0;
uniform sampler2D u_image_shadow_map;

out vec4 frag_color;

float decodeDepth(vec4 rgba) {
    return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

// tone mapping

// Computes the luminance of the specified linear RGB color using the
// luminance coefficients from Rec. 709.
float luminance(vec3 linear) {
    return dot(linear, vec3(0.2126, 0.7152, 0.0722));
}

vec3 tone_exposure(vec3 x, float exposure) {
    // reinhard tone mapping
    return vec3(1.0) - exp(-x * exposure);
}

vec3 linear_to_gamma(vec3 x) {
    return pow(x, vec3(1.0 / 2.2));
}

vec3 tonemap_default(vec3 x) {
    // gamma correction
    return linear_to_gamma(tone_exposure(x, 1.0));
}

vec3 tonemap_unreal(vec3 x) {
    // Unreal, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return x / (x + 0.155) * 1.019;
}

vec3 tonemap_reinhard(const vec3 x) {
    return linear_to_gamma(x / (1.0 + luminance(x)));
}

/*** dithering ***/
float triangle_noise(highp vec2 n) {
    // triangle noise, in [-1.0..1.0[ range
    n += vec2(0.07 * fract(u_time.x));
    n  = fract(n * vec2(5.3987, 5.4421));
    n += dot(n.yx, n.xy + vec2(21.5351, 14.3137));

    highp float xy = n.x * n.y;
    // compute in [0..2[ and remap to [-1.0..1.0[
    return fract(xy * 95.4307) + fract(xy * 75.04961) - 1.0;
}

vec4 dither(const vec4 x) {
    // Gjøl 2016, "Banding in Games: A Noisy Rant"
    //return x + triangle_noise(gl_FragCoord.xy * u_resolution.zw) / 255.0;

    // RGB
    vec2 uv = gl_FragCoord.xy * u_resolution.zw;
    vec3 dither = 16.0 * vec3(
    triangle_noise(uv),
    triangle_noise(uv + 0.1337),
    triangle_noise(uv + 0.3141)) / 255.0;
    return vec4(x.rgb + dither, x.a + dither.x);
}

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed, i);
    float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dot_product) * 43758.5453);
}

float attenuation_inverse(float x, float energy, float quadratic, float linear, float constant) {
    return energy * (1.0 / (quadratic * x * x + linear * x + constant));
}

float attenuation_sphere(float x, float radius, float distance) {
    return x * (1.0 - min(1.0, distance / radius));
}

float attenuation_point(vec3 distance_by_radius, float falloff) {
    // wrong, some "out" curve
    //return pow(1.0 - clamp(dot(distance_by_radius, distance_by_radius), 0.0, 1.0), falloff);
    float x = min(1.0, length(distance_by_radius));
    return pow(1.0 - x, 2.0 * falloff);
}

float attenuation_disk(float radius, float distance, float cutoff) {
    // cutoff is like 0.001 to 0.15 ?
    // https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation
    float denom = distance / radius + 1.0;
    float att = 1.0 / (denom * denom);
    float t = (att - cutoff) / (1.0 - cutoff);
    return max(t, 0.0);
}

float calc_spec(vec3 light_dir, vec3 view_dir, vec3 normal) {
    // blinn
    if (true)
    {
        float kEnergyConservation = (8.0 + mat_shininess) / (8.0 * PI);
        vec3 halfway_dir = normalize(light_dir + view_dir);
        return kEnergyConservation * pow(max(dot(normal, halfway_dir), 0.0), mat_shininess);
    }
    else
    {
        float kEnergyConservation = (2.0 + mat_shininess) / (2.0 * PI);
        vec3 reflect_dir = reflect(-light_dir, normal);
        return kEnergyConservation * pow(max(dot(view_dir, reflect_dir), 0.0), mat_shininess);
    }
}

float calc_shadow(const float cos_theta) {
    vec3 shadow_pos = v_shadow_pos.xyz / v_shadow_pos.w;
    vec2 shadow_uv = shadow_pos.xy;
    shadow_uv = shadow_uv * 0.5 + 0.5;
    #if !SOKOL_GLSL
    shadow_uv.y = 1.0f - shadow_uv.y;
    #endif
    float bias = 0.5 * clamp(0.005 * tan(acos(cos_theta)), 0.0, 0.01);
    // current depth
    float bias_n = shadow_pos.z - bias;
//    float its = 1.0 / 2048.0;
//    float sum = 0.0;

    //for (float y = -1.5; y <= 1.5; y += 1.0) {
    //  for (float x = -1.5; x <= 1.5; x += 1.0) {
    //    sum += step(texture2D(u_image_shadow_map, shadow_uv + vec2(x, y) * its).x, bias_n);
    //  }
    //}
    //return sum / 16.0;

//    for (int y = -1; y <= 1; y += 1) {
//        for (int x = -1; x <= 1; x += 1) {
//            sum += step(decodeDepth(texture(u_image_shadow_map, shadow_uv + vec2(x, y) * its)), bias_n);
//        }
//    }
//    return sum / 9.0;

    return step(decodeDepth(texture(u_image_shadow_map, shadow_uv)), bias_n);
}

float oren_nayar_diffuse(vec3 light_direction, vec3 view_direction, vec3 surface_normal, float roughness, float albedo) {

    float LdotV = dot(light_direction, view_direction);
    float NdotL = dot(light_direction, surface_normal);
    float NdotV = dot(surface_normal, view_direction);

    float s = LdotV - NdotL * NdotV;
    float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

    float sigma2 = roughness * roughness;
    float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    return albedo * max(0.0, NdotL) * (A + B * s / t) / PI;
}

float lambert_diffuse(vec3 light_direction, vec3 normal) {
    return dot(normal, light_direction);
}

void main() {
    vec3 norm = normalize(vNormal);
    vec3 view_dir = normalize(uViewPos - vFragPos);
    vec3 total = mat_emission;
    vec4 color = texture(uImage0, vTexCoord) * vColorMult;
    vec3 material_diffuse = vec3(color) * mat_diffuse;

    // directional light
    {
        float diff = oren_nayar_diffuse(light_position, view_dir, norm, 0.5, 1.0);
        total += mat_ambient *  light_ambient;
        if (diff > 0.0) {
            total += (1.0 - calc_shadow(clamp(lambert_diffuse(light_position, norm), 0.0, 1.0))) * (
            mat_specular *  light_specular * calc_spec(light_position, view_dir, norm) +
            material_diffuse *  light_diffuse * diff
            );
        }
    }

    // point lights
    {
        vec3 light_vec =  light2_position - vFragPos;
        float distance = length(light_vec);
        vec3 light_dir = light_vec / distance;
        float diff = oren_nayar_diffuse(light_dir, view_dir, norm, 0.5, 1.0);

        total += mat_ambient * light2_ambient;
        if (diff > 0.0) {
            float falloff = attenuation_point(light_vec / light2_radius, light2_falloff);
            if (falloff > 0.0) {
                //falloff = falloff < random(vec3(gl_FragCoord.xy * u_resolution.zw, 0.0), 0) ? 0.0 : 1.0;
                total += falloff * (mat_specular * light2_specular * calc_spec(light_dir, view_dir, norm)
                + material_diffuse * light2_diffuse * diff);
            }
        }
    }

    //    total = tonemap_unreal(total);
    total = tonemap_reinhard(total);

    //    gl_FragColor = dither(vec4(total, color.w));
    frag_color = vec4(total, color.w);
}
#pragma sokol @end

#pragma sokol @program render3d vs fs

#pragma sokol @vs vs_skybox
uniform vs_skybox_params {
    mat4 uModelViewProjection;
};

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTexCoord;
in vec4 aColorMult;
in vec4 aColorOffset;

out vec3 vTexCoord;

void main() {
    vTexCoord = aPosition.xzy;
    vec4 pos = uModelViewProjection * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
}

#pragma sokol @end

#pragma sokol @fs fs_skybox

in vec3 vTexCoord;
uniform samplerCube imageSkybox;
out vec4 fragColor;

void main() {
    fragColor = texture(imageSkybox, vTexCoord);
}

#pragma sokol @end

#pragma sokol @program render3d_skybox vs_skybox fs_skybox