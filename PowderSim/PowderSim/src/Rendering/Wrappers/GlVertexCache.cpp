#include "GlVertexCache.h"

#include "GL/glew.h"


namespace powd::rendering
{
	namespace
	{
		unsigned CopyBuffer = 0;
	}

	unsigned GlVertexCache::VBO = 0;
	unsigned GlVertexCache::EBO = 0;
	std::vector<GlMeshData*> GlVertexCache::meshes = {};
	unsigned GlVertexCache::VBOSize = 0;
	unsigned GlVertexCache::EBOSize = 0;



	void GlVertexCache::Setup()
	{
		unsigned buffers[3];
		glGenBuffers(3, buffers);
		CopyBuffer = buffers[0];
		VBO = buffers[1];
		EBO = buffers[2];

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	}
	void GlVertexCache::Shutdown()
	{
		if (VBO != 0)
		{
			unsigned buffers[3] = { EBO, VBO, CopyBuffer };
			glDeleteBuffers(3, buffers);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	GlMeshID GlVertexCache::CreateMesh(void* _vertexData, unsigned _vertexSize, void* _indexData, unsigned _indexSize)
	{
		GlMeshID meshID = new GlMeshData();
		meshID->vertSize = _vertexSize;
		meshID->indxSize = _indexSize;
		meshID->meshIndex = meshes.size();

		if (meshes.size() > 0)
		{
			glBindBuffer(GL_COPY_READ_BUFFER, VBO);

			void* bufferData = new char[VBOSize + _vertexSize];
			glGetBufferSubData(GL_COPY_READ_BUFFER, 0, VBOSize, bufferData); // copy all the data over to the temporary buffer
			memcpy((char*)bufferData + VBOSize, _vertexData, _vertexSize); // copy the new data onto the end of the temp buffer
			glBufferData(GL_COPY_READ_BUFFER, VBOSize + _vertexSize, bufferData, GL_STATIC_DRAW); // use the temp buffer to regenerate the buffer object
			delete bufferData;

			glBindBuffer(GL_COPY_READ_BUFFER, EBO);

			bufferData = new char[EBOSize + _indexSize];
			glGetBufferSubData(GL_COPY_READ_BUFFER, 0, EBOSize, bufferData); // do the same here but for the EBO
			memcpy((char*)bufferData + EBOSize, _indexData, _indexSize);
			glBufferData(GL_COPY_READ_BUFFER, EBOSize + _indexSize, bufferData, GL_STATIC_DRAW);
			delete bufferData;

			meshID->vertOffset = VBOSize;
			meshID->indxOffset = EBOSize;
		}
		else
		{
			glBindBuffer(GL_COPY_READ_BUFFER, VBO);
			glBufferData(GL_COPY_READ_BUFFER, _vertexSize, _vertexData, GL_STATIC_DRAW); // in this instance, they were already empty so we just generate the data from scratch
			glBindBuffer(GL_COPY_READ_BUFFER, EBO);
			glBufferData(GL_COPY_READ_BUFFER, _vertexSize, _vertexData, GL_STATIC_DRAW);
		}

		VBOSize += _vertexSize;
		EBOSize += _indexSize;

		meshes.push_back(meshID);
		return meshID;
	}
	void GlVertexCache::DeleteMesh(GlMeshID _mesh)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, VBO);

		void* bufferData = new char[VBOSize - _mesh->vertSize];
		glGetBufferSubData(GL_COPY_READ_BUFFER, 0, _mesh->vertOffset, bufferData); // copy all the data from before this mesh
		if (_mesh->meshIndex < meshes.size() - 1)
		{
			// if there is more data after the mesh, we copy that over
			glGetBufferSubData(GL_COPY_READ_BUFFER, meshes[_mesh->meshIndex + 1]->vertOffset, (meshes[meshes.size() - 1]->vertOffset + meshes[meshes.size() - 1]->vertSize) - meshes[_mesh->meshIndex + 1]->vertOffset, (char*)bufferData + _mesh->vertOffset);
			for (unsigned i = _mesh->meshIndex + 1; i < meshes.size(); i++)
			{ // we also shift the data offsets
				meshes[i]->vertOffset -= _mesh->vertSize;
			}
		}
		glBufferData(GL_COPY_READ_BUFFER, VBOSize - _mesh->vertSize, bufferData, GL_STATIC_DRAW);
		delete bufferData; // regen the buffer data

		glBindBuffer(GL_COPY_READ_BUFFER, EBO);

		bufferData = new char[EBOSize - _mesh->indxSize]; // same again but for the EBO
		glGetBufferSubData(GL_COPY_READ_BUFFER, 0, _mesh->indxOffset, bufferData);
		if (_mesh->meshIndex < meshes.size() - 1)
		{
			glGetBufferSubData(GL_COPY_READ_BUFFER, meshes[_mesh->meshIndex + 1]->indxOffset, (meshes[meshes.size() - 1]->indxOffset + meshes[meshes.size() - 1]->indxSize) - meshes[_mesh->meshIndex + 1]->indxOffset, (char*)bufferData + _mesh->vertOffset);
			for (unsigned i = _mesh->meshIndex + 1; i < meshes.size(); i++)
			{
				meshes[i]->indxOffset -= _mesh->indxSize;
				meshes[i - 1] = meshes[i];
			}
		}
		glBufferData(GL_COPY_READ_BUFFER, EBOSize - _mesh->indxSize, bufferData, GL_STATIC_DRAW);
		delete bufferData;

		VBOSize -= _mesh->vertSize;
		EBOSize -= _mesh->indxSize;
	}

