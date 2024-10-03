#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct PlanetParams {
    float orbitRadius;
    float orbitSpeed;
    float orbitInclination;
    float spinSpeed;
    float dimension;
};

struct FollowCamParams {
	float orbitRadius;
	float minZoom;
	float maxZoom;	
    float zoomSpeed;
};

glm::mat4 calculateOrbit(float time, const PlanetParams& params) {
    float angle = time * params.orbitSpeed;
    glm::vec3 position = glm::vec3(
        params.orbitRadius * cos(angle),
        0.0f,
        params.orbitRadius * sin(angle)
    );
    glm::mat4 orbitTilt = glm::rotate(glm::mat4(1.0f), params.orbitInclination, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::translate(glm::mat4(1.0f), position) * orbitTilt;
}

glm::mat4 calculateSpin(float time, const PlanetParams& params) {
    float spinAngle = time * params.spinSpeed;
    return glm::rotate(glm::mat4(1.0f), spinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
}

FollowCamParams generateFollowCamParams(const PlanetParams& planetParams) {
    FollowCamParams camParams;

    camParams.orbitRadius = planetParams.dimension * 5.0f;

    camParams.minZoom = planetParams.dimension * 1.5f;
    camParams.maxZoom = planetParams.dimension * 20.0f;
    camParams.zoomSpeed = planetParams.dimension * 5.0f;

    return camParams;
}
