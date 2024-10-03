#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
    vec3 lightPos;
    vec4 lightColor;
    vec3 eyePos;
} gubo;

layout(set = 1, binding = 1) uniform sampler2D surfaceTex;
layout(set = 1, binding = 2) uniform sampler2D atmosphereTex;

layout(set = 1, binding = 3) uniform VenusUniformBufferObject {
	int showAtmosphere;
} ubo;

void main() {
    vec3 lightDir = normalize(gubo.lightPos - fragPos);
    vec3 viewDir = normalize(gubo.eyePos - fragPos);

    float diff = max(dot(fragNorm, lightDir), 0.0);
    vec3 surfaceColor = texture(surfaceTex, fragUV).rgb;
    vec3 atmosphereColor = texture(atmosphereTex, fragUV).rgb;

    vec3 finalColor;

    // Determine which texture to use based on showAtmosphere
    if (ubo.showAtmosphere == 1) {
        finalColor = atmosphereColor;
    } else if (ubo.showAtmosphere == 2) {
        finalColor = surfaceColor;
    } else {
        //Atmosphere more visible at the edges and at sharper view angles
        float blendFactor = 0.5 + 0.5*(0.5 * (1.0 - diff) + 0.5 * (1.0 - max(dot(fragNorm, viewDir), 0.0)));
        finalColor = mix(surfaceColor, atmosphereColor, blendFactor);
    }

    // Apply lighting
    vec3 color = diff * finalColor * gubo.lightColor.rgb;
    outColor = vec4(color, 1.0);
}
