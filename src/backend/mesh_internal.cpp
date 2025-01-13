#pragma once
#include <iostream>
#include <algorithm>
#include "types_internal.h"
#include "mesh_internal.h"


namespace RenderingFramework3D {


using namespace MathUtil;

Mesh::MeshInternal::MeshInternal(const Renderer::RendererInternal& renderer, const VertDataLayout& layout, unsigned numVerts, unsigned numIndices)
	:
	_num_indices(numIndices),
	_num_verts(numVerts),
	_dev_id(renderer.GetDeviceID()),
	_layout(layout),
	_verts(),
	_normals(),
	_indices(),
	_custom_data(),
	_vertbuffer_res(),
	_idxbuffer_res(),
	_loaded(false),
    _dynamic_load(false)
{}

Mesh::MeshInternal::~MeshInternal() {
    if(_loaded) {
        UnloadMesh();
    }
}

bool Mesh::MeshInternal::SetMeshDataLayout(const VertDataLayout& layout) {
    if(_loaded == false) {
        _layout = layout;
        if(_layout.customVertInputLayouts.size() > 1) std::sort(_layout.customVertInputLayouts.begin(), _layout.customVertInputLayouts.end(), [](const CustomVertInputLayout& lhs, const CustomVertInputLayout& rhs) { return lhs.shaderInputSlot < rhs.shaderInputSlot; });
        return true;
    }
    return false;
}

void Mesh::MeshInternal::SetVertices(const std::vector<Vec<4>>& vertexBuffer) {
	_verts = vertexBuffer;
}

void Mesh::MeshInternal::SetVertexNormals(const std::vector<Vec<3>>& normalBuffer) {
	_normals = normalBuffer;
    for(auto& normal : _normals) {
        normal.Normalize();
    }
}

void Mesh::MeshInternal::SetIndexBuffer(const std::vector<unsigned>& indexBuffer) {
	_indices = indexBuffer;
}

void Mesh::MeshInternal::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<bool>& data) {
	unsigned size = sizeof(data[0])*data.size();
	_custom_data[shaderInputSlot] = std::vector<uint8_t>(size);
	uint8_t* dst = &_custom_data[shaderInputSlot][0];
	for (auto d : data) {
		memcpy(dst, &d, size);
		dst += sizeof(d);
	}
}
void Mesh::MeshInternal::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<uint32_t>& data) {
	unsigned size = sizeof(data[0]) * data.size();
	_custom_data[shaderInputSlot] = std::vector<uint8_t>(size);
	memcpy(_custom_data[shaderInputSlot].data(), &data[0], size);
}
void Mesh::MeshInternal::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<int32_t>& data) {
	unsigned size = sizeof(data[0]) * data.size();
	_custom_data[shaderInputSlot] = std::vector<uint8_t>(size);
	memcpy(_custom_data[shaderInputSlot].data(), &data[0], size);
}
void Mesh::MeshInternal::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<float>& data) {
	unsigned size = sizeof(data[0]) * data.size();
	_custom_data[shaderInputSlot] = std::vector<uint8_t>(size);
	memcpy(_custom_data[shaderInputSlot].data(), &data[0], size);
}
void Mesh::MeshInternal::SetCustomVertexDataBuffer(unsigned shaderInputSlot, const std::vector<double>& data) {
	unsigned size = sizeof(data[0]) * data.size();
	_custom_data[shaderInputSlot] = std::vector<uint8_t>(size);
	memcpy(_custom_data[shaderInputSlot].data(), &data[0], size);
}

unsigned Mesh::MeshInternal::GetNumVertices() const {
	return _num_verts;
}
unsigned Mesh::MeshInternal::GetNumIndices() const {
	return _num_indices;
}

const std::vector<Vec<4>>& Mesh::MeshInternal::GetVertices() const {
	return _verts;
}
const std::vector<Vec<3>>& Mesh::MeshInternal::GetVertexNormals() const {
	return _normals;
}
const std::vector<unsigned>& Mesh::MeshInternal::GetIndexBuffer() const {
	return _indices;
}

