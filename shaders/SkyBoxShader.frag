#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
    bool showMilky;
    bool isEnabled;
} ubo;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D stars;
layout(binding = 2) uniform sampler2D milky;

void main() {

	//If skybox is disabled don't compute anything
	if(!ubo.isEnabled)
		discard;

	//From assignment 10
	float yaw = -(atan(fragTexCoord.x, fragTexCoord.z)/6.2831853+0.5);
	float pitch = -(atan(fragTexCoord.y, sqrt(fragTexCoord.x*fragTexCoord.x+fragTexCoord.z*fragTexCoord.z))/3.14159265+0.5);

	//Pick the correct texture according to showMilky
	if(ubo.showMilky)
		outColor = texture(milky, vec2(yaw, pitch));
	else
		outColor = texture(stars, vec2(yaw, pitch));
}