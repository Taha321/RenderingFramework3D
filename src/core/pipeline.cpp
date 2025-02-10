#include <algorithm>
#include "pipeline.h"
#include "default_shaders.h"


namespace RenderingFramework3D {

using namespace MathUtil;

Pipeline::Pipeline() 
    :
    _init(false),
    _pipelinelayout(VK_NULL_HANDLE),
    _graphics_pipeline(VK_NULL_HANDLE),
    _uniform_shader_input_layout(),
    _ubo_allocator(),
    _dev_id(0)
{}

Pipeline::~Pipeline() {}

bool Pipeline::Initialize(unsigned devID, const PipelineConfig& config, VkRenderPass renderPass) {
    _dev_id = devID;
    _uniform_shader_input_layout = { config.uniformShaderInputLayout, VK_NULL_HANDLE };
	
    bool ret = createPipeline(config, renderPass);
    ret = ret && _ubo_allocator.Initialize(devID, _uniform_shader_input_layout);
    _init = ret;
    return ret;
}

bool Pipeline::Cleanup() {
    if (_init) {
        _init = false;
        VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
        if (dev == VK_NULL_HANDLE) {
            return false;
        }
        if (_ubo_allocator.Cleanup() == false) {
            return false;
        }

        vkDestroyPipeline(dev, _graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(dev, _pipelinelayout, nullptr);
        vkDestroyDescriptorSetLayout(dev, _uniform_shader_input_layout.vklayoutobject, nullptr);
        vkDestroyDescriptorSetLayout(dev, _uniform_shader_input_layout.vklayoutglobal, nullptr);
        return true;
    }

    return false;
}

bool Pipeline::IsReady() {
    return _init;
}

bool Pipeline::SetLightDir(const Vec<3>& lightDir) {
    if (_uniform_shader_input_layout.layout.GlobalInputs.useDirectionalLight) {
        unsigned size;
        void* dst = _ubo_allocator.GetGlobalUniformBuffer(GLOB_UB_TYPE_DIRLIGHT, size);
        float* dst_f = (float*)dst;
        dst_f += 4;

        if (dst == nullptr) {
            return false;
        }

        lightDir.Normalized().CopyRaw(dst_f);
    }
    return true;
}
bool Pipeline::SetLightColour(const MathUtil::Vec<4>& lightColour) {
    if (_uniform_shader_input_layout.layout.GlobalInputs.useDirectionalLight) {
        unsigned size;
        void* dst = _ubo_allocator.GetGlobalUniformBuffer(GLOB_UB_TYPE_DIRLIGHT, size);
        float* dst_f = (float*)dst;

        if (dst == nullptr) {
            return false;
        }

        lightColour.CopyRaw(dst_f);
    }
    return true;
}

bool Pipeline::SetLightIntensity(float intensity) {
    if (_uniform_shader_input_layout.layout.GlobalInputs.useDirectionalLight) {
        unsigned size;
        void* dst = _ubo_allocator.GetGlobalUniformBuffer(GLOB_UB_TYPE_DIRLIGHT, size);
        float* dst_f = (float*)dst;
        dst_f += 7;

        if (dst == nullptr) {
            return false;
        }

        memcpy(dst_f, &intensity, sizeof(float));
    }
    return true;
}

bool Pipeline::SetAmbientLightIntensity(float intensity) {
    if (_uniform_shader_input_layout.layout.GlobalInputs.useDirectionalLight) {
        unsigned size;
        void* dst = _ubo_allocator.GetGlobalUniformBuffer(GLOB_UB_TYPE_DIRLIGHT, size);
        float* dst_f = (float*)dst;
        dst_f += 8;

        if (dst == nullptr) {
            return false;
        }

        memcpy(dst_f, &intensity, sizeof(float));
    }
    return true;
}

bool Pipeline::SetCustomGlobalData(unsigned binding, void* data, unsigned size, unsigned offset) {
    unsigned retsize;
    void* dst = _ubo_allocator.GetGlobalUniformBuffer(GLOB_UB_TYPE_CUSTOM, retsize, binding);
    if (dst && (size+offset <= retsize)) {
        memcpy(dst, data, size);
        return true;
    }
    return false;
}

const UniformShaderInputLayout& Pipeline::GetUniformBufferSetLayout() const {
    return _uniform_shader_input_layout.layout;
}

bool Pipeline::AddCommandBindPipeline(VkCommandBuffer cmdBuffer) {
    if (_init) {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);
        return true;
    }
    return false;
}

bool Pipeline::AddCommandBindUniformBufferSet(VkCommandBuffer cmdBuffer, const WorldObject& obj, Camera& cam) {
    if (_init == false) {
        return false;
    }

    unsigned size;
    unsigned ubo_id = 0;
    void* dst = nullptr;
    float* dst_f;

    if (_ubo_allocator.AllocateObjectUniformBufferSet(ubo_id) == false) {
        return false;
    }
    
    if (_uniform_shader_input_layout.layout.ObjectInputs.useObjToWorldTransform ||
        _uniform_shader_input_layout.layout.ObjectInputs.useWorldToCamTransform ||
        _uniform_shader_input_layout.layout.ObjectInputs.useCamToScreenTransform||
        _uniform_shader_input_layout.layout.ObjectInputs.useObjToScreenTransform) {

        dst = _ubo_allocator.GetObjectUniformBuffer(ubo_id, OBJ_UB_TYPE_TRANSFORM, size);
        dst_f = (float*)dst;

        if (dst == nullptr) {
            return false;
        }

        Matrix<4,4> transfrom = obj.GetTransform();
        // object to world transform
        if (_uniform_shader_input_layout.layout.ObjectInputs.useObjToWorldTransform) {
            transfrom.CopyRaw(dst_f);
            dst_f += 16;
        }
        // world to camera transform
        if (_uniform_shader_input_layout.layout.ObjectInputs.useWorldToCamTransform) {
            cam.GetWorldToCameraTransform().CopyRaw(dst_f);
            dst_f += 16;
        }
        //camera to screen transform
        if (_uniform_shader_input_layout.layout.ObjectInputs.useCamToScreenTransform) {
            cam.GetCamToScreenTransform().CopyRaw(dst_f);
            dst_f += 16;
        }

        //cam.GetCamToScreenTransform().Print();
        //object to screen tranform
        Matrix<4,4> o_to_s = cam.GetCamToScreenTransform() * cam.GetWorldToCameraTransform() * transfrom;
        if (_uniform_shader_input_layout.layout.ObjectInputs.useObjToScreenTransform) {
            o_to_s.CopyRaw(dst_f);
            dst_f += 16;
        }
        //object scales
        if (_uniform_shader_input_layout.layout.ObjectInputs.useObjectScale) {
            obj.GetObjectScale().CopyRaw(dst_f);
        }
    }

    if (_uniform_shader_input_layout.layout.ObjectInputs.useMaterialData) {
        dst_f = (float*)_ubo_allocator.GetObjectUniformBuffer(ubo_id, OBJ_UB_TYPE_MATERIAL, size);
        obj.GetMaterial().colour.CopyRaw(dst_f);
        dst_f += obj.GetMaterial().colour.Size();

        memcpy(dst_f, &obj.GetMaterial().diffuseConstant, sizeof(float));
        dst_f++;

        memcpy(dst_f, &obj.GetMaterial().specularConstant, sizeof(float));
        dst_f++;

        memcpy(dst_f, &obj.GetMaterial().shininess, sizeof(float));
        dst_f++;
    }

    if (_uniform_shader_input_layout.layout.ObjectInputs.useCamTransform) {
        dst = _ubo_allocator.GetObjectUniformBuffer(ubo_id, OBJ_UB_TYPE_CAM, size);
        cam.GetTransform().CopyRaw((float*)dst);
    }
    
    for (const auto& input : _uniform_shader_input_layout.layout.ObjectInputs.CustomUniformShaderInput) {
        dst = _ubo_allocator.GetObjectUniformBuffer(ubo_id, OBJ_UB_TYPE_CUSTOM, size, input.bindSlot);
        if (dst && obj.GetCustomData(input.bindSlot).size() >= input.size) {
            memcpy(dst, obj.GetCustomData(input.bindSlot).data(), input.size);
        }
    }

    
    if (_ubo_allocator.AddCommandBindUniformBufferSet(ubo_id, _pipelinelayout, cmdBuffer) == false) {
        return false;
    }

    return true;
}

bool Pipeline::EndRenderPass() {
    if (_init == false) {
        return false;
    }
    _ubo_allocator.FreeAllObjectUniformBufferSet();
    return true;
}

bool Pipeline::createPipeline(const PipelineConfig& config, VkRenderPass renderPass) {
    VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    std::vector<CustomVertInputLayout> customIpSorted = config.vertDataLayout.customVertInputLayouts;
    std::sort(customIpSorted.begin(), customIpSorted.end(), [](const CustomVertInputLayout& lhs, const CustomVertInputLayout& rhs) {return lhs.shaderInputSlot < rhs.shaderInputSlot;});

    VkShaderModule vertMod = VK_NULL_HANDLE;
    VkShaderModule fragMod = VK_NULL_HANDLE;

    if (config.useDefaultShaders) {
        switch(config.defFragShaderSelect) {
            case DEFAULT_FRAG_SHADER_LIT:
                fragMod = createShaderModule(dev, litFragShaderBin);
                break;
            case DEFAULT_FRAG_SHADER_UNLIT:
                fragMod = createShaderModule(dev, unlitFragShaderBin);
                break;
            default:
                return false;
        }
        switch(config.defVertShaderSelect) {
            case DEFAULT_VERT_SHADER_LIT:
                vertMod = createShaderModule(dev, litVertShaderBin);
                break;
            case DEFAULT_VERT_SHADER_UNLIT:
                vertMod = createShaderModule(dev, unlitVertShaderBin);
                break;
            default:
                return false;
        }
    } else {
        std::vector<uint8_t> vertCode = readFile(config.customVertexShaderPath);
        std::vector<uint8_t> fragCode = readFile(config.customFragmentShaderPath);

        vertMod = createShaderModule(dev, vertCode);
        fragMod = createShaderModule(dev, fragCode);
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertMod;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragMod;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    createVertexInputInfo(config, customIpSorted, bindingDescription, attributeDescriptions);

    //vertex input layout
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    //descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindingList;
    createObjectUniformBufferBindList(uboLayoutBindingList);
    if (createDescriptorSetLayout(dev, uboLayoutBindingList, _uniform_shader_input_layout.vklayoutobject) == false) {
        return false;
    }

    uboLayoutBindingList.clear();
    createGlobalUniformBufferBindList(uboLayoutBindingList);
    if (createDescriptorSetLayout(dev, uboLayoutBindingList, _uniform_shader_input_layout.vklayoutglobal) == false) {
        return false;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    switch(config.primitiveType) {
        case PRIM_TYPE_TRIANGLE_FILLED:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PRIM_TYPE_TRIANGLE_WIREFRAME:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case PRIM_TYPE_LINE_LINKED:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break; 
        case PRIM_TYPE_LINE_UNLINKED:
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
    }
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_CULL_MODE
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    switch(config.primitiveType) {
        case PRIM_TYPE_TRIANGLE_FILLED:
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        case PRIM_TYPE_TRIANGLE_WIREFRAME:
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            break;
        case PRIM_TYPE_LINE_LINKED:
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            break; 
        case PRIM_TYPE_LINE_UNLINKED:
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            break;
    }
    rasterizer.lineWidth = 1.0f;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = config.alphaBlendEnable ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    std::array<VkDescriptorSetLayout, 2> descsets = { _uniform_shader_input_layout.vklayoutobject , _uniform_shader_input_layout.vklayoutglobal };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descsets.size();
    pipelineLayoutInfo.pSetLayouts = descsets.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &_pipelinelayout) != VK_SUCCESS) {
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelinelayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphics_pipeline) != VK_SUCCESS) {
        return false;
    }

