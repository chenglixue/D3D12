#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

using Microsoft::WRL::ComPtr;

using namespace DirectX;

struct Vertex
{
	Vertex() = default;
	Vertex(const Vertex& rhs)
	{
		this->position = rhs.position;
		this->normal = rhs.normal;
		this->tangent = rhs.tangent;
		this->texCoord = rhs.texCoord;
	}
	Vertex& operator= (Vertex& rhs)
	{
		return rhs;
	}

	Vertex(Vertex&& rhs) = default;

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 texCoord;
};

class Model
{
public:

	struct Mesh
	{
		Mesh() = default;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Mesh(std::vector<Vertex>& vertices, std::vector<UINT>& indices)
		{
			this->vertices = vertices;
			this->indices = indices;
		}
	};


	// load scene for assimp
	Model(const std::string& path);

	// Traverse and process the nodes in assimp in turn
	void TraverseNode(const aiScene* scene, aiNode* node);

	// load mesh, which includes vertex, index, normal, tangent, texture, material information
	Mesh LoadMesh(const aiScene* scene, aiMesh* mesh);

	std::vector<Vertex> GetVertices();

	std::vector<uint32_t> GetIndices();

private:
	std::string directory;
	std::vector<Mesh> m_meshs;
};