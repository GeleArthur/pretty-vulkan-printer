#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_samplerless_texture_functions: enable

layout (set = 0, binding = 0) uniform SceneGlobals {
    mat4x4 camera_view;
    mat4x4 camera_projection;
    vec3 position;
} sceneInfo;


layout (set = 1, binding = 0) uniform sampler shardedSampler;
layout (set = 1, binding = 1) uniform texture2D albedoImage;
layout (set = 1, binding = 2) uniform texture2D normalImage2;
layout (set = 1, binding = 3) uniform texture2D metalRoughnessImage;
layout (set = 1, binding = 4) uniform texture2D depthImage;

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
};

struct DirectionalLight {
    vec4 direction;
    vec4 color;
    float intensity;
};

layout (set = 2, binding = 0) uniform LightsPoint {
    uint count;
    PointLight point[10];
} lights_point;

layout (set = 2, binding = 1) uniform LightsDirection {
    uint count;
    DirectionalLight direction[10];
} lights_direction;

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
    n.x >= 0.0 ? -t : t,
    n.y >= 0.0 ? -t : t
    );

    // 4) Renormalize to unit length
    return normalize(n);
}

vec3 GetWolrdPositionFromDepth(in float depth, in ivec2 fragcoords, in ivec2 resolution, in mat4 invProj, in mat4 invView) {
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
    float a = roughness;
    const bool squareRougness = false;
    if (squareRougness) {
        a = roughness * roughness;
    }

    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
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
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    const vec2 normal = texelFetch(normalImage2, ivec2(gl_FragCoord.xy), 0).xy;
    const vec4 metalRoughness = texelFetch(metalRoughnessImage, ivec2(gl_FragCoord.xy), 0);
    const vec3 albedo = texelFetch(albedoImage, ivec2(gl_FragCoord.xy), 0).xyz;

    const vec3 N = DecodeNormalOcta(normal);// Normalize needed??
    //    outColor = vec4(N, 1.0f);
    //    return;

    const float roughness = metalRoughness.r;
    const float metallic = metalRoughness.g;
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

    for (int i = 0; i < lights_direction.count; ++i)
    {
        const vec3 lightDirection = lights_direction.direction[i].direction.xyz;
        const vec3 lightColor = lights_direction.direction[i].color.rgb;
        const float illuminace = lights_direction.direction[i].intensity;

        vec3 L = -lightDirection;
        vec3 H = normalize(V + L);
        vec3 irradiance = lightColor * illuminace;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * irradiance * NdotL;
    }

    for (int i = 0; i < lights_point.count; ++i)
    {
        vec3 lightPositions = lights_point.point[i].position.xyz;
        vec3 lightColors = lights_point.point[i].color.xyz;

        const float lumen = lights_point.point[i].intensity;
        const float luminousIntensity = lumen / (4.0 * PI);

        vec3 L = normalize(lightPositions - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions - WorldPos);
        const float attenuation = 1.0 / max((distance * distance), 0.00001f);
        const float illuminace = luminousIntensity * attenuation;
        vec3 irradiance = lightColors * illuminace;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * irradiance * NdotL;
    }

    vec3 ambient = vec3(0.33) * albedo;
    vec3 color = ambient + Lo;

    outColor = vec4(color, 1.0);
}