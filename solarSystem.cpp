#include "modules/Starter.hpp"
#include "PlanetaryMotion.hpp"
#include "Geometries.hpp"
#include "modules/TextMaker.hpp"

const float distanceScale = 1.0f;
const float sizeScale = 1.0f;

std::vector<SingleText> outText = {
            {4, {"Press to follow:", "1 - Mercury, 2 - Venus, 3 - Earth, 4 - Mars, 5 - Jupiter, 6 - Saturn, 7 - Uranus, 8 - Neptune", "Press T to speed up time, G to slow down time", "Press M to toggle Milky Way, N to toggle Skybox"}, 0, 0},
            {4, {"Mercury", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Venus", "Press space to exit, Press V to change how we see Venus", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Earth", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Mars", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Jupiter", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Saturn", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Uranus", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0},
            {4, {"Neptune", "Press space to exit", "Press T to speed up time, G to slow down time", "You can move around with arrow keys, zoom in with Z, out with X"}, 0, 0}
        };

float sunDimension = 20.0f*sizeScale;
//Planets parameters
PlanetParams earthParams = {
    120.0f * distanceScale,           // Scaled orbital radius
    0.1f,                             // Orbital speed
    glm::radians(23.5f),              // Orbital inclination
    3.0f,                             // Spin speed
	1.0f * sizeScale				  // Radius
};

PlanetParams mercuryParams = {
	60.0f * distanceScale,
	0.15f,
	glm::radians(7.0f),
	1.5f,
	0.5f * sizeScale
};

PlanetParams venusParams = {
	90.0f * distanceScale,
	0.035f,
	glm::radians(177.4f),
	6.5f,
	0.95 * sizeScale
};

PlanetParams marsParams = {
	150.0f * distanceScale,
	0.08f,
	glm::radians(25.19f),
	2.4f,
	0.8f * sizeScale
};

PlanetParams moonParams = { //These are relative to Earth
	5.0f * distanceScale,
	0.5f, 
	glm::radians(5.145f),
	-0.5f,
	0.4f
};

PlanetParams jupiterParams = {
	300.0f * distanceScale,
	0.04f,
	glm::radians(1.3f),
	9.9f,
	10.0f * sizeScale
};

PlanetParams saturnParams = {
	400.0f * distanceScale,
	0.03f,
	glm::radians(26.7f),
	10.7f,
	9.5f * sizeScale
};

RingParams saturnRingsParams = {
	64,							//Slices
	1.2f,						//Inner Radius
	2.2f,						//Outer Radius
	glm::vec3(0.0f, 0.0f, 1.0f),//Tilt Axis
	0.0f						//Tilt angle
};

PlanetParams uranusParams = {
	700.0f * distanceScale,
	0.02f,
	glm::radians(98.0f),
	17.2f,
	7.5f * sizeScale
};

PlanetParams neptuneParams = {
	1000.0f * distanceScale,
	0.015f,
	glm::radians(1.77f),
	16.1f,
	8.0f * sizeScale
};

//Uniforms
struct EmissionUniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
};

struct SunUniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(4) float time;
};

struct PlanetUniformBufferObject {	//Generic planet uniform
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;	
};

//Uniforms for single planet parameters
struct EarthUniformBufferObject {
	alignas(4) float Pow;
	alignas(4) float Ang;
};

struct VenusUniformBufferObject {
	alignas(4) int showAtmosphereState;
};

struct RockyUniformBufferObject {
	alignas(4) float roughness;
};

struct GasUniformBufferObject {
	alignas(4) float shininess;
	alignas(4) float specularStrength;
	alignas(16) glm::vec3 atmosphereColor;
	alignas(4) float atmosphereThickness;
};

//UBO's for the shadows
struct PlanetCastingShadowUniformBufferObject {
	alignas(16) glm::mat4 planetWorldMatrix;
	alignas(4) float planetScaleFactor;
};

struct RingsCastingShadowUniformBufferObject {
	alignas(16) glm::mat4 worldMatrix;
	alignas(4) float ringsInnerRadius;
	alignas(4) float ringsOuterRadius;
};

struct SkyBoxUniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(4) bool showMilkyWay;
	alignas(4) bool skyboxEnabled;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};

//Vertex Descriptors
struct SkyBoxVertex {
	glm::vec3 pos;
};

struct SimpleVertex {
	glm::vec3 pos;
	glm::vec2 UV;
};

struct ComplexVertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct EarthVertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
	glm::vec4 tan;	
};

class SolarSystem : public BaseProject {
	protected:
	
	// Descriptor Layouts
	DescriptorSetLayout DSLGlobal;	// For Global values

	DescriptorSetLayout DSLEmission;
	DescriptorSetLayout DSLSun;
	DescriptorSetLayout DSLEarth;
	DescriptorSetLayout DSLMoon;
	DescriptorSetLayout DSLVenus;
	DescriptorSetLayout DSLRocky;
	DescriptorSetLayout DSLGas;
	DescriptorSetLayout DSLSaturn;
	DescriptorSetLayout DSLRings;
	DescriptorSetLayout DSLSkyBox;

