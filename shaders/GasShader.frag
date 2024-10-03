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

layout(set = 1, binding = 2) uniform GasUniformBufferObject {
    float shininess;
	float specularStrength;
	vec3 atmosphereColor;
	float atmosphereThickness;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D tex;

void main() {
    
    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(gubo.lightPos - fragPos);  // Direction from fragment to light
    vec3 viewDir = normalize(gubo.eyePos - fragPos);     // Direction from fragment to viewer

    // Diffuse component (Lambertian)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(gubo.lightColor);

    // Specular component (Blinn)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), ubo.shininess);
    vec3 specular = ubo.specularStrength * spec * vec3(gubo.lightColor);

    vec3 texColor = texture(tex, fragUV).rgb;

    // Rim component
    float distanceToCamera = length(gubo.eyePos - fragPos);
    float distanceFactor = clamp(1.0 / (distanceToCamera * 0.5), 0.0, 1.0);
    float rimStrength = 1.0 - dot(norm, viewDir);
    float rimExponent = mix(10.0, 2.0, ubo.atmosphereThickness);
    rimStrength = pow(rimStrength, rimExponent) * distanceFactor;
    vec3 rimColor = rimStrength * ubo.atmosphereColor;

    // Combine all effects
    vec3 finalColor = (diffuse + specular) * texColor + rimColor;
    //vec3 finalColor = (diffuse + specular) * vec3(texColor);
    outColor = vec4(finalColor, 1.0f);
}
