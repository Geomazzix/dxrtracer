#pragma once
#include <core/containers/string.h>
#include <core/containers/array.h>
#include <core/vath/vath.h>

struct aiMesh;
struct aiNode;
struct aiScene;

namespace dxray
{
	struct Mesh
	{
		Array<vath::Vector3f> Positions;
		Array<vath::Vector3f> Normals;
		Array<vath::Vector2f> Uvs;
		Array<u32> Indices;
	};


	struct Model
	{
		Array<Mesh> Meshes;
		WString DebugName;
	};


	inline Model BuildQuadModel()
	{
		Mesh mesh;
		mesh.Positions = {
			{ -0.5f, 0, -0.5f },
			{ -0.5f, 0, 0.5f },
			{ 0.5f, 0, 0.5f },
			{ 0.5f, 0, -0.5f },
		};

		mesh.Normals = {
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
		};

		mesh.Uvs = 
		{
			{ 0.0f, 0.0f },
			{ 0.0f, 1.0f },
			{ 1.0f, 1.0f },
			{ 1.0f, 0.0f }
		};

		mesh.Indices = {
			0, 1, 2,
			0, 2, 3
		};

		return { { mesh } };
	}


	/*!
	 * @brief the AssimpModelLoader loads models in 3 steps:
	 * 1. Load the model from disk.
	 * 2. Parses it's hierarchical relationships to retrieve the transformations and resource indicies.
	 * 3. Queues the resource indicies for API back-end loading.
	 */
	class AssimpModelLoader final
	{
	public:
		AssimpModelLoader(const Path& a_modelFilePath);
		~AssimpModelLoader() = default;

		bool LoadModel();
		Model& GetModel();

	private:
		void ProcessNode(const aiNode* a_pNode, const aiScene* a_pScene);
		Mesh ProcessMesh(const aiMesh* a_pMesh, const aiScene* a_pScene);

		Path m_assetFilePath;
		Model m_model;
	};
}