#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_samplerless_texture_functions : enable

layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
} sceneInfo;

layout(set = 1, binding = 0) uniform sampler shardedSampler;
layout(set = 1, binding = 1) uniform texture2D albedoImage;
layout(set = 1, binding = 2) uniform texture2D normalImage;
layout(set = 1, binding = 3) uniform texture2D depthImage;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;

vec3 DecodeNormalOcta(vec2 f) {
    // 1) Remap from [0,1] to [-1,1]
    f = f * 2.0 - 1.0;

    // 2) Reconstruct Z and handle fold-over
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.0, 1.0);
    // 3) Unwrap (component‑wise)
    n.xy += vec2(
    n.x >= 0.0 ? -t :  t,
    n.y >= 0.0 ? -t :  t
    );

    // 4) Renormalize to unit length
    return normalize(n);
}



vec3 GetWolrdPositionFromDepth(in float depth, in ivec2 fragcoords, in ivec2 resolution, in mat4 invProj, in mat4 invView){
    vec2 ndc = vec2(
    (float(fragcoords.x) / resolution.x) * 2.0f - 1.0f,
    (float(fragcoords.y) / resolution.y) * 2.0f - 1.0f);
    //    ndc.y *= -1;
    const vec4 clipPos = vec4(ndc, depth, 1.0f);

    vec4 viewPos = invProj * clipPos;
    viewPos /= viewPos.w;

    vec4 worldPos = invView * viewPos;

    return worldPos.xyz;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    const vec4 normalMetalRougness = texelFetch(normalImage, ivec2(gl_FragCoord.xy), 0);

    const vec3 albedo = texelFetch(albedoImage, ivec2(gl_FragCoord.xy), 0).xyz;
    const vec3 N = normalize(DecodeNormalOcta(normalMetalRougness.xy));// Normalize needed??
    const float roughness = normalMetalRougness.b;
    const float metallic = normalMetalRougness.a;
    const float depth = texelFetch(sampler2D(depthImage, shardedSampler), ivec2(gl_FragCoord.xy), 0).r;

    const vec3 WorldPos = GetWolrdPositionFromDepth(
    depth,
    ivec2(gl_FragCoord.xy),
    textureSize(depthImage, 0),
    inverse(sceneInfo.camera_projection),
    inverse(sceneInfo.camera_view)
    );

    const vec3 V = normalize(sceneInfo.position - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    vec3 lightPositions = vec3(3.0f, 1.0f, 0.0f);
    vec3 lightColors = vec3(0.0f, 1.0f, 0.0f);

    {
        vec3 L = normalize(lightPositions - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.0);
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, 1.0);

    //    outColor = vec4(view, 1.0f);


    //    outColor = dot(normal, normalize(vec3(0.577, -0.577, -0.577))) * texelFetch(albedoImage, ivec2(gl_FragCoord.xy), 0);
    //    outColor = vec4(normal, 1.0);
}