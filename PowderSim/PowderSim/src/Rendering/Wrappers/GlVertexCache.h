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

		friend GlVertexCache;
	};
	typedef GlMeshData* GlMeshID;

	class GlVertexCache
	{
	private:
		static unsigned VBO;
		static std::vector<GlMeshData*> meshes;
		static unsigned VBOSize;

		GlVertexCache();

	public:
		static void Setup();
		static void Shutdown();

		static unsigned GetVBOSize() { return VBOSize; }

		static GlMeshID CreateMesh(void* _vertexData, unsigned _vertexSize);
		static void DeleteMesh(GlMeshID _mesh);

		static void UpdateMeshVerts(GlMeshID _mesh, void* _vertexData, unsigned _vertexSize);

		static void GetVertexData(void* _ptr);
		static void GetVertexData(void* _ptr, GlMeshID _mesh);
		static unsigned GetVertexSize(GlMeshID _mesh) { return _mesh->vertSize; }
		static unsigned GetVertexOffset(GlMeshID _mesh) { return _mesh->vertOffset; }

		static void RebindBuffers();
	};
}
