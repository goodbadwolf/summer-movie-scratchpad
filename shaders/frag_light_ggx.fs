#version 400

in float data;
in vec3 position;
in vec3 normal;

uniform vec3 cameraloc;  // Camera position
uniform vec3 lightdir;   // Lighting direction
uniform vec3 lightloc;   // Lighting direction
uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha
uniform ivec2 resolution; // Viewport resolution

uniform sampler2D modelTexture;

out vec4 frag_color;


/*
PBR lighting adapted from: https://learnopengl.com/PBR/Theory
*/

const float PI = 3.14159265359;
// Base color for the surface
const vec3 BASE_COLOR = vec3(1.0, 1.0, 1.0);
// This controls whether the surface material acts as a plastic or metal.
// 1.0 is purely metallic and 0.0 is purely plastic
const float METALLIC = 0.5;
// This controls how tight the specular reflections are.
// 0.0 is perfectly specular and 1.0 is perfectly diffuse.
// Extreme values can cause errors in calculations, so avoid using 0.0 or 1.0
const float ROUGHNESS = 0.5;
const vec3 AMBIENT_LIGHT = vec3(0.3);

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraloc - position);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    // Albedo should be read from the texture instead of a constant.
    vec3 albedo = BASE_COLOR;
    float metallic = METALLIC;
    float roughness = ROUGHNESS;

    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    // calculate per-light radiance
    vec3 L = normalize(lightloc - position);
    vec3 H = normalize(V + L);
    float distance = length(lightloc - position);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float attentuation = 1.0;
    vec3 radiance = lightColor * attentuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    vec3 ambient = AMBIENT_LIGHT * albedo;
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    frag_color = vec4(color, 1.0);
}
