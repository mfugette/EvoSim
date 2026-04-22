#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "Shader.h"
#include "stb_perlin.h"

class Terrain {
public:
	int Width;
	int Depth;
	float Scale; // noise zoom (smaller = smoother)
	float HeightMultiplier;
	int Octaves; // FMB layers

	Terrain(int width = 128, int depth = 128, float scale = 0.05f, float heightMultiplier = 20.0f, int octaves = 6) {
		generateHeightMap();
		buildMesh();
	}

	~Terrain() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	float GetHeight(int x, int z) const {
		if (x < 0 || x >= Width || z < 0 || z >= Depth)
			return 0.0f;
		return heightmap[z * Width + x];
	}
	
	float GetHeightInterpolated(float worldX, float worldZ) const {
		float hx = worldX + Width * 0.5f;
		float hz = worldZ + Depth * 0.5f;

		int x0 = static_cast<int>(hx);
		int z0 = static_cast<int>(hz);
		int x1 = x0 + 1;
		int z1 = z0 + 1;

		x0 = glm::clamp(x0, 0, Width - 1);
		x1 = glm::clamp(x1, 0, Width - 1);
		z0 = glm::clamp(z0, 0, Depth - 1);
		z1 = glm::clamp(z1, 0, Depth - 1);

		float tx = hx - static_cast<float>(x0);
		float tz = hz - static_cast<float>(z0);

		float h00 = GetHeight(x0, z0);
		float h10 = GetHeight(x1, z0);
		float h01 = GetHeight(x0, z1);
		float h11 = GetHeight(x1, z1);

		return glm::mix(glm::mix(h00, h10, tx), glm::mix(h01, h11, tx), tz);
	}

	void Draw(Shader &shader) {
		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("model", model);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}


private:
	unsigned int VAO, VBO, EBO;
	int indexCount;
	std::vector<float> heightmap;

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	void generateHeightMap() {
		heightmap.resize(Width * Depth);

		for (int z = 0; z < Depth; z++) {
			for (int x = 0; x < Width; x++) {
				float nx = x * Scale;
				float nz = z * Scale;
				float h = stb_perlin_noise3(nx, 0.0f, nz, 2.0f, 0.5f, Octaves);
				heightmap[z * Width + x] = h * HeightMultiplier;
			}
		}
	}

	void buildMesh() {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		vertices.reserve(Width * Depth);
		indices.reserve((Width - 1) * (Depth - 1) * 6);

		float halfW = Width * 0.5f;
		float halfD = Depth * 0.5f;

		for (int z = 0; z < Depth; z++) {
			for (int x = 0; x < Width; x++) {
				Vertex v;
				v.Position = glm::vec3(x = halfW, heightmap[z * Width + x], z - halfD);
				v.Normal = glm::vec3(0.0f);
				vertices.push_back(v);
			}
		}

		for (int z = 0; z < Depth - 1; z++) {
			for (int x = 0; x < Width - 1; x++) {
				unsigned int tl = z * Width + x;
				unsigned int bl = (z + 1) * Width + x;
				unsigned int tr = z * Width + (x + 1);
				unsigned int br = (z + 1) * Width + (x + 1);

				indices.push_back(tl);
				indices.push_back(bl);
				indices.push_back(tr);

				indices.push_back(bl);
				indices.push_back(br);
				indices.push_back(tr);
			}
		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			Vertex& v0 = vertices[indices[i]];
			Vertex& v1 = vertices[indices[i + 1]];
			Vertex& v2 = vertices[indices[i + 2]];

			glm::vec3 edge1 = v1.Position - v0.Position;
			glm::vec3 edge2 = v2.Position - v0.Position;
			glm::vec3 faceNormal = glm::cross(edge1, edge2);

			v0.Normal += faceNormal;
			v1.Normal += faceNormal;
			v2.Normal += faceNormal;
		}

		for (auto &v : vertices)
			v.Normal = glm::normalize(v.Normal);

		indexCount = static_cast<int>(indices.size());

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);




	}
};