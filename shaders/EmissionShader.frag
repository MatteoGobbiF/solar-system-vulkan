#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D tex;

void main() {
	vec3 Emit = texture(tex, fragUV).rgb;
	
	outColor = vec4(Emit, 1.0f);
}