#pragma once

#include "renderer.h"
#include "mesh_internal.h"


namespace RenderingFramework3D {


using namespace MathUtil;


Mesh::Mesh(const Renderer& renderer, unsigned numVerts, unsigned numIndices) {
    _internal = std::make_shared<MeshInternal>(*renderer._internal, VertDataLayout(), numVerts, numIndices);
}

Mesh::Mesh(const Renderer& renderer, const VertDataLayout& layout, unsigned numVerts, unsigned numIndices) {
    _internal = std::make_shared<MeshInternal>(*renderer._internal, layout, numVerts, numIndices);
}

Mesh::Mesh(const std::shared_ptr<MeshInternal>& internal) {
    _internal = internal;
}

void Mesh::SetVertices(const std::vector<Vec<4>>& vertexBuffer) {
	_internal->SetVertices(vertexBuffer);
}

void Mesh::SetVertexNormals(const std::vector<Vec<3>>& normalBuffer) {
	_internal->SetVertexNormals(normalBuffer);
}

void Mesh::SetIndexBuffer(const std::vector<unsigned>& indexBuffer) {
	_internal->SetIndexBuffer(indexBuffer);
}

void Mesh::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<bool>& data) {
	_internal->SetCustomVertexDataBuffer(shaderInputSlot, data);
}
void Mesh::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<uint32_t>& data) {
	_internal->SetCustomVertexDataBuffer(shaderInputSlot, data);
}
void Mesh::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<int32_t>& data) {
	_internal->SetCustomVertexDataBuffer(shaderInputSlot, data);
}
void Mesh::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<float>& data) {
	_internal->SetCustomVertexDataBuffer(shaderInputSlot, data);
}
void Mesh::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<double>& data) {
	_internal->SetCustomVertexDataBuffer(shaderInputSlot, data);
}

unsigned Mesh::GetNumVertices() const {
	return _internal->GetNumVertices();
}
unsigned Mesh::GetNumIndices() const {
	return _internal->GetNumIndices();
}

const std::vector<Vec<4>>& Mesh::GetVertices() const {
	return _internal->GetVertices();
}
const std::vector<Vec<3>>& Mesh::GetVertexNormals() const {
	return _internal->GetVertexNormals();
}
const std::vector<unsigned>& Mesh::GetIndexBuffer() const {
	return _internal->GetIndexBuffer();
}

const std::vector<uint8_t>& Mesh::GetCustomVertexData(unsigned shaderInputSlot) const {
	return _internal->GetCustomVertexData(shaderInputSlot);
}

bool Mesh::LoadMesh(bool dynamic) {
    return _internal->LoadMesh(dynamic);
}
bool Mesh::UnloadMesh() {
    return _internal->UnloadMesh();
}
bool Mesh::Reload(bool dynamic) {
    return _internal->Reload(dynamic);
}

bool Mesh::SetVertexDynamic(unsigned vertIndex, const MathUtil::Vec<4>& position) {
    return _internal->SetVertexDynamic(vertIndex, position);
}
bool Mesh::SetVertexNormalDynamic(unsigned vertIndex, const MathUtil::Vec<3>& normal) {
    return _internal->SetVertexNormalDynamic(vertIndex, normal);
}
bool Mesh::SetIndexDynamic(unsigned idx, unsigned vertIndex) {
    return _internal->SetIndexDynamic(idx, vertIndex);
}

bool Mesh::SetCustomVertexDataDynamic(unsigned vertIndex, unsigned shaderInputSlot, uint8_t* data, unsigned maxSize) {
    return _internal->SetCustomVertexDataDynamic(vertIndex, shaderInputSlot, data, maxSize);
}

