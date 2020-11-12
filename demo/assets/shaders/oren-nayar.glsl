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