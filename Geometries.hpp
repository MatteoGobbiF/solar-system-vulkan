struct Vertex {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct RingParams {
    int slices;
    float innerRadius;
    float outerRadius;
    glm::vec3 tiltAxis;
    float tiltAngle;
};

void generateRing(std::vector<unsigned char>& vertices, std::vector<uint32_t>& indices, RingParams params) {
    int numVertices = params.slices * 4; // 2 vertices per slice, per side (top and bottom)
    int numIndices = params.slices * 12; // 6 indices per slice, per side (top and bottom)

    vertices.resize(numVertices * sizeof(Vertex));
    Vertex* V_vertex = (Vertex*)(&vertices[0]);

    indices.resize(numIndices);

    glm::vec3 center = glm::vec3(0.0f);
    glm::mat4 tiltMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(params.tiltAngle), params.tiltAxis);

    for (int i = 0; i < params.slices; i++) {
        float ang = 2 * M_PI * (float)i / (float)params.slices;

        //Create two circles using polar coordinates
        glm::vec3 innerPos = glm::vec3(params.innerRadius * cos(ang), 0.0f, params.innerRadius * sin(ang));
        glm::vec3 outerPos = glm::vec3(params.outerRadius * cos(ang), 0.0f, params.outerRadius * sin(ang));

        innerPos = glm::vec3(tiltMatrix * glm::vec4(innerPos, 1.0f)) + center;
        outerPos = glm::vec3(tiltMatrix * glm::vec4(outerPos, 1.0f)) + center;

        // Top side vertices
        V_vertex[i] = { innerPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, (float)i / (float)params.slices) };
        V_vertex[i + params.slices] = { outerPos, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, (float)i / (float)params.slices) };

        // Bottom side vertices (flipped normals)
        V_vertex[i + 2 * params.slices] = { innerPos, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, (float)i / (float)params.slices) };
        V_vertex[i + 3 * params.slices] = { outerPos, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, (float)i / (float)params.slices) };

        int nextIndex = (i + 1) % params.slices;

        // Indices for top side
        indices[6 * i]     = i;
        indices[6 * i + 1] = i + params.slices;
        indices[6 * i + 2] = nextIndex;
        indices[6 * i + 3] = nextIndex;
        indices[6 * i + 4] = i + params.slices;
        indices[6 * i + 5] = nextIndex + params.slices;

        // Indices for bottom side
        indices[6 * i + numIndices / 2]     = i + 2 * params.slices;
        indices[6 * i + 1 + numIndices / 2] = nextIndex + 2 * params.slices;
        indices[6 * i + 2 + numIndices / 2] = i + 3 * params.slices;
        indices[6 * i + 3 + numIndices / 2] = nextIndex + 2 * params.slices;
        indices[6 * i + 4 + numIndices / 2] = nextIndex + 3 * params.slices;
        indices[6 * i + 5 + numIndices / 2] = i + 3 * params.slices;
    }
}