Mesh Mesh::Cube(const Renderer& renderer) {
    std::vector<Vec<4>> cubeVerts = {
        Vec<4>({ 1.0f,  1.0f,  1.0f, 1}),
        Vec<4>({ 1.0f,  1.0f, -1.0f, 1}),
        Vec<4>({-1.0f,  1.0f, -1.0f, 1}),
        Vec<4>({-1.0f,  1.0f,  1.0f, 1}),
        
        Vec<4>({ 1.0f,  1.0f,  1.0f, 1}),
        Vec<4>({-1.0f,  1.0f,  1.0f, 1}),
        Vec<4>({-1.0f, -1.0f,  1.0f, 1}),
        Vec<4>({ 1.0f, -1.0f,  1.0f, 1}),
        
        Vec<4>({1.0f,   1.0f,  1.0f, 1}),
        Vec<4>({1.0f,  -1.0f,  1.0f, 1}),
        Vec<4>({1.0f,  -1.0f, -1.0f, 1}),
        Vec<4>({1.0f,   1.0f, -1.0f, 1}),
        
        Vec<4>({-1.0f,  1.0f,  1.0f, 1}),
        Vec<4>({-1.0f,  1.0f, -1.0f, 1}),
        Vec<4>({-1.0f, -1.0f, -1.0f, 1}),
        Vec<4>({-1.0f, -1.0f,  1.0f, 1}),
        
        Vec<4>({-1.0f, -1.0f,  1.0f, 1}),
        Vec<4>({-1.0f, -1.0f, -1.0f, 1}),
        Vec<4>({ 1.0f, -1.0f, -1.0f, 1}),
        Vec<4>({ 1.0f, -1.0f,  1.0f, 1}),
        
        Vec<4>({ 1.0f,  1.0f, -1.0f, 1}),
        Vec<4>({ 1.0f, -1.0f, -1.0f, 1}),
        Vec<4>({-1.0f, -1.0f, -1.0f, 1}),
        Vec<4>({-1.0f,  1.0f, -1.0f, 1}),
    };
    std::vector<Vec<3>> cubeNormals = {
        Vec<3>({ 0.0f,  1.0f,  0.0f}),
        Vec<3>({ 0.0f,  1.0f,  0.0f}),
        Vec<3>({ 0.0f,  1.0f,  0.0f}),
        Vec<3>({ 0.0f,  1.0f,  0.0f}),

        Vec<3>({ 0.0f,  0.0f,  1.0f}),
        Vec<3>({ 0.0f,  0.0f,  1.0f}),
        Vec<3>({ 0.0f,  0.0f,  1.0f}),
        Vec<3>({ 0.0f,  0.0f,  1.0f}),

        Vec<3>({ 1.0f,  0.0f,  0.0f}),
        Vec<3>({ 1.0f,  0.0f,  0.0f}),
        Vec<3>({ 1.0f,  0.0f,  0.0f}),
        Vec<3>({ 1.0f,  0.0f,  0.0f}),

        Vec<3>({-1.0f,  0.0f,  0.0f}),
        Vec<3>({-1.0f,  0.0f,  0.0f}),
        Vec<3>({-1.0f,  0.0f,  0.0f}),
        Vec<3>({-1.0f,  0.0f,  0.0f}),

        Vec<3>({ 0.0f, -1.0f,  0.0f}),
        Vec<3>({ 0.0f, -1.0f,  0.0f}),
        Vec<3>({ 0.0f, -1.0f,  0.0f}),
        Vec<3>({ 0.0f, -1.0f,  0.0f}),

        Vec<3>({ 0.0f,  0.0f, -1.0f}),
        Vec<3>({ 0.0f,  0.0f, -1.0f}),
        Vec<3>({ 0.0f,  0.0f, -1.0f}),
        Vec<3>({ 0.0f,  0.0f, -1.0f}),
    };

    std::vector<unsigned> cubeIndices = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,

        8, 9,10,
        8,10,11,

        12,13,14,
        12,14,15,

        16,17,18,
        16,18,19,

        20,21,22,
        20,22,23,
    };

    Mesh cubeMesh(renderer, cubeVerts.size(), cubeIndices.size());
    cubeMesh.SetVertices(cubeVerts);
    cubeMesh.SetVertexNormals(cubeNormals);
    cubeMesh.SetIndexBuffer(cubeIndices);

    return cubeMesh;
}
Mesh Mesh::Quad(const Renderer& renderer) {
    
    std::vector<Vec<4>> quadVerts = {
        Vec<4>({0.5f,  0.5f, 0.0f, 1}),
        Vec<4>({0.5f,  -0.5f, 0.0f, 1}),
        Vec<4>({-0.5f,  -0.5f, 0.0f, 1}),
        Vec<4>({-0.5f,  0.5f, 0.0f, 1}),
    };

    std::vector<Vec<3>> quadNormals = {
        Vec<3>({0.0f,  0.0f, -1.0f}),
        Vec<3>({0.0f,  0.0f, -1.0f}),
        Vec<3>({0.0f,  0.0f, -1.0f}),
        Vec<3>({0.0f,  0.0f, -1.0f}),
    };

    std::vector<unsigned> quadIndices = {
        0,1,2,
        0,2,3,
    };

    Mesh quadMesh(renderer, quadVerts.size(), quadIndices.size());
    quadMesh.SetVertices(quadVerts);
    quadMesh.SetVertexNormals(quadNormals);
    quadMesh.SetIndexBuffer(quadIndices);

    return quadMesh;
}