const std::vector<uint8_t>& Mesh::MeshInternal::GetCustomVertexData(unsigned shaderInputSlot) const {
	static std::vector<uint8_t> dummy;
    if (_custom_data.find(shaderInputSlot) == _custom_data.end()) {
		return dummy;
	}
	return _custom_data.at(shaderInputSlot);
}

bool Mesh::MeshInternal::LoadMesh(bool dynamic) {
	if(_loaded == true) {
		return false;
	}
	if(_num_verts <= 0 || _num_indices <= 0) {
		return false;
	}

    for (const auto idx : _indices) {
        if (idx >= _num_verts) {
            printf("unable to load mesh, one or more indices in the index buffer out of range\n");
            return false;
        }
    }

    unsigned strideSize = 0;
    unsigned bufferSize = 0;
    VkBuffer stagingBuffer, vertexBuffer, indexBuffer;
    VkDeviceMemory stagingBufferMemory, vertexBufferMemory, indexBufferMemory;
    uint8_t* data;
    
    if (_layout.useVertBuffer) strideSize += sizeof(float)*4;
    if (_layout.useVertNormBuffer) strideSize += sizeof(float)*3;

    for (auto& e : _layout.customVertInputLayouts) {
        unsigned size = getVertDataSize(e.type, e.components);
        strideSize += size;
    }
    VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    VkPhysicalDevice physdev = DeviceManager::GetVkPhyDevice(_dev_id);
    if(physdev == VK_NULL_HANDLE) {
        return false;
    }

    bufferSize = strideSize * _num_verts;
    void* pdata = nullptr;
    if(bufferSize <= 0) {
        return false;
    }
    if(dynamic == false) {
        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory) == false) {
            printf("failed to create vertex staging buffer\n");
            return false;
        }
        if (vkMapMemory(dev, stagingBufferMemory, 0, bufferSize, 0, &pdata) != VK_SUCCESS) {
            return false;
        }
    } else {
        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, vertexBuffer, vertexBufferMemory) == false) {
            printf("failed to create vertex staging buffer\n");
            return false;
        }
        if (vkMapMemory(dev, vertexBufferMemory, 0, bufferSize, 0, &pdata) != VK_SUCCESS) {
            return false;
        }
    }
    
    data = (uint8_t*)pdata;
    for (int i = 0; i < _num_verts; i++) {
        int offset = 0;
        if (_layout.useVertBuffer) {
            if(i < _verts.size()) _verts[i].CopyRaw((float*)(&data[i * strideSize + offset]));
            offset += sizeof(float) * 4;
        }
        if (_layout.useVertNormBuffer) {
            if(i < _normals.size())_normals[i].CopyRaw((float*)(&data[i * strideSize + offset]));
            offset += sizeof(float) * 3;
        }
        for (const auto& layout : _layout.customVertInputLayouts) {
            unsigned size = getVertDataSize(layout.type, layout.components);
            unsigned idx = size * i;
			if(_custom_data.find(layout.shaderInputSlot) == _custom_data.end()) {
				continue;
			}

            if ((idx + size) > _custom_data[layout.shaderInputSlot].size()) {
                continue;
            }

            const void* src = &_custom_data[layout.shaderInputSlot][idx];
            void* dst = &data[i * strideSize + offset];

            memcpy(dst, src, size);
            offset += size;
        }
    }
    if(dynamic == false) {
        vkUnmapMemory(dev, stagingBufferMemory);

        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory) == false) {
            printf("failed to create vertex buffer\n");
            return false;
        }

        if( DeviceManager::CopyBuffer(_dev_id, stagingBuffer, vertexBuffer, bufferSize) == false ) {
            return false;
        }
        
        vkFreeMemory(dev, stagingBufferMemory, nullptr);
        vkDestroyBuffer(dev, stagingBuffer, nullptr);

    } else {
        _vertbuffer_mapped = pdata;
    }

    unsigned numIndices = _num_indices < _indices.size() ? _num_indices : _indices.size();
    bufferSize = numIndices * sizeof(unsigned);
    pdata = nullptr;
    if(bufferSize <= 0) {
        return false;
    }
    if(dynamic == false) {
        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory) == false) {
            printf("failed to create index staging buffer\n");
            return false;
        }
        if(vkMapMemory(dev, stagingBufferMemory, 0, bufferSize, 0, &pdata) != VK_SUCCESS) {
            return false;
        }
    } else {
        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, indexBuffer, indexBufferMemory) == false) {
            printf("failed to create index staging buffer\n");
            return false;
        }
        if(vkMapMemory(dev, indexBufferMemory, 0, bufferSize, 0, &pdata) != VK_SUCCESS) {
            return false;
        }
    }

    data = (uint8_t*)pdata;
    memcpy(data, _indices.data(), (size_t)bufferSize);

    if(dynamic == false) {
        vkUnmapMemory(dev, stagingBufferMemory);

        if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory) == false) {
            printf("failed to create index buffer\n");
            return false;
        }
        if( DeviceManager::CopyBuffer(_dev_id, stagingBuffer, indexBuffer, bufferSize) == false ) {
            return false;
        }
        vkFreeMemory(dev, stagingBufferMemory, nullptr);
        vkDestroyBuffer(dev, stagingBuffer, nullptr);
    } else {
        _indexbuffer_mapped = pdata;
    }

	_vertbuffer_res = {vertexBufferMemory,vertexBuffer};
	_idxbuffer_res = {indexBufferMemory, indexBuffer};

	_loaded = true;
    _dynamic_load = dynamic;

    return true;
}