    vkDestroyShaderModule(dev, vertMod, nullptr);
    vkDestroyShaderModule(dev, fragMod, nullptr);

    return true;
}


void Pipeline::createVertexInputInfo(const PipelineConfig& config, const std::vector<CustomVertInputLayout>& customSorted, VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) {
    unsigned offset = 0;
    bool useVerts = config.vertDataLayout.useVertBuffer || config.useDefaultVertData;
    bool useNorm = config.vertDataLayout.useVertNormBuffer || config.useDefaultVertData;
    unsigned maxLocation = 0;
    unsigned vertSlot = config.useDefaultVertData ? 0 : config.vertDataLayout.vertInputSlot;
    unsigned normalSlot = config.useDefaultVertData ? 1 : config.vertDataLayout.useVertNormBuffer;
    unsigned numInputs = (unsigned)useVerts + (unsigned)useNorm + config.vertDataLayout.customVertInputLayouts.size();
    

    bindingDescription.binding = 0;
    bindingDescription.stride = 0;
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    if (useVerts) {
        bindingDescription.stride += sizeof(float) * 4;
        if (vertSlot > maxLocation) {
            maxLocation = config.vertDataLayout.vertInputSlot;
        }
    }
    if (useNorm) {
        bindingDescription.stride += sizeof(float) * 3;
        if (normalSlot > maxLocation) {
            maxLocation = config.vertDataLayout.vertNormInputSlot;
        }
    }
    if (config.useDefaultVertData == false) {
        for (const auto& data : customSorted) {
            bindingDescription.stride += getVertDataSize(data.type, data.components);
            if (data.shaderInputSlot > maxLocation) {
                maxLocation = data.shaderInputSlot;
            }
        }
    }

    attributeDescriptions.resize(numInputs);
    
    unsigned idx = 0;    
    if (useVerts) {
        VkFormat format = getVkFormat(GLSL_FLOAT, 4);
        unsigned size = getVertDataSize(GLSL_FLOAT, 4);

        attributeDescriptions[idx].binding = 0;
        attributeDescriptions[idx].location = vertSlot;
        attributeDescriptions[idx].format = format;
        attributeDescriptions[idx].offset = offset;
        offset += size;
        idx++;
    }
    if (useNorm) {
        VkFormat format = getVkFormat(GLSL_FLOAT, 3);
        unsigned size = getVertDataSize(GLSL_FLOAT, 3);

        //printf("colour format %d, size %d\n", format, size);
        attributeDescriptions[idx].binding = 0;
        attributeDescriptions[idx].location = normalSlot;
        attributeDescriptions[idx].format = format;
        attributeDescriptions[idx].offset = offset;
        offset += size;
        idx++;
    }

    if (config.useDefaultVertData == false) {
        for (int i = 0; i < customSorted.size(); i++) {
            VkFormat format = getVkFormat(customSorted[i].type, customSorted[i].components);
            unsigned size = getVertDataSize(customSorted[i].type, customSorted[i].components);

            printf("custom %d format %d, size %d, offset 0x%0x\n", i, format, size, offset);
            attributeDescriptions[idx].binding = 0;
            attributeDescriptions[idx].location = config.vertDataLayout.customVertInputLayouts[i].shaderInputSlot;
            attributeDescriptions[idx].format = format;
            attributeDescriptions[idx].offset = offset;
            offset += size;
            idx++;
        }
    }
}