uint32_t getMidpointIndex(uint32_t v1, uint32_t v2, std::unordered_map<uint64_t, uint32_t>& midpointCache, std::vector<Vec<4>>& positions, std::vector<Vec<3>>& normals); 

Mesh Mesh::Icosphere(const Renderer& renderer, unsigned subdivisions) {


    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Define initial icosahedron vertices (homogeneous positions)
    std::vector<Vec<3>> initialVertices = {
        Vec<3>({-1,  t,  0}).Normalized(), Vec<3>({ 1,  t,  0}).Normalized(),
        Vec<3>({-1, -t,  0}).Normalized(), Vec<3>({ 1, -t,  0}).Normalized(),
        Vec<3>({ 0, -1,  t}).Normalized(), Vec<3>({ 0,  1,  t}).Normalized(),
        Vec<3>({ 0, -1, -t}).Normalized(), Vec<3>({ 0,  1, -t}).Normalized(),
        Vec<3>({ t,  0, -1}).Normalized(), Vec<3>({ t,  0,  1}).Normalized(),
        Vec<3>({-t,  0, -1}).Normalized(), Vec<3>({-t,  0,  1}).Normalized()
    };

    // Initialize positions (homogeneous) and normals
    std::vector<Vec<4>>positions;
    std::vector<Vec<3>> normals;
    for (const auto& vertex : initialVertices) {
        positions.push_back(Vec<4>({ vertex(0), vertex(1), vertex(2), 1.0f }));
        normals.push_back(vertex);
    }

    // Initial icosahedron triangles
    std::vector<std::array<uint32_t, 3>> triangles = {
        { 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
        { 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
        { 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
        { 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 }
    };

    // Subdivide triangles
    std::unordered_map<uint64_t, uint32_t> midpointCache;
    for (int i = 0; i < subdivisions; ++i) {
        std::vector<std::array<uint32_t, 3>> newTriangles;
        for (const auto& tri : triangles) {
            uint32_t a = getMidpointIndex(tri[0], tri[1], midpointCache, positions, normals);
            uint32_t b = getMidpointIndex(tri[1], tri[2], midpointCache, positions, normals);
            uint32_t c = getMidpointIndex(tri[2], tri[0], midpointCache, positions, normals);

            newTriangles.push_back({ tri[0], a, c });
            newTriangles.push_back({ tri[1], b, a });
            newTriangles.push_back({ tri[2], c, b });
            newTriangles.push_back({ a, b, c });
        }
        triangles = std::move(newTriangles);
    }

    // Flatten triangle indices into a single array
    std::vector<unsigned> indices;
    for (const auto& tri : triangles) {
        indices.insert(indices.end(), { tri[0], tri[1], tri[2] });
    }

	Mesh icosphereMesh(renderer, positions.size(), indices.size());
    icosphereMesh.SetVertices(positions);
    icosphereMesh.SetIndexBuffer(indices);
    icosphereMesh.SetVertexNormals(normals);

	return icosphereMesh;
}


// Function to get a midpoint vertex index
uint32_t getMidpointIndex(uint32_t v1, uint32_t v2,
                          std::unordered_map<uint64_t, uint32_t>& midpointCache,
                          std::vector<Vec<4>>& positions,
                          std::vector<Vec<3>>& normals) {
    uint64_t key = (uint64_t)std::min(v1, v2) << 32 | std::max(v1, v2);
    if (midpointCache.count(key)) {
        return midpointCache[key];
    }

    // Compute the midpoint and normalize it
    const auto& pos1 = positions[v1];
    const auto& pos2 = positions[v2];
    Vec<3> midpoint = Vec<3>({ pos1(0), pos1(1), pos1(2) }) + 
                       Vec<3>({ pos2(0), pos2(1), pos2(2) });
    midpoint *= 0.5f;
    midpoint.Normalize();

    // Add the new vertex position (homogeneous) and normal
    positions.push_back(Vec<4>({ midpoint(0), midpoint(1), midpoint(2), 1.0f }));
    normals.push_back(midpoint);
    uint32_t index = static_cast<uint32_t>(positions.size() - 1);

    midpointCache[key] = index;
    return index;
}

}