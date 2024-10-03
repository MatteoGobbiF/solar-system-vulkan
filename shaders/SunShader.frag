#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	float time;
} ubo;

void main() {
	// Basic emission color from the texture
    vec3 texColor = texture(tex, fragUV).rgb;
    
    // Pulsing effect
    float pulse = 0.6 + 0.4 * sin(ubo.time * 5.0);  // Pulses between 0.6 and 1.0
    
    float brightness = dot(texColor, vec3(0.299, 0.587, 0.114)); //Standard values to compute brightness (Luma coefficients)
    float intensity = mix(1.0, pulse, brightness);
    
    vec3 finalColor = texColor * intensity;
    
    // Set the final color output
    outColor = vec4(finalColor, 1.0f);
}