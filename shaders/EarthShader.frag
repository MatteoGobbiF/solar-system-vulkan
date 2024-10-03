#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform NormalMapUniformBufferObject {
	vec3 lightPos;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

layout(set = 1, binding = 6) uniform NormalMapParUniformBufferObject {
	float Pow;
	float Ang;
} ubo;

//For Ray Casting
layout(set = 1, binding = 7) uniform MoonUniformBufferObject {
	mat4 moonWorldMatrix;
	float moonScaleFactor;
} mubo;

layout(set = 1, binding = 1) uniform sampler2D texDiff;
layout(set = 1, binding = 2) uniform sampler2D texSpec;
layout(set = 1, binding = 3) uniform sampler2D texNM;
layout(set = 1, binding = 4) uniform sampler2D texEmit;
layout(set = 1, binding = 5) uniform sampler2D texClouds;

float shadowFactor = 0.2; //Mulitply factor if the fragment is in shadow

float computeShadow(vec3 fragPos, vec3 lightDir, mat4 M, float scale){
	//Compute moon position to check if the vertex in question is in shadow
	vec3 moonPos = (M * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
	vec3 toMoon = normalize(moonPos - fragPos);
	vec3 toSun = lightDir;
	float distance = length(moonPos - fragPos);
	float projection = dot(toMoon, toSun);

	float shadow =  (dot(fragNorm, toMoon) > 0.0 && dot(fragNorm, toSun) > 0.0) && (projection > 0.0) && (projection * projection > 1.0 - (scale / distance) * (scale / distance)) ? shadowFactor : 1.0;
	return shadow;
}

//Same as in assignment 10
void main() {
	vec3 lightDir = normalize(gubo.lightPos-fragPos);
	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
	mat3 tbn = mat3(Tan, Bitan, Norm);
	vec4 nMap = texture(texNM, fragUV);
	vec3 N = normalize(tbn * (vec3(-1, 1, -1) + nMap.rgb * vec3(2, -2, 2)));
	
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightColor = gubo.lightColor.rgb;
	
	float DiffInt = max(dot(N, lightDir),0.0);
	float CloudInt = max(dot(Norm, lightDir),0.0);
	float EmitInt = pow(1 - DiffInt,20);
	float SpecInt = pow(max(dot(N, normalize(lightDir + EyeDir)),0.0), ubo.Pow);
	vec3 DiffColor = texture(texDiff, fragUV).rgb; 
	vec3 SpecColor = texture(texSpec, fragUV).rgb; 
	vec3 CloudCol = texture(texClouds, fragUV + vec2(ubo.Ang,0)).rgb;

	vec3 Diffuse = (DiffColor * DiffInt + CloudCol * CloudInt ) * (1-EmitInt);
	vec3 Specular = (SpecColor + CloudCol.rgb * 0.05) * SpecInt;
	vec3 Emit = texture(texEmit, fragUV).rgb * EmitInt * (1 - CloudCol.g) + 0.01 * CloudCol.g;
	
	vec3 col  = ((Diffuse + Specular) * lightColor + Emit);

	
	// Apply shadow effect if the fragment is in the Moon's shadow
	float shadow = computeShadow(fragPos, lightDir, mubo.moonWorldMatrix, mubo.moonScaleFactor);
    col *= shadow; //inShadow = 1 if it's not in shadow, = shadowFactor if it's in shadow

	outColor = vec4(col, 1.0f);
}