	//Vertex Formats
	VertexDescriptor VDSimple;
	VertexDescriptor VDComplex;
	VertexDescriptor VDEarth;
	VertexDescriptor VDSkyBox;

	//Pipelines
	Pipeline PEmission;
	Pipeline PSun;
	Pipeline PEarth;
	Pipeline PMoon;
	Pipeline PVenus;
	Pipeline PRocky;
	Pipeline PGas;
	Pipeline PSaturn;
	Pipeline PRings;
	Pipeline PSkyBox;

	TextMaker txt;

	// Models, textures and Descriptor Sets
	DescriptorSet DSGlobal;

	Model Msun;
	Texture Tsun;
	DescriptorSet DSsun;

	Model Mearth;
	Texture TearthDiff, TearthSpec, TearthNorm, TearthEmiss, TearthClouds;
	DescriptorSet DSearth;

	Model Mmoon;
	Texture Tmoon;
	DescriptorSet DSmoon;

	Model Mmercury;
	Texture Tmercury;
	DescriptorSet DSmercury;

	Model Mvenus;
	Texture TvenusSur, TvenusAtm;
	DescriptorSet DSVenus;

	Model Mmars;
	Texture Tmars;
	DescriptorSet DSmars;

	Model Mjupiter;
	Texture Tjupiter;
	DescriptorSet DSjupiter;

	Model Msaturn;
	Texture Tsaturn;
	DescriptorSet DSsaturn;

	Model Mrings;
	Texture Trings;
	DescriptorSet DSrings;

	Model Muranus;
	Texture Turanus;
	DescriptorSet DSuranus;

	Model Mneptune;
	Texture Tneptune;
	DescriptorSet DSneptune;

	Model Mskybox;
	Texture Tskybox, TskyboxMilky;
	DescriptorSet DSskybox;

	// Other application parameters

	glm::vec3 CamPos = glm::vec3(sunDimension * 1.5, 0.0, 0.0); //Initial position
	glm::mat4 ViewMatrix;

	int displayTxt = 0;

