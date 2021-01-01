#pragma once

#include <vector>


namespace powd::rendering
{
	class GlVertexCache;

	struct GlMeshData
	{
	private:
		size_t meshIndex;

		unsigned vertOffset;
		unsigned vertSize;

		unsigned indxOffset;
		unsigned indxSize;

		friend GlVertexCache;
	};
	typedef GlMeshData* GlMeshID;

	class GlVertexCache
	{
	private:
		static unsigned VBO;
		static unsigned EBO;
		static std::vector<GlMeshData*> meshes;
		static unsigned VBOSize;
		static unsigned EBOSize;

		GlVertexCache();

	public:
		static void Setup();
		static void Shutdown();

		static GlMeshID CreateMesh(void* _vertexData, unsigned _vertexSize, void* _indexData, unsigned _indexSize);
		static void DeleteMesh(GlMeshID _mesh);

		static void UpdateMeshVerts(GlMeshID _mesh, void* _vertexData, unsigned _vertexSize);
		static void UpdateMeshIndices(GlMeshID _mesh, void* _indexData, unsigned _indexSize);
		static void UpdateMeshData(GlMeshID _mesh, void* _vertexData, unsigned _vertexSize, void* _indexData, unsigned _indexSize);

		static void GetVertexData(void* _ptr);
		static void GetVertexData(GlMeshID _mesh, void* _ptr);
		static unsigned GetVertexSize(GlMeshID _mesh) { return _mesh->vertSize; }
		static unsigned GetVertexOffset(GlMeshID _mesh) { return _mesh->vertOffset; }

		static void GetIndexData(void* _ptr);
		static void GetIndexData(GlMeshID _mesh, void* _ptr);
		static unsigned GetIndexSize(GlMeshID _mesh) { return _mesh->indxSize; }
		static unsigned GetIndexOffset(GlMeshID _mesh) { return _mesh->indxOffset; }

		static void RebindBuffers();
	};
}
