#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


layout(set = 0, binding = 0) uniform PlanetUniformBufferObject {
	vec3 lightPos;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

layout(set = 1, binding = 2) uniform RockyUniformBufferObject {
	float roughness;
} rubo;

layout(set = 1, binding = 1) uniform sampler2D tex;


void main() {

    vec3 baseColor = texture(tex, fragUV).rgb;

    //Oren - Nayar diffuse model
    vec3 N = normalize(fragNorm);                 // Surface normal
    vec3 V = normalize(gubo.eyePos - fragPos);    // View vector
    vec3 L = normalize(gubo.lightPos - fragPos);  // Light vector

    float theta_i = acos(dot(L, N));    // Angle between light and normal
    float theta_r = acos(dot(V, N));    // Angle between view and normal
    float alpha = max(theta_i, theta_r);
    float beta = min(theta_i, theta_r);

    // Compute A and B parameters
    float sigma2 = rubo.roughness * rubo.roughness;
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * (sigma2 / (sigma2 + 0.09));

    // Calculate vi and vr
    vec3 vi = normalize(L - dot(L, N) * N);
    vec3 vr = normalize(V - dot(V, N) * N);

    // Calculate G
    float G = max(0.0, dot(vi, vr));

    // Compute the diffuse term using Oren-Nayar model
    float Ld = max(0.0, dot(L, N));
    float diffuse = Ld * (A + B * G * sin(alpha) * tan(beta));

    // Calculate final color
    vec3 finalColor = baseColor * gubo.lightColor.rgb * diffuse;

    outColor = vec4(finalColor, 1.0);
}