bool Mesh::MeshInternal::UnloadMesh() {
    if (_loaded == false) {
        return false;
    }
    VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    if(_dynamic_load) {
        vkUnmapMemory(dev, _vertbuffer_res.vkBufferMem);
        _vertbuffer_mapped = nullptr;
    }

    vkDestroyBuffer(dev, _vertbuffer_res.vkBuffer, nullptr);
    vkFreeMemory(dev, _vertbuffer_res.vkBufferMem, nullptr);

    vkDestroyBuffer(dev, _idxbuffer_res.vkBuffer, nullptr);
    vkFreeMemory(dev, _idxbuffer_res.vkBufferMem, nullptr);

	_loaded = false;

    return true;
}

bool Mesh::MeshInternal::Reload(bool dynamic) {
	if(_loaded) {
		if(UnloadMesh() == false) return false;
	}
	return LoadMesh(dynamic);
}

bool Mesh::MeshInternal::SetVertexDynamic(unsigned idx, const MathUtil::Vec<4>& position) {
    if(_loaded == true && _dynamic_load == false) {
        return false;
    }

    if(_layout.useVertBuffer == false) {
        return false;
    }

    if(idx < _verts.size()) {
        _verts[idx] = position;
    }

    if(_loaded == false) {
        return true;
    }

    if(idx >= _num_verts) {
        return false;
    }
    
    unsigned offset = 0;
    unsigned strideSize = 0;
    uint8_t* data = (uint8_t*)_vertbuffer_mapped;

    if (_layout.useVertBuffer) strideSize += sizeof(float)*4;
    if (_layout.useVertNormBuffer) strideSize += sizeof(float)*3;
    for (auto& e : _layout.customVertInputLayouts) {
        unsigned size = getVertDataSize(e.type, e.components);
        strideSize += size;
    }
    position.CopyRaw((float*)(&data[idx * strideSize + offset]));
    return true;
}