	void GlVertexCache::UpdateMeshVerts(GlMeshID _mesh, void* _vertexData, unsigned _vertexSize)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, VBO);

		if (_mesh->vertSize == _vertexSize)
		{
			// no resizing necessary
			glBufferSubData(GL_COPY_READ_BUFFER, _mesh->vertOffset, _vertexSize, _vertexData);
		}
		else
		{
			unsigned deltaSize = _vertexSize - _mesh->vertSize;

			void* bufferData = new char[VBOSize + deltaSize];
			glGetBufferSubData(GL_COPY_READ_BUFFER, 0, _mesh->vertOffset, bufferData); // copy all the data from before this mesh
			if (_mesh->meshIndex < meshes.size() - 1)
			{
				// if there is more data after the mesh, we copy that over, leaving enough room for the mesh itself
				glGetBufferSubData(GL_COPY_READ_BUFFER, meshes[_mesh->meshIndex + 1]->vertOffset, (meshes[meshes.size() - 1]->vertOffset + meshes[meshes.size() - 1]->vertSize) - meshes[_mesh->meshIndex + 1]->vertOffset, (char*)bufferData + _mesh->vertOffset + _vertexSize);
				for (unsigned i = _mesh->meshIndex + 1; i < meshes.size(); i++)
				{ // we also shift the data offsets
					meshes[i]->vertOffset += deltaSize;
				}
			}
			memcpy((char*)bufferData + _mesh->vertOffset, _vertexData, _vertexSize);
			glBufferData(GL_COPY_READ_BUFFER, VBOSize + deltaSize, bufferData, GL_STATIC_DRAW);
			delete bufferData; // regen the buffer data
		}
	}
	void GlVertexCache::UpdateMeshIndices(GlMeshID _mesh, void* _indexData, unsigned _indexSize)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, EBO);

		if (_mesh->indxSize == _indexSize)
		{
			// no resizing necessary
			glBufferSubData(GL_COPY_READ_BUFFER, _mesh->indxOffset, _indexSize, _indexData);
		}
		else
		{
			unsigned deltaSize = _indexSize - _mesh->indxSize;

			void* bufferData = new char[EBOSize + deltaSize];
			glGetBufferSubData(GL_COPY_READ_BUFFER, 0, _mesh->indxOffset, bufferData); // copy all the data from before this mesh
			if (_mesh->meshIndex < meshes.size() - 1)
			{
				// if there is more data after the mesh, we copy that over, leaving enough room for the mesh itself
				glGetBufferSubData(GL_COPY_READ_BUFFER, meshes[_mesh->meshIndex + 1]->indxOffset, (meshes[meshes.size() - 1]->indxOffset + meshes[meshes.size() - 1]->indxSize) - meshes[_mesh->meshIndex + 1]->indxOffset, (char*)bufferData + _mesh->indxOffset + _indexSize);
				for (unsigned i = _mesh->meshIndex + 1; i < meshes.size(); i++)
				{ // we also shift the data offsets
					meshes[i]->indxOffset += deltaSize;
				}
			}
			memcpy((char*)bufferData + _mesh->indxOffset, _indexData, _indexSize);
			glBufferData(GL_COPY_READ_BUFFER, EBOSize + deltaSize, bufferData, GL_STATIC_DRAW);
			delete bufferData; // regen the buffer data
		}
	}
	void GlVertexCache::UpdateMeshData(GlMeshID _mesh, void* _vertexData, unsigned _vertexSize, void* _indexData, unsigned _indexSize)
	{
		UpdateMeshVerts(_mesh, _vertexData, _vertexSize);
		UpdateMeshIndices(_mesh, _indexData, _indexSize);
	}

	void GlVertexCache::GetVertexData(void* _ptr)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, VBO);
		glGetBufferSubData(GL_COPY_READ_BUFFER, 0, VBOSize, _ptr);
	}
	void GlVertexCache::GetVertexData(GlMeshID _mesh, void* _ptr)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, VBO);
		glGetBufferSubData(GL_COPY_READ_BUFFER, _mesh->vertOffset, _mesh->vertSize, _ptr);
	}
	void GlVertexCache::GetIndexData(void* _ptr)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, EBO);
		glGetBufferSubData(GL_COPY_READ_BUFFER, 0, EBOSize, _ptr);
	}
	void GlVertexCache::GetIndexData(GlMeshID _mesh, void* _ptr)
	{
		glBindBuffer(GL_COPY_READ_BUFFER, EBO);
		glGetBufferSubData(GL_COPY_READ_BUFFER, _mesh->indxOffset, _mesh->indxSize, _ptr);
	}

	void GlVertexCache::RebindBuffers()
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	}
}
