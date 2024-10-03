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

layout(set = 1, binding = 3) uniform RingsUniformBufferObject {
    mat4 worldMatrix;
    float innerRadius;
    float outerRadius;
} rubo;

layout(set = 1, binding = 1) uniform sampler2D tex;

float shadowFactor = 0.7;

float computeShadow(vec3 fragPos, vec3 lightPos, mat4 M, float inner, float outer) {
    // Transform fragment position and light position into Saturn's local space (rings' coordinate system)
    vec3 fragPosLocal = (inverse(M) * vec4(fragPos, 1.0)).xyz;
    vec3 lightPosLocal = (inverse(M) * vec4(lightPos, 1.0)).xyz;

    // Calculate the vector from light to fragment
    vec3 lightToFrag = normalize(fragPosLocal - lightPosLocal);

    // Ensure light source and fragment are on the correct sides of the ring plane
    if (lightPosLocal.y * fragPosLocal.y > 0.0) {
        return 1.0;  // Light and fragment are on the same side of the rings, no shadow
    }

    // Calculate the intersection point of the light ray with the ring plane (y = 0 in local space)
    float t = -lightPosLocal.y / lightToFrag.y;
    vec3 intersection = lightPosLocal + t * lightToFrag;

    // Calculate distance from Saturn's center to the intersection point in the xz-plane
    float distanceFromCenter = length(vec2(intersection.x, intersection.z));

    // Check if the intersection point lies within the ring's bounds
    if (distanceFromCenter >= inner && distanceFromCenter <= outer) {
        return shadowFactor;  // Fragment is in shadow
    }

    return 1.0;  // Fragment is not in shadow
}

void main() {

    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(gubo.lightPos - fragPos);  // Direction from fragment to light
    vec3 viewDir = normalize(gubo.eyePos - fragPos);     // Direction from fragment to viewer

    // Diffuse component (Lambertian )
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(gubo.lightColor);

    // Specular component (Blinn)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), ubo.shininess);
    vec3 specular = ubo.specularStrength * spec * vec3(gubo.lightColor);

    // Rim component
    float distanceToCamera = length(gubo.eyePos - fragPos);
    float distanceFactor = clamp(1.0 / (distanceToCamera * 0.5), 0.0, 1.0);
    float rimStrength = 1.0 - dot(norm, viewDir);
    float rimExponent = mix(10.0, 2.0, ubo.atmosphereThickness);
    rimStrength = pow(rimStrength, rimExponent) * distanceFactor;
    vec3 rimColor = rimStrength * ubo.atmosphereColor;


    vec3 texColor = texture(tex, fragUV).rgb;

    // Combine all effects
    vec3 finalColor = (diffuse + specular) * texColor + rimColor;

    // Add Shadow
    float shadow = computeShadow(fragPos, gubo.lightPos, rubo.worldMatrix, rubo.innerRadius, rubo.outerRadius);
    finalColor *= shadow;
    
    outColor = vec4(finalColor, 1.0);
   
}
