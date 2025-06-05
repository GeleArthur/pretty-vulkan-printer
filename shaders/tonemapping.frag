#version 450
#pragma shader_stage(fragment)
#extension GL_EXT_samplerless_texture_functions: enable


layout (set = 0, binding = 0) uniform texture2D lightpassTexture;
layout (location = 0) out vec4 outColor;

vec3 ACESFilmToneMapping(in vec3 color) {
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0f, 1.0f);
}

float CalulateEV100FromPhysicalCamera(in float aperture, in float shutterTime, in float ISO) {
    return log2(pow(aperture, 2) / shutterTime * 100 / ISO);
}

float ConvertEV100ToExposure(in float EV100) {
    const float maxLuminance = 1.2f * pow(2.0f, EV100);
    return 1.0f / max(maxLuminance, 0.0001f);
}

void main() {
    const vec3 image = texelFetch(lightpassTexture, ivec2(gl_FragCoord.xy), 0).xyz;

    #define INDOOR

    #ifdef SUNNY_16
    float apature = 5.f;
    float ISO = 100.f;
    float shutterSpeed = 1.f / 200.0f;
    #endif

    #ifdef INDOOR
    float apature = 1.4f;
    float ISO = 1600.f;
    float shutterSpeed = 1.f / 60.0f;
    #endif

    const float EV100_HardCoded = 1.0f;
    const float EV100_PhysicalCamera = CalulateEV100FromPhysicalCamera(apature, ISO, shutterSpeed);

    float exposure = ConvertEV100ToExposure(EV100_PhysicalCamera);

    outColor = vec4(ACESFilmToneMapping(image * exposure), 1.0f);
}