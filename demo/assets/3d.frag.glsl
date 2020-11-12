#ifdef GL_ES
precision mediump float;
#else
#define highp
#endif

#define PI 3.14159265

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vFragPos;
varying vec4 vColorMult;
varying vec4 vColorOffset;
varying vec4 v_shadow_pos;

uniform sampler2D uImage0;
uniform sampler2D u_image_shadow_map;

uniform vec3 uViewPos;

struct material_t {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emission;
    float shininess;
    float roughness;
};

struct light_t {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float radius;
    float falloff;
};

uniform light_t u_lights[2];
uniform material_t u_material;

uniform vec4 u_time;
uniform vec4 u_resolution;

/*** tone mapping ***/

/**
 * Computes the luminance of the specified linear RGB color using the
 * luminance coefficients from Rec. 709.
 *
 * @public-api
 */
float luminance(const vec3 linear) {
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
	vec4 seed4 = vec4(seed,i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
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
    if(true)
    {
       float kEnergyConservation = (8.0 + u_material.shininess) / (8.0 * PI);
       vec3 halfway_dir = normalize(light_dir + view_dir);
       return kEnergyConservation * pow(max(dot(normal, halfway_dir), 0.0), u_material.shininess);
    }
    else
    {
       float kEnergyConservation = (2.0 + u_material.shininess) / (2.0 * PI);
       vec3 reflect_dir = reflect(-light_dir, normal);
       return kEnergyConservation * pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);
    }
}

float calc_shadow(const float cos_theta) {
    vec3 shadow_pos = v_shadow_pos.xyz / v_shadow_pos.w;
    shadow_pos = shadow_pos * 0.5 + 0.5;
    vec2 shadow_uv = shadow_pos.xy;
    float bias = 0.5 * clamp(0.005 * tan(acos(cos_theta)), 0.0, 0.01);
    // current depth
    float bias_n = shadow_pos.z - bias;
     //closestDepth < currentDepth - bias ? 0.0 : 1.0;
    float its = 1.0 / 2048.0;
    float sum = 0.0;

    //for (float y = -1.5; y <= 1.5; y += 1.0) {
    //  for (float x = -1.5; x <= 1.5; x += 1.0) {
    //    sum += step(texture2D(u_image_shadow_map, shadow_uv + vec2(x, y) * its).x, bias_n);
    //  }
    //}
    //return sum / 16.0;

    for (float y = -0.5; y <= 0.5; y += 1.0) {
          for (float x = -0.5; x <= 0.5; x += 1.0) {
            sum += step(texture2D(u_image_shadow_map, shadow_uv + vec2(x, y) * its).x, bias_n);
          }
        }
        return sum / 4.0;
}

float oren_nayar_diffuse(
  vec3 light_direction,
  vec3 view_direction,
  vec3 surface_normal,
  float roughness,
  float albedo) {

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
    vec3 total = u_material.emission;
    vec4 color = texture2D(uImage0, vTexCoord) * vColorMult;
    vec3 material_diffuse = vec3(color) * u_material.diffuse;

    /** directional light **/
    {
        vec3 light_dir = u_lights[0].position;
        float diff = oren_nayar_diffuse(light_dir, view_dir, norm, 0.5, 1.0);
        total += u_material.ambient * u_lights[0].ambient;
        if(diff > 0.0) {
            total += (1.0 - calc_shadow(clamp(lambert_diffuse(light_dir, norm), 0.0, 1.0))) * (
                u_material.specular * u_lights[0].specular * calc_spec(light_dir, view_dir, norm) +
                material_diffuse * u_lights[0].diffuse * diff
            );
        }
    }

    /** point lights **/
    {
        vec3 light_vec = u_lights[1].position - vFragPos;
        float distance = length(light_vec);
        vec3 light_dir = light_vec / distance;
        float diff = oren_nayar_diffuse(light_dir, view_dir, norm, 0.5, 1.0);

        total += u_material.ambient * u_lights[1].ambient;
        if(diff > 0.0) {
            float falloff = attenuation_point(light_vec / u_lights[1].radius, u_lights[1].falloff);
            if(falloff > 0.0) {
            //falloff = falloff < random(vec3(gl_FragCoord.xy * u_resolution.zw, 0.0), 0) ? 0.0 : 1.0;
            total += falloff * (u_material.specular * u_lights[1].specular * calc_spec(light_dir, view_dir, norm)
                                     + material_diffuse * u_lights[1].diffuse * diff);
            }
        }
    }

//    total = tonemap_unreal(total);
    total = tonemap_reinhard(total);

//    gl_FragColor = dither(vec4(total, color.w));
    gl_FragColor = vec4(total, color.w);

// Z BUFFER
//  float far = 1000.0;
//  float near = 0.1;
//  float z = gl_FragCoord.z * 2.0 - 1.0; // back to NDC
//  z = (2.0 * near * far) / (far + near - z * (far - near));
//  z = z / far;
//  gl_FragColor = vec4(vec3(z), 1.0);
}