#pragma once
#include <vector>
#include <unordered_map>
#include "matrix.h"
#include "mesh.h"
#include "renderer_internal.h"
#include "types_internal.h"

namespace RenderingFramework3D {

class Mesh::MeshInternal {
public:
	MeshInternal(const Renderer::RendererInternal& renderer, const VertDataLayout& layout, unsigned numVerts, unsigned numIndices);
	~MeshInternal();

	bool SetMeshDataLayout(const VertDataLayout& layout);

	void SetVertices(const std::vector<MathUtil::Vec<4>>& vertexBuffer);
	void SetVertexNormals(const std::vector<MathUtil::Vec<3>>& colourBuffer);
	void SetIndexBuffer(const std::vector<unsigned>& indexBuffer);

	//fill custom vertex data buffers, layout of this data is decided by the pipline used to draw
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<bool>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<uint32_t>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<int32_t>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<float>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<double>& data);

	unsigned GetNumVertices() const;
	unsigned GetNumIndices() const;

	const std::vector<MathUtil::Vec<4>>& GetVertices() const;
	const std::vector<MathUtil::Vec<3>>& GetVertexNormals() const;
	const std::vector<unsigned>& GetIndexBuffer() const;

	const std::vector<uint8_t>& GetCustomVertexData(unsigned shaderInputSlot) const;

	bool LoadMesh(bool dynamic);
    bool UnloadMesh();
	bool Reload(bool dynamic);

	bool SetVertexDynamic(unsigned idx, const MathUtil::Vec<4>& position);
	bool SetVertexNormalDynamic(unsigned idx, const MathUtil::Vec<3>& normal);
	bool SetIndexDynamic(unsigned idx, unsigned vertIndex);

	bool SetCustomVertexDataDynamic(unsigned vertIndex, unsigned shaderInputSlot, uint8_t* data, unsigned maxSize);

	bool AddCommandDrawMesh(VkCommandBuffer cmdBuffer, unsigned maxIndices);

private:
	unsigned _num_verts;
	unsigned _num_indices;

	std::vector<MathUtil::Vec<4>> _verts;
	std::vector<MathUtil::Vec<3>> _normals;
	std::vector<unsigned> _indices;
	std::unordered_map<int, std::vector<uint8_t>> _custom_data;

	void* _vertbuffer_mapped;
	void* _indexbuffer_mapped;
	BufferResources _vertbuffer_res;
	BufferResources _idxbuffer_res;

	VertDataLayout _layout;

	bool _loaded;
	bool _dynamic_load;

    unsigned _dev_id;
};
}