void Pipeline::createObjectUniformBufferBindList(std::vector<VkDescriptorSetLayoutBinding>& uboLayoutBindingList) {

    if (_uniform_shader_input_layout.layout.ObjectInputs.useObjToScreenTransform ||
        _uniform_shader_input_layout.layout.ObjectInputs.useObjToWorldTransform ||
        _uniform_shader_input_layout.layout.ObjectInputs.useWorldToCamTransform ||
        _uniform_shader_input_layout.layout.ObjectInputs.useCamToScreenTransform ) {

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = _uniform_shader_input_layout.layout.ObjectInputs.transformBindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        if(_uniform_shader_input_layout.layout.ObjectInputs.transformVertInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if(_uniform_shader_input_layout.layout.ObjectInputs.transformFragInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }
    if (_uniform_shader_input_layout.layout.ObjectInputs.useMaterialData) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = _uniform_shader_input_layout.layout.ObjectInputs.materialDataBindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        if(_uniform_shader_input_layout.layout.ObjectInputs.materialVertInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if(_uniform_shader_input_layout.layout.ObjectInputs.materialFragInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }
    if (_uniform_shader_input_layout.layout.ObjectInputs.useCamTransform) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = _uniform_shader_input_layout.layout.ObjectInputs.camTransformBindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        if(_uniform_shader_input_layout.layout.ObjectInputs.camTransformVertInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if(_uniform_shader_input_layout.layout.ObjectInputs.camTransformFragInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }

    for (auto& desc : _uniform_shader_input_layout.layout.ObjectInputs.CustomUniformShaderInput) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = desc.bindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = 0;
        if (desc.vertInput) uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (desc.fragInput) uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }
}

void Pipeline::createGlobalUniformBufferBindList(std::vector<VkDescriptorSetLayoutBinding>& uboLayoutBindingList) {
    if (_uniform_shader_input_layout.layout.GlobalInputs.useDirectionalLight) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = _uniform_shader_input_layout.layout.GlobalInputs.dirLightBindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        if(_uniform_shader_input_layout.layout.GlobalInputs.dirlightVertInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if(_uniform_shader_input_layout.layout.GlobalInputs.dirlightFragInput)uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }
    for (auto& desc : _uniform_shader_input_layout.layout.GlobalInputs.CustomUniformShaderInput) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = desc.bindSlot;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = 0;
        if (desc.vertInput) uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        if (desc.fragInput) uboLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        uboLayoutBindingList.push_back(uboLayoutBinding);
    }
}


bool Pipeline::createDescriptorSetLayout(VkDevice dev, const std::vector <VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout& vkdesclayout) {
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(dev, &layoutInfo, nullptr, &vkdesclayout) != VK_SUCCESS) {
        return false;
    }
    return true;
}
}