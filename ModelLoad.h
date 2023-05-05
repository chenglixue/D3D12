#pragma once
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include "Texture.h"
#include "Geometry.h"

using Microsoft::WRL::ComPtr;

using namespace DirectX;

namespace Model
{
	class ModelLoader
	{
	public:
		// load scene for assimp
		ModelLoader(const std::string& path);

		// Traverse and process the nodes in assimp in turn
		void TraverseNode(const aiScene* scene, aiNode* node);

		// load mesh, which includes vertex, index, normal, tangent, texture, material information
		Mesh LoadMesh(const aiScene* scene, aiMesh* mesh);

		// get complete model vertex
		std::vector<Vertex> GetVertices();

		// get complete model vertex index
		std::vector<uint32_t> GetIndices();

		std::string GetDirectory()
		{
			return m_directory;
		}

		std::string GetTextureName()
		{
			return m_textureName;
		}

	private:
		std::string m_directory;
		std::string m_textureName;
		std::vector<Mesh> m_meshs;
	};
}