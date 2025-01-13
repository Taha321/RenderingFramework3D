#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include "matrix.h"


namespace RenderingFramework3D {

class Renderer;
class WorldObject;
class Mesh {
public:
	Mesh(const Renderer& renderer, unsigned numVerts, unsigned numIndices);
	Mesh(const Renderer& renderer, const VertDataLayout& layout, unsigned numVerts, unsigned numIndices);

//	vertex data set functions
	//	set before loading to GPU
	void SetVertices(const std::vector<MathUtil::Vec<4>>& vertexBuffer);
	void SetVertexNormals(const std::vector<MathUtil::Vec<3>>& colourBuffer);
	void SetIndexBuffer(const std::vector<unsigned>& indexBuffer);

	//	fill custom vertex data buffers, format/type of this data is decided by the pipline used to draw
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<bool>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<uint32_t>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<int32_t>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<float>& data);
	void SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<double>& data);

//	getters
	unsigned GetNumVertices() const;
	unsigned GetNumIndices() const;

	const std::vector<MathUtil::Vec<4>>& GetVertices() const;
	const std::vector<MathUtil::Vec<3>>& GetVertexNormals() const;
	const std::vector<unsigned>& GetIndexBuffer() const;

	const std::vector<uint8_t>& GetCustomVertexData(unsigned shaderInputSlot) const;

//	GPU load/unload functions
	bool LoadMesh(bool dynamic=false);
    bool UnloadMesh();
	bool Reload(bool dynamic=false);

//	Dynamically set vertex data if loaded dynamically
	bool SetVertexDynamic(unsigned vertIndex, const MathUtil::Vec<4>& position);
	bool SetVertexNormalDynamic(unsigned vertIndex, const MathUtil::Vec<3>& normal);
	bool SetIndexDynamic(unsigned idx, unsigned vertIndex);

	bool SetCustomVertexDataDynamic(unsigned vertIndex, unsigned shaderInputSlot, uint8_t* data, unsigned maxSize);

public:
	static Mesh Quad(const Renderer& renderer);
	static Mesh Cube(const Renderer& renderer);
	static Mesh Icosphere(const Renderer& renderer, unsigned subdivisions);
	
private:
    class MeshInternal;
    std::shared_ptr<MeshInternal> _internal;
	
	Mesh(const std::shared_ptr<MeshInternal>& internal);

	friend WorldObject;
	friend Renderer;
};
}