bool Mesh::MeshInternal::SetVertexNormalDynamic(unsigned idx, const MathUtil::Vec<3>& normal) {
    if(_loaded == true && _dynamic_load == false) {
        return false;
    }

    if(_layout.useVertNormBuffer == false) {
        return false;
    }

    if(idx < _normals.size()) {
        _normals[idx] = normal;
    }

    if(_loaded == false) {
        return true;

    }

    if(idx >= _num_verts) {
        return false;
    }

    unsigned offset = 0;
    unsigned strideSize = 0;
    uint8_t* data = (uint8_t*)_vertbuffer_mapped;

    if (_layout.useVertBuffer) {
        strideSize += sizeof(float)*4;
        offset += sizeof(float)*4;
    }
    if (_layout.useVertNormBuffer) strideSize += sizeof(float)*3;
    for (auto& e : _layout.customVertInputLayouts) {
        unsigned size = getVertDataSize(e.type, e.components);
        strideSize += size;
    }
    normal.CopyRaw((float*)(&data[idx * strideSize + offset]));
    return true;
}
bool Mesh::MeshInternal::SetIndexDynamic(unsigned idx, unsigned vertIndex) {
    if(_loaded == true && _dynamic_load == false) {
        return false;
    }
    if(idx < _num_indices) {
        _indices[idx] = vertIndex;
    }
    if(_loaded == false) {
        return true;
    }
    if(idx >= _num_indices) {
        return false;
    }
    unsigned* data = (unsigned*)_indexbuffer_mapped;
    memcpy(&data[idx], &vertIndex, sizeof(vertIndex));
    return true;
}


bool Mesh::MeshInternal::SetCustomVertexDataDynamic(unsigned vertIndex, unsigned shaderInputSlot, uint8_t* data, unsigned maxSize) {
    if(_loaded == true && _dynamic_load == false) {
        return false;
    }

    unsigned strideSize = 0;
    unsigned offset = 0;
    if(_layout.useVertBuffer) {
        offset += sizeof(float)*4;
        strideSize += sizeof(float)*4;
    }
    
    if(_layout.useVertNormBuffer) {
        offset += sizeof(float)*3;
        strideSize += sizeof(float)*3;
    }

    for(const auto& e : _layout.customVertInputLayouts) {
        unsigned sizefromlayout = getVertDataSize(e.type, e.components);
        strideSize += sizefromlayout;
    }

    unsigned layoutidx = 0;
    for(const auto& e : _layout.customVertInputLayouts) {
        unsigned sizefromlayout = getVertDataSize(e.type, e.components);
        if(e.shaderInputSlot == shaderInputSlot) {
            unsigned size = maxSize < sizefromlayout ? maxSize : sizefromlayout;
            uint8_t* datadst = (uint8_t*)_vertbuffer_mapped;

            if (_custom_data.find(shaderInputSlot) != _custom_data.end() && _custom_data[shaderInputSlot].size() > vertIndex * sizefromlayout) {
                memcpy(&_custom_data[shaderInputSlot][vertIndex*sizefromlayout], data, size);
            }
            
            if(_loaded == false) {
                return true;
            }
            
            if(vertIndex < _num_verts) {
                memcpy(&datadst[vertIndex * strideSize + offset], data, size);
            }
            
            return true;
        }
        offset += sizefromlayout; 
        layoutidx++;
    }
    return false;
}


bool Mesh::MeshInternal::AddCommandDrawMesh(VkCommandBuffer cmdBuffer, unsigned maxIndices) {
    VkBuffer vertexBuffers[] = { _vertbuffer_res.vkBuffer };
    VkDeviceSize offsets[] = { 0 };

    unsigned indices = _num_indices;

    if (maxIndices > 0) {
        indices = _num_indices > maxIndices ? maxIndices : _num_indices;
    }

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers,offsets);
    vkCmdBindIndexBuffer(cmdBuffer, _idxbuffer_res.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuffer, indices, 1, 0, 0, 0);
    return true;
}

}