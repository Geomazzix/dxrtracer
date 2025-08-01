#include "dxrtracer/modelLoader.h"

#include <core/vath/vath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace dxray
{
	AssimpModelLoader::AssimpModelLoader(const Path& a_modelFilePath) :
		m_assetFilePath(a_modelFilePath)
	{}

	bool AssimpModelLoader::LoadModel()
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(m_assetFilePath.string(),
			aiProcess_MakeLeftHanded |
			aiProcess_FlipUVs |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
		);

		if (!scene)
		{
			DXRAY_ERROR("Assimp: %s", importer.GetErrorString());
			return false;
		}

		m_model.Meshes.reserve(scene->mNumMeshes);
		ProcessNode(scene->mRootNode, scene);
		return true;
	}

	void AssimpModelLoader::ProcessNode(const aiNode* a_pNode, const aiScene* a_pScene)
	{
		for (u32 meshIdx = 0; meshIdx < a_pNode->mNumMeshes; ++meshIdx)
		{
			const aiMesh* mesh = a_pScene->mMeshes[meshIdx];
			m_model.Meshes.push_back(ProcessMesh(mesh, a_pScene));
		}

		for (u32 childIdx = 0; childIdx < a_pNode->mNumChildren; ++childIdx)
		{
			ProcessNode(a_pNode->mChildren[childIdx], a_pScene);
		}
	}

	Mesh AssimpModelLoader::ProcessMesh(const aiMesh* const a_pMesh, const aiScene* const a_pScene)
	{
		Mesh mesh;

		mesh.Positions.reserve(a_pMesh->mNumVertices);
		for (usize vertexIdx = 0; vertexIdx < a_pMesh->mNumVertices; ++vertexIdx)
		{
			DXRAY_ASSERT(a_pMesh->HasPositions());

			aiVector3f position = a_pMesh->mVertices[vertexIdx];
			mesh.Positions.push_back({ position.x, position.y, position.z });

			if (a_pMesh->HasNormals())
			{
				aiVector3f normal = a_pMesh->mNormals[vertexIdx];
				mesh.Normals.push_back({ normal.x, normal.y, normal.z });
			}

			// #Todo: support more than one UV channel.
			if (a_pMesh->HasTextureCoords(0))
			{
				aiVector3D uvCoord = a_pMesh->mTextureCoords[0][vertexIdx];
				mesh.Uvs.push_back({ uvCoord.x, uvCoord.y });
			}
		}

		mesh.Indices.reserve(a_pMesh->mNumFaces * 3); // Always force triangulation when importing.
		for (usize faceIdx = 0; faceIdx < a_pMesh->mNumFaces; ++faceIdx)
		{
			const aiFace& face = a_pMesh->mFaces[faceIdx];
			mesh.Indices.push_back(face.mIndices[0]);
			mesh.Indices.push_back(face.mIndices[1]);
			mesh.Indices.push_back(face.mIndices[2]);
		}

		return mesh;
	}

	Model& AssimpModelLoader::GetModel()
	{
		return m_model;
	}
}