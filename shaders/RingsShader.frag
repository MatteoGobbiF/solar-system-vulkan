#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 1) uniform sampler2D ringTexture;  

layout(set = 0, binding = 0) uniform GlobalUBO {
    vec3 lightPos;     
    vec4 lightColor;     
    vec3 eyePos;        
} gubo;

layout(location = 0) in vec3 fragPos; 
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV; 

layout(set = 1, binding = 2) uniform SaturnUniformBufferObject {
    mat4 worldMatrix;      
    float scaleFactor;  
} subo;

layout(location = 0) out vec4 outColor;

float shadowFactor = 0.0; //Ring is completely dark if in shadow

float computeShadow(vec3 fragPos, vec3 lightPos, mat4 M, float planetRadius) {

    vec3 saturnCenter = (M * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    // Vector from the fragment to the Sun (light source)
    vec3 d_fs = lightPos - fragPos;

    // Vector from Saturn's center to the fragment
    vec3 d_f = fragPos - saturnCenter;

    // Coefficients for the quadratic equation
    float A = dot(d_fs, d_fs);
    float B = 2.0 * dot(d_f, d_fs);
    float C = dot(d_f, d_f) - planetRadius * planetRadius;

    // Solve the quadratic equation for the discriminant
    float discriminant = B * B - 4.0 * A * C;

    // If the discriminant is negative, no intersection occurs
    if (discriminant < 0.0) {
        return 1.0;  // No shadow
    }

    // Otherwise, solve for t using the quadratic formula
    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-B - sqrtDiscriminant) / (2.0 * A);
    float t2 = (-B + sqrtDiscriminant) / (2.0 * A);

    // Check if the intersection occurs between the fragment and the light source
    if ((t1 >= 0.0 && t1 <= 1.0) || (t2 >= 0.0 && t2 <= 1.0)) {
        return shadowFactor;  // In shadow
    }

    return 1.0;  // No shadow
}

void main() {
    vec3 normal = normalize(fragNorm);

    vec3 lightDir = normalize(gubo.lightPos - fragPos);

    // Lambert Diffusion
    float diff = max(dot(lightDir, normal), 0.0);

    vec3 textureColor = texture(ringTexture, fragUV).rgb;

    vec3 ambient = 0.05 * textureColor;  // Ambient component
    vec3 diffuse = diff * textureColor * gubo.lightColor.rgb; // Diffuse component

    //Darken if in shadow
    float shadow = computeShadow(fragPos, gubo.lightPos, subo.worldMatrix, subo.scaleFactor);
    
    // Combine the components and output the final color
    vec3 finalColor = (ambient + diffuse)*shadow;
    outColor = vec4(finalColor, 1.0);

}