	float Ar;

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 2000;
		windowHeight = 1000;
		windowTitle = "Solar System Simulator";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.1f, 0.1f, 0.1f, 1.0f};
		
		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}

	void localInit() {
		// Descriptor Set Layouts
		DSLGlobal.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1}
				});

		DSLEmission.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(EmissionUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}
				});
		DSLSun.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SunUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}
				});
		DSLEarth.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1},
					{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1},
					{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1},
					{5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1},
					{6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(EarthUniformBufferObject), 1},
					{7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PlanetCastingShadowUniformBufferObject), 1}
				  });
		DSLMoon.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RockyUniformBufferObject), 1},
					{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PlanetCastingShadowUniformBufferObject), 1}
				  });
		DSLVenus.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1},
					{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(VenusUniformBufferObject), 1},
				  });
		DSLRocky.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RockyUniformBufferObject), 1},
				  });
		DSLGas.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GasUniformBufferObject), 1},
				  });
		DSLSaturn.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GasUniformBufferObject), 1},
					{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(RingsCastingShadowUniformBufferObject), 1}
				  });
		DSLRings.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(PlanetUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PlanetCastingShadowUniformBufferObject), 1}
				  });
		DSLSkyBox.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(SkyBoxUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1},
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1}
				  });				  

		// Vertex descriptors
		VDSimple.init(this, {
				  {0, sizeof(SimpleVertex), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(SimpleVertex, UV),
				         sizeof(glm::vec2), UV}
				});
		VDComplex.init(this, {
				  {0, sizeof(ComplexVertex), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ComplexVertex, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ComplexVertex, norm),
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(ComplexVertex, UV),
				         sizeof(glm::vec2), UV},					 
				});
		VDEarth.init(this, {
				  {0, sizeof(EarthVertex), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(EarthVertex, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(EarthVertex, norm),
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(EarthVertex, UV),
				         sizeof(glm::vec2), UV},
				  {0, 3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(EarthVertex, tan),
				         sizeof(glm::vec4), TANGENT}						 
				});
		VDSkyBox.init(this, {
				  {0, sizeof(SkyBoxVertex), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SkyBoxVertex, pos),
				         sizeof(glm::vec3), POSITION}
				});

		// Pipelines
		PEmission.init(this, &VDSimple,  "shaders/EmissionVert.spv",    "shaders/EmissionFrag.spv", {&DSLEmission});
		PSun.init(this, &VDSimple, "shaders/SunVert.spv", "shaders/SunFrag.spv", {&DSLSun});
		PEarth.init(this, &VDEarth, "shaders/EarthVert.spv", "shaders/EarthFrag.spv", {&DSLGlobal, &DSLEarth});
		PMoon.init(this, &VDComplex, "shaders/MoonVert.spv", "shaders/MoonFrag.spv", {&DSLGlobal, &DSLMoon});
		PVenus.init(this, &VDComplex, "shaders/VenusVert.spv", "shaders/VenusFrag.spv", {&DSLGlobal, &DSLVenus});
		PRocky.init(this, &VDComplex, "shaders/RockyVert.spv", "shaders/RockyFrag.spv", {&DSLGlobal, &DSLRocky});
		PGas.init(this, &VDComplex, "shaders/GasVert.spv", "shaders/GasFrag.spv", {&DSLGlobal, &DSLGas});
		PSaturn.init(this, &VDComplex, "shaders/SaturnVert.spv", "shaders/SaturnFrag.spv", {&DSLGlobal, &DSLSaturn});
		PRings.init(this, &VDComplex, "shaders/RingsVert.spv", "shaders/RingsFrag.spv", {&DSLGlobal, &DSLRings});
		PSkyBox.init(this, &VDSkyBox, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", {&DSLSkyBox});
		PSkyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_BACK_BIT, false);

		// Create models
		Msun.init(this, &VDSimple, "models/Sphere.obj", OBJ);
		Mearth.init(this, &VDEarth, "models/Sphere.gltf", GLTF);
		Mmoon.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mmercury.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mvenus.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mmars.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mjupiter.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Msaturn.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Muranus.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mneptune.init(this, &VDComplex, "models/Sphere.obj", OBJ);
		Mskybox.init(this, &VDSkyBox, "models/SkyBoxCube.obj", OBJ);

		//Saturn Rings Model 
		std::vector<unsigned char> ringsVertices;
   		std::vector<uint32_t> ringsIndices;

		generateRing(ringsVertices, ringsIndices, saturnRingsParams);

		Mrings.vertices = ringsVertices;
		Mrings.indices = ringsIndices;
		Mrings.initMesh(this, &VDComplex);

		// Create the textures
		Tsun.init(this, "textures/2k_sun.jpg");

		TearthDiff.init(this, "textures/2k_earth_daymap.jpg");
		TearthSpec.init(this, "textures/2k_earth_specular_map.png");
		TearthNorm.init(this, "textures/2k_earth_normal_map.png", VK_FORMAT_R8G8B8A8_UNORM);
		TearthEmiss.init(this, "textures/2k_earth_nightmap.jpg");
		TearthClouds.init(this, "textures/2k_earth_clouds.jpg");
		
		Tmoon.init(this, "textures/2k_moon.jpg");

		Tmercury.init(this, "textures/2k_mercury.jpg");

		TvenusSur.init(this, "textures/2k_venus_surface.jpg");
		TvenusAtm.init(this, "textures/2k_venus_atmosphere.jpg");

		Tmars.init(this, "textures/2k_mars.jpg");

		Tjupiter.init(this, "textures/2k_jupiter.jpg");
		Tsaturn.init(this, "textures/2k_saturn.jpg");
		Trings.init(this, "textures/2k_saturn_ring_alpha.png");
		Turanus.init(this, "textures/2k_uranus.jpg");
		Tneptune.init(this, "textures/2k_neptune.jpg");

		Tskybox.init(this, "textures/2k_stars.jpg");
		TskyboxMilky.init(this, "textures/2k_stars_milky_way.jpg");

		// Descriptor pool sizes
		DPSZs.uniformBlocksInPool = 26;
		DPSZs.texturesInPool = 18;
		DPSZs.setsInPool = 13;

		txt.init(this, &outText);

		//Inizialization of the ViewMatrix
		ViewMatrix = glm::rotate(glm::translate(glm::mat4(1), -CamPos), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void pipelinesAndDescriptorSetsInit() {
		// Creation of Pipelines
		PEmission.create();
		PSun.create();
		PEarth.create();
		PMoon.create();
		PVenus.create();
		PRocky.create();
		PGas.create();
		PSaturn.create();
		PRings.create();
		PSkyBox.create();

		// Descriptor Sets Initialization
		DSsun.init(this, &DSLSun, {&Tsun});
		DSearth.init(this, &DSLEarth, {&TearthDiff, &TearthSpec, &TearthNorm, &TearthEmiss, &TearthClouds});
		DSmoon.init(this, &DSLMoon, {&Tmoon});
		DSmercury.init(this, &DSLRocky, {&Tmercury});
		DSVenus.init(this, &DSLVenus, {&TvenusSur, &TvenusAtm});
		DSmars.init(this, &DSLRocky, {&Tmars});
		DSjupiter.init(this, &DSLGas, {&Tjupiter});
		DSsaturn.init(this, &DSLSaturn, {&Tsaturn});
		DSrings.init(this, &DSLRings, {&Trings});
		DSuranus.init(this, &DSLGas, {&Turanus});
		DSneptune.init(this, &DSLGas, {&Tneptune});
		DSskybox.init(this, &DSLSkyBox, {&Tskybox, &TskyboxMilky});

		DSGlobal.init(this, &DSLGlobal, {});

		txt.pipelinesAndDescriptorSetsInit();		

	}

	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PEmission.cleanup();
		PSun.cleanup();
		PEarth.cleanup();
		PMoon.cleanup();
		PVenus.cleanup();
		PRocky.cleanup();
		PGas.cleanup();
		PSaturn.cleanup();
		PRings.cleanup();
		PSkyBox.cleanup();
		
		// Cleanup descriptor sets
		DSsun.cleanup();
		DSearth.cleanup();
		DSmoon.cleanup();
		DSmercury.cleanup();
		DSVenus.cleanup();
		DSmars.cleanup();
		DSjupiter.cleanup();
		DSsaturn.cleanup();
		DSrings.cleanup();
		DSuranus.cleanup();
		DSneptune.cleanup();
		DSskybox.cleanup();
		DSGlobal.cleanup();

		txt.pipelinesAndDescriptorSetsCleanup();

	}

	void localCleanup() {	

		//Cleanup Textures and Models
		Tsun.cleanup();
		Msun.cleanup();

		Mearth.cleanup();
		TearthDiff.cleanup();
		TearthSpec.cleanup();
		TearthNorm.cleanup();
		TearthEmiss.cleanup();
		TearthClouds.cleanup();

		Tmoon.cleanup();
		Mmoon.cleanup();

		Tmercury.cleanup();
		Mmercury.cleanup();

		TvenusSur.cleanup();
		TvenusAtm.cleanup();
		Mvenus.cleanup();

		Tmars.cleanup();
		Mmars.cleanup();

		Tjupiter.cleanup();
		Mjupiter.cleanup();

		Tsaturn.cleanup();
		Msaturn.cleanup();

		Trings.cleanup();
		Mrings.cleanup();

		Turanus.cleanup();
		Muranus.cleanup();

		Tneptune.cleanup();
		Mneptune.cleanup();

		Tskybox.cleanup();
		TskyboxMilky.cleanup();
		Mskybox.cleanup();

		// Cleanup descriptor set layouts
		DSLEmission.cleanup();
		DSLSun.cleanup();
		DSLEarth.cleanup();
		DSLMoon.cleanup();
		DSLVenus.cleanup();
		DSLRocky.cleanup();
		DSLGas.cleanup();
		DSLSaturn.cleanup();
		DSLRings.cleanup();
		DSLSkyBox.cleanup();
		DSLGlobal.cleanup();

		// Destroies the pipelines
		PEmission.destroy();
		PSun.destroy();
		PEarth.destroy();
		PMoon.destroy();
		PVenus.destroy();
		PRocky.destroy();
		PGas.destroy();
		PSaturn.destroy();
		PRings.destroy();
		PSkyBox.destroy();

		txt.localCleanup();		

	}
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		
		//Sun
		PSun.bind(commandBuffer);
		Msun.bind(commandBuffer);
		DSsun.bind(commandBuffer, PSun, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(Msun.indices.size()), 1, 0, 0, 0);

		//Earth
		PEarth.bind(commandBuffer);
		Mearth.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PEarth, 0, currentImage);
		DSearth.bind(commandBuffer, PEarth, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mearth.indices.size()), 1, 0, 0, 0);

		//Venus
		PVenus.bind(commandBuffer);
		Mvenus.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PVenus, 0, currentImage);
		DSVenus.bind(commandBuffer, PVenus, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mvenus.indices.size()), 1, 0, 0, 0);		

		//Moon
		PMoon.bind(commandBuffer);
		Mmoon.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PMoon, 0, currentImage);
		DSmoon.bind(commandBuffer, PMoon, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mmoon.indices.size()), 1, 0, 0, 0);

		//Mercury
		PRocky.bind(commandBuffer);
		Mmercury.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PRocky, 0, currentImage);
		DSmercury.bind(commandBuffer, PRocky, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mmercury.indices.size()), 1, 0, 0, 0);

		//Mars	
		PRocky.bind(commandBuffer);
		Mmars.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PRocky, 0, currentImage);
		DSmars.bind(commandBuffer, PRocky, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mmars.indices.size()), 1, 0, 0, 0);

		//Jupiter
		PGas.bind(commandBuffer);
		Mjupiter.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PGas, 0, currentImage);
		DSjupiter.bind(commandBuffer, PGas, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mjupiter.indices.size()), 1, 0, 0, 0);

		//Saturn
		PSaturn.bind(commandBuffer);
		Msaturn.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PSaturn, 0, currentImage);
		DSsaturn.bind(commandBuffer, PSaturn, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Msaturn.indices.size()), 1, 0, 0, 0);
		
		//Saturn's Rings
		PRings.bind(commandBuffer);
		Mrings.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PRings, 0, currentImage);
		DSrings.bind(commandBuffer, PRings, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mrings.indices.size()), 1, 0, 0, 0);

		//Uranus
		PGas.bind(commandBuffer);
		Muranus.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PGas, 0, currentImage);
		DSuranus.bind(commandBuffer, PGas, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Muranus.indices.size()), 1, 0, 0, 0);

		//Neptune
		PGas.bind(commandBuffer);
		Mneptune.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, PGas, 0, currentImage);
		DSneptune.bind(commandBuffer, PGas, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mneptune.indices.size()), 1, 0, 0, 0);

		//SkyBox
		PSkyBox.bind(commandBuffer);
		Mskybox.bind(commandBuffer);
		DSskybox.bind(commandBuffer, PSkyBox, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(Mskybox.indices.size()), 1, 0, 0, 0);

		
		txt.populateCommandBuffer(commandBuffer, currentImage, displayTxt);
		
	}

	void updateUniformBuffer(uint32_t currentImage) {
		//Basic Movement
		static bool debounce = false;
		static int curDebounce = 0;

		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		
		static float autoTime = true;
		static float time = 0.0;
		static float timeMultiplier = 1.0f; //To control the speed of time
		
		if(autoTime) {
			time = time + deltaT * timeMultiplier;
		}

		static float tTime = 0.0;
		const float TturnTime = 60.0f;
		const float TangTurnTimeFact = 1.0f / TturnTime;
		
		if(autoTime) {
			tTime = tTime + deltaT * timeMultiplier;
			tTime = (tTime > TturnTime) ? (tTime - TturnTime) : tTime;
		}
		

		const float ROT_SPEED = glm::radians(120.0f);
		const float MOVE_SPEED = 50.0f;
		
		glm::mat4 MovementMatrix = glm::mat4(1);
		// The Fly model update proc.
		MovementMatrix = glm::rotate(glm::mat4(1), ROT_SPEED * r.x * deltaT,
								 glm::vec3(1, 0, 0)) * MovementMatrix;
		MovementMatrix = glm::rotate(glm::mat4(1), ROT_SPEED * r.y * deltaT,
								 glm::vec3(0, 1, 0)) * MovementMatrix;
		MovementMatrix = glm::rotate(glm::mat4(1), -ROT_SPEED * r.z * deltaT,
								 glm::vec3(0, 0, 1)) * MovementMatrix;
		MovementMatrix = glm::translate(glm::mat4(1), -glm::vec3(
								   MOVE_SPEED * m.x * deltaT, MOVE_SPEED * m.y * deltaT, MOVE_SPEED * m.z * deltaT))
													   * MovementMatrix;	
		ViewMatrix = MovementMatrix * ViewMatrix;

		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		//Change time speed
		if(glfwGetKey(window, GLFW_KEY_T)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_T;

				timeMultiplier = timeMultiplier + 1.0f;
			}
		} else {
			if((curDebounce == GLFW_KEY_T) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		if(glfwGetKey(window, GLFW_KEY_G)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_G;

				timeMultiplier = timeMultiplier - 1.0f;
			}
		} else {
			if((curDebounce == GLFW_KEY_G) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		//Change Venus Atmosphere display model
		static int showVenusAtmosphereState = 0;

		if(glfwGetKey(window, GLFW_KEY_V)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_V;

				showVenusAtmosphereState = (showVenusAtmosphereState + 1) % 3;
			}
		} else {
			if((curDebounce == GLFW_KEY_V) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		//Change skybox visibility status 
		static bool showMilky = false;
		static bool skyboxEnabled = true;

		if(glfwGetKey(window, GLFW_KEY_M)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_M;

				showMilky = !showMilky;
			}
		} else {
			if((curDebounce == GLFW_KEY_M) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		if(glfwGetKey(window, GLFW_KEY_N)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_N;

				skyboxEnabled = !skyboxEnabled;
			}
		} else {
			if((curDebounce == GLFW_KEY_N) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}

		//Model to orbit a planet 
		static bool isFollowingPlanet = false;
		static int followedPlanet = -1;
		static float orbitRadius;
		bool followChanged = false;

		if (glfwGetKey(window, GLFW_KEY_1)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_1;

				isFollowingPlanet = true;
				followedPlanet = 0; // Mercury
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_1) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_2)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_2;

				isFollowingPlanet = true;
				followedPlanet = 1; // Venus
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_2) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_3)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_3;

				isFollowingPlanet = true;
				followedPlanet = 2; // Earth
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_3) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_4)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_4;

				isFollowingPlanet = true;
				followedPlanet = 3; // Mars
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_4) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_5)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_5;

				isFollowingPlanet = true;
				followedPlanet = 4; // Jupiter
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_5) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_6)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_6;

				isFollowingPlanet = true;
				followedPlanet = 5; // Saturn
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_6) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_7)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_7;

				isFollowingPlanet = true;
				followedPlanet = 6; // Uranus
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_7) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_8)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_8;

				isFollowingPlanet = true;
				followedPlanet = 7; // Neptune
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_8) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			if(!debounce) {
				debounce = true;
				curDebounce = GLFW_KEY_SPACE;

				isFollowingPlanet = false;
				followedPlanet = -1; // Stop following any planet
				followChanged = true;
			}
		} else {
			if((curDebounce == GLFW_KEY_SPACE) && debounce) {
				debounce = false;
				curDebounce = 0;
			}
		}			

		if (isFollowingPlanet) {
			glm::mat4 planetTransform;
			FollowCamParams followCamParams;

			switch (followedPlanet) {
				case 0: //Mercury
					planetTransform = calculateOrbit(time, mercuryParams);
					followCamParams = generateFollowCamParams(mercuryParams);
					displayTxt = 1;
					break;
				case 1: //Venus
					planetTransform = calculateOrbit(time, venusParams);
					followCamParams = generateFollowCamParams(venusParams);
					displayTxt = 2;
					break;
				case 2: //Earth
					planetTransform = calculateOrbit(time, earthParams);
					followCamParams = generateFollowCamParams(earthParams);
					displayTxt = 3;
					break;
				case 3: //Mars
					planetTransform = calculateOrbit(time, marsParams);
					followCamParams = generateFollowCamParams(marsParams);
					displayTxt = 4;
					break;
				case 4: //Jupiter
					planetTransform = calculateOrbit(time, jupiterParams);
					followCamParams = generateFollowCamParams(jupiterParams);
					displayTxt = 5;
					break;
				case 5: //Saturn
					planetTransform = calculateOrbit(time, saturnParams);
					followCamParams = generateFollowCamParams(saturnParams);
					displayTxt = 6;
					break;
				case 6: //Uranus
					planetTransform = calculateOrbit(time, uranusParams);
					followCamParams = generateFollowCamParams(uranusParams);
					displayTxt = 7;
					break;
				case 7: //Neptune
					planetTransform = calculateOrbit(time, neptuneParams);
					followCamParams = generateFollowCamParams(neptuneParams);
					displayTxt = 8;
					break;
				default:
					planetTransform = glm::mat4(1.0f); // Identity matrix if no planet is followed
					displayTxt = 0;
					break;
			}


			// Define static variables for spherical coordinates (angles in radians)
			static float azimuth = glm::radians(-45.0f); // Horizontal angle (yaw)
			static float elevation = glm::radians(0.0f); // Vertical angle (pitch)
			if(followChanged){
				orbitRadius = followCamParams.orbitRadius;
				azimuth = glm::radians(-45.0f);
				elevation = glm::radians(0.0f);
			}
			// Handle user input to change azimuth and elevation (for example, using arrow keys)
			const float ROTATION_SPEED = glm::radians(45.0f); // Rotation speed in radians per second

			if (glfwGetKey(window, GLFW_KEY_LEFT)) {
				azimuth += ROTATION_SPEED * deltaT; // Rotate left (increase azimuth)
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
				azimuth -= ROTATION_SPEED * deltaT; // Rotate right (decrease azimuth)
			}
			if (glfwGetKey(window, GLFW_KEY_UP)) {
				elevation += ROTATION_SPEED * deltaT; // Rotate up (increase elevation)
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN)) {
				elevation -= ROTATION_SPEED * deltaT; // Rotate down (decrease elevation)
			}

			// Clamp the elevation to avoid flipping the camera upside down
			elevation = glm::clamp(elevation, glm::radians(-89.0f), glm::radians(89.0f));

			// Handle zooming in/out
			if (glfwGetKey(window, GLFW_KEY_Z)) {
				orbitRadius -= followCamParams.zoomSpeed * deltaT; // Zoom in
			}
			if (glfwGetKey(window, GLFW_KEY_X)) {
				orbitRadius += followCamParams.zoomSpeed * deltaT; // Zoom out
			}

			
			// Clamp the orbit radius to prevent the camera from getting too close or too far
			orbitRadius = glm::clamp(orbitRadius, followCamParams.minZoom, followCamParams.maxZoom);

			// Calculate the offset in Cartesian coordinates from spherical coordinates
			glm::vec3 offset;
			offset.x = orbitRadius * cos(elevation) * sin(azimuth);
			offset.y = orbitRadius * sin(elevation);
			offset.z = orbitRadius * cos(elevation) * cos(azimuth);

			// Calculate the camera position by adding the offset to the planet's position
			glm::vec3 planetPos = glm::vec3(planetTransform[3]); // Extract planet's position
			glm::vec3 cameraPos = planetPos + offset; // Camera is at the offset relative to the planet

			// Set the view matrix to look at the planet
			ViewMatrix = glm::lookAt(cameraPos, planetPos, glm::vec3(0.0f, 1.0f, 0.0f));

		} 
		else
			displayTxt = 0;
		if(followChanged)
			RebuildPipeline();


		// Projection Matrix (Perspective)
		float farPlaneDistance = 2*neptuneParams.orbitRadius; //So everything is always visible even from neptune's POV
		glm::mat4 ProjMatrix = glm::perspective(glm::radians(60.0f), Ar, 0.1f, farPlaneDistance);
		ProjMatrix[1][1] *= -1;

		//View Projection Matrix
		glm::mat4 ViewPrj =  ProjMatrix * ViewMatrix;

		//Uniform Buffers
		// Global
		GlobalUniformBufferObject gubo{};
		gubo.lightPos = glm::vec3(0.0f, 0.0f, 0.0f); //Sun is in the middle of the solar system
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = glm::vec3(glm::inverse(ViewMatrix) * glm::vec4(0, 0, 0, 1));
		DSGlobal.map(currentImage, &gubo, 0);

		//SUN
		SunUniformBufferObject sunUbo{};
		sunUbo.mvpMat = ViewPrj * glm::scale(glm::translate(glm::mat4(1), gubo.lightPos), glm::vec3(sunDimension));
		sunUbo.time = time;
		DSsun.map(currentImage, &sunUbo, 0);

		//EARTH
		PlanetUniformBufferObject epubo{};
		EarthUniformBufferObject eubo{};
		PlanetCastingShadowUniformBufferObject mesubo{};
	
   		glm::mat4 earthWorldMatrix = glm::scale(calculateOrbit(time, earthParams) * calculateSpin(time, earthParams), glm::vec3(earthParams.dimension));
		epubo.mvpMat = ViewPrj * earthWorldMatrix;
		epubo.mMat = earthWorldMatrix;
		epubo.nMat = glm::transpose(glm::inverse(earthWorldMatrix));		
		DSearth.map(currentImage, &epubo, 0);

		//From assignment 10
		eubo.Pow = 200.0f;
		eubo.Ang = tTime * TangTurnTimeFact;
		DSearth.map(currentImage, &eubo, 6);

		//MOON
		PlanetUniformBufferObject moonpubo{};
		RockyUniformBufferObject moonubo{};
		PlanetCastingShadowUniformBufferObject emsubo{};

		//Moon world matrix with respect to earth world matrix
		glm::mat4 moonWorldMatrix = earthWorldMatrix * //Plus a rotation of 180 degrees to put the correct face towards the Earth
									glm::rotate(glm::scale(calculateOrbit(time, moonParams) * calculateSpin(time, moonParams), glm::vec3(moonParams.dimension)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		moonpubo.mvpMat = ViewPrj * moonWorldMatrix;
		moonpubo.mMat = moonWorldMatrix;
		moonpubo.nMat = glm::transpose(glm::inverse(moonWorldMatrix));
		DSmoon.map(currentImage, &moonpubo, 0);

		moonubo.roughness = 0.75f;
		DSmoon.map(currentImage, &moonubo, 2);

		//Uniform for earth's shadow on the moon
		emsubo.planetScaleFactor = earthParams.dimension;
		emsubo.planetWorldMatrix = earthWorldMatrix;
		DSmoon.map(currentImage, &emsubo, 3);

		//Uniform for the moon's shadow on Earth
		mesubo.planetWorldMatrix = moonWorldMatrix;
		mesubo.planetScaleFactor = moonParams.dimension;
		DSearth.map(currentImage, &mesubo, 7);

		//MERCURY
		PlanetUniformBufferObject mercpubo{};
		RockyUniformBufferObject mercubo{};

   		glm::mat4 mercuryWorldMatrix = glm::scale(calculateOrbit(time, mercuryParams) * calculateSpin(time, mercuryParams), glm::vec3(mercuryParams.dimension));
		mercpubo.mvpMat = ViewPrj * mercuryWorldMatrix;
		mercpubo.mMat = mercuryWorldMatrix;
		mercpubo.nMat = glm::transpose(glm::inverse(mercuryWorldMatrix));
		DSmercury.map(currentImage, &mercpubo, 0);

		mercubo.roughness = 0.7f;
		DSmercury.map(currentImage, &mercubo, 2);

		//VENUS
		PlanetUniformBufferObject venpubo{};
		VenusUniformBufferObject venubo{};

   		glm::mat4 venusWorldMatrix = glm::scale(calculateOrbit(time, venusParams) * calculateSpin(time, venusParams), glm::vec3(venusParams.dimension ));
		venpubo.mvpMat = ViewPrj * venusWorldMatrix;
		venpubo.mMat = venusWorldMatrix;
		venpubo.nMat = glm::transpose(glm::inverse(venusWorldMatrix)); 
		DSVenus.map(currentImage, &venpubo, 0);

		venubo.showAtmosphereState = showVenusAtmosphereState;
		DSVenus.map(currentImage, &venubo, 3);

		//MARS
		PlanetUniformBufferObject marspubo{};
		RockyUniformBufferObject marsubo{};

   		glm::mat4 marsWorldMatrix = glm::scale(calculateOrbit(time, marsParams) * calculateSpin(time, marsParams), glm::vec3(marsParams.dimension));
		marspubo.mvpMat = ViewPrj * marsWorldMatrix;
		marspubo.mMat = marsWorldMatrix;
		marspubo.nMat = glm::transpose(glm::inverse(marsWorldMatrix));
		DSmars.map(currentImage, &marspubo, 0);

		marsubo.roughness = 0.85f;
		DSmars.map(currentImage, &marsubo, 2);

		//JUPITER
		PlanetUniformBufferObject jupipubo{};
		GasUniformBufferObject jupiubo{};

		glm::mat4 jupiterWorldMatrix = glm::scale(calculateOrbit(time, jupiterParams) * calculateSpin(time, jupiterParams), glm::vec3(jupiterParams.dimension));
		jupipubo.mvpMat = ViewPrj * jupiterWorldMatrix;
		jupipubo.mMat = jupiterWorldMatrix;
		jupipubo.nMat = glm::transpose(glm::inverse(jupiterWorldMatrix));
		DSjupiter.map(currentImage, &jupipubo, 0);

		jupiubo.shininess = 10.0f;
		jupiubo.specularStrength = 0.1f;
		jupiubo.atmosphereColor = glm::vec3(1.0f, 0.7f, 0.4f);
		jupiubo.atmosphereThickness = 0.8f;
		DSjupiter.map(currentImage, &jupiubo, 2);

		//SATURN
		PlanetUniformBufferObject satpubo{};
		GasUniformBufferObject satubo{};
		RingsCastingShadowUniformBufferObject rssubo{};

		glm::mat4 saturnWorldMatrix = glm::scale(calculateOrbit(time, saturnParams) * calculateSpin(time, saturnParams), glm::vec3(saturnParams.dimension));
		satpubo.mvpMat = ViewPrj * saturnWorldMatrix;
		satpubo.mMat = saturnWorldMatrix;
		satpubo.nMat = glm::transpose(glm::inverse(saturnWorldMatrix));
		DSsaturn.map(currentImage, &satpubo, 0);

		satubo.shininess = 15.0f;
		satubo.specularStrength = 0.3f;
		satubo.atmosphereColor = glm::vec3(0.9f, 0.8f, 0.5f);
		satubo.atmosphereThickness = 0.7f;
		DSsaturn.map(currentImage, &satubo, 2);

		//Uniform for saturn's rings' shadow on Saturn
		rssubo.worldMatrix = saturnWorldMatrix;
		rssubo.ringsInnerRadius = saturnRingsParams.innerRadius;
		rssubo.ringsOuterRadius = saturnRingsParams.outerRadius;
		DSsaturn.map(currentImage, &rssubo, 3);

		//SATURN RINGS
		PlanetCastingShadowUniformBufferObject srsubo{};
		srsubo.planetScaleFactor = saturnParams.dimension;
		srsubo.planetWorldMatrix = saturnWorldMatrix;
		DSrings.map(currentImage, &satpubo, 0); //Same as Saturn since they are generated assuming saturn's center in 0,0,0
		DSrings.map(currentImage, &srsubo, 2); //This is for saturn's shadow onto the rings

		//URANUS
		PlanetUniformBufferObject urapubo{};
		GasUniformBufferObject uraubo{};

		glm::mat4 uranusWorldMatrix = glm::scale(calculateOrbit(time, uranusParams) * calculateSpin(time, uranusParams), glm::vec3(uranusParams.dimension));
		urapubo.mvpMat = ViewPrj * uranusWorldMatrix;
		urapubo.mMat = uranusWorldMatrix;
		urapubo.nMat = glm::transpose(glm::inverse(uranusWorldMatrix));
		DSuranus.map(currentImage, &urapubo, 0);

		uraubo.shininess = 25.0f;
		uraubo.specularStrength = 0.8f;
		uraubo.atmosphereColor = glm::vec3(0.5f, 0.9f, 1.0f);
		uraubo.atmosphereThickness = 0.5f;
		DSuranus.map(currentImage, &uraubo, 2);

		//NEPTUNE
		PlanetUniformBufferObject neptpubo{};
		GasUniformBufferObject neptubo{};

		glm::mat4 neptuneModelMatrix = glm::scale(calculateOrbit(time, neptuneParams) * calculateSpin(time, neptuneParams), glm::vec3(neptuneParams.dimension));
		neptpubo.mvpMat = ViewPrj * neptuneModelMatrix;
		neptpubo.mMat = neptuneModelMatrix;
		neptpubo.nMat = glm::transpose(glm::inverse(neptuneModelMatrix));
		DSneptune.map(currentImage, &neptpubo, 0);

		neptubo.shininess = 20.0f;
		neptubo.specularStrength = 0.35f;
		neptubo.atmosphereColor = glm::vec3(0.2f, 0.4f, 0.9f);
		neptubo.atmosphereThickness = 0.6f;
		DSneptune.map(currentImage, &neptubo, 2);

		//SKYBOX
		SkyBoxUniformBufferObject sbubo{};

		sbubo.mvpMat = ProjMatrix * glm::mat4(glm::mat3(ViewMatrix)); //Remove translation part, unnecessary for skybox
		sbubo.showMilkyWay = showMilky;
		sbubo.skyboxEnabled = skyboxEnabled;
		DSskybox.map(currentImage, &sbubo, 0);
	}
};


// This is the main: probably you do not need to touch this!
int main() {
    SolarSystem app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
