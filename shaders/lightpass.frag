#version 450

layout (binding = 0) uniform sampler2D albedoImage;
layout (binding = 1) uniform sampler2D normalImage;

//layout(std140, set = 0, binding = 2) uniform ScreenUBO{
//    vec2 screensize;
//} uScreen;

layout (location = 0) out vec4 outColor;

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

void main() {

    vec4 normalAndOthers = texture(normalImage, gl_FragCoord.xy / vec2(800, 600));

    vec3 normal = DecodeNormalOcta(normalAndOthers.xy);

    outColor = dot(normal, normalize(vec3(-0.3, 0.5, 0))) * texture(albedoImage, gl_FragCoord.xy / vec2(800, 600));
}