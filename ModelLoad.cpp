#include "pch.h"
#include "ModelLoad.h"

namespace Model
{
	ModelLoader::ModelLoader(const std::string& path)
	{
		Assimp::Importer localImporter;

#define AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE	// allows to specify a maximum smoothing angle for the algorithm for aiProcess_CalcTangentSpace

		const aiScene* pLocalScene = localImporter.ReadFile(
			path,
			// Triangulates all faces of all meshes
			aiProcess_Triangulate |
			// Supersedes the aiProcess_MakeLeftHanded and aiProcess_FlipUVs and aiProcess_FlipWindingOrder flags
			aiProcess_ConvertToLeftHanded |
			// This preset enables almost every optimization step to achieve perfectly optimized data. In D3D, need combine with aiProcess_ConvertToLeftHanded
			aiProcessPreset_TargetRealtime_MaxQuality |
			// Calculates the tangents and bitangents for the imported meshes
			aiProcess_CalcTangentSpace |
			// Splits large meshes into smaller sub-meshes
			// This is quite useful for real-time rendering, 
			// where the number of triangles which can be maximally processed in a single draw - call is limited by the video driver / hardware
			aiProcess_SplitLargeMeshes |
			// A postprocessing step to reduce the number of meshes
			aiProcess_OptimizeMeshes |
			// A postprocessing step to optimize the scene hierarchy
			aiProcess_OptimizeGraph |
			// Calculates the tangents and bitangents for the imported meshes
			aiProcess_CalcTangentSpace
		);

		// "localScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE" is used to check whether value data returned is incomplete
		if (pLocalScene == nullptr || pLocalScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || pLocalScene->mRootNode == nullptr)
		{
			std::cout << "ERROR::ASSIMP::" << localImporter.GetErrorString() << std::endl;
		}

		m_directory = path.substr(0, path.find_last_of('/')) + '/';

		std::string temp = path.substr(m_directory.size());
		m_textureName = temp.substr(0, temp.find_last_of('.'));

		TraverseNode(pLocalScene, pLocalScene->mRootNode);
	}

	void ModelLoader::TraverseNode(const aiScene* scene, aiNode* node)
	{
		// load mesh
		for (UINT i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* pLocalMesh = scene->mMeshes[node->mMeshes[i]];
			m_meshs.push_back(LoadMesh(scene, pLocalMesh));
		}

		// after we've processed all of the meshes then recursively process each of the children nodes
		for (UINT i = 0; i < node->mNumChildren; ++i)
		{
			TraverseNode(scene, node->mChildren[i]);
		}
	}

	Mesh ModelLoader::LoadMesh(const aiScene* scene, aiMesh* mesh)
	{
		std::vector<Vertex> localVertices;
		std::vector<uint32_t> localIndices;

		// process vertex position, normal, tangent, texture coordinates
		for (UINT i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex localVertex;

			localVertex.position.x = mesh->mVertices[i].x;
			localVertex.position.y = mesh->mVertices[i].y;
			localVertex.position.z = mesh->mVertices[i].z;

			localVertex.normal.x = mesh->mNormals[i].x;
			localVertex.normal.y = mesh->mNormals[i].y;
			localVertex.normal.z = mesh->mNormals[i].z;

			if (mesh->mTangents != nullptr && mesh->mBitangents != nullptr)
			{
				localVertex.tangent.x = mesh->mTangents[i].x;
				localVertex.tangent.y = mesh->mTangents[i].y;
				localVertex.tangent.z = mesh->mTangents[i].z;

				/*
				localVertex.bitNormal.x = mesh->mBitangents[i].x;
				localVertex.bitNormal.y = mesh->mBitangents[i].y;
				localVertex.bitNormal.z = mesh->mBitangents[i].z;
				*/
			}

			// assimp allow one model have 8 different texture coordinates in one vertex, but we just care first texture coordinates because we will not use so many
			if (mesh->mTextureCoords[0])
			{
				localVertex.texCoord.x = mesh->mTextureCoords[0][i].x;
				localVertex.texCoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				localVertex.texCoord = XMFLOAT2(0.0f, 0.0f);
			}

			localVertices.push_back(localVertex);
		}

		for (UINT i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace localFace = mesh->mFaces[i];
			for (UINT j = 0; j < localFace.mNumIndices; ++j)
			{
				localIndices.push_back(localFace.mIndices[j]);
			}
		}

		aiMaterial* pMaterial = scene->mMaterials[mesh->mMaterialIndex];

		return Mesh(localVertices, localIndices);
	}

	std::vector<Vertex> ModelLoader::GetVertices()
	{
		std::vector<Vertex> localVertices;

		for (auto& m : m_meshs)
		{
			for (auto& v : m.vertices)
			{
				localVertices.push_back(v);
			}
		}

		return localVertices;
	}

	std::vector<uint32_t> ModelLoader::GetIndices()
	{
		std::vector<uint32_t> localIndices;

		for (auto& m : m_meshs)
		{
			for (auto& i : m.indices)
			{
				localIndices.push_back(i);
			}
		}

		return localIndices;
	}
}

