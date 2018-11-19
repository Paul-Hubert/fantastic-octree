#include "premcubes.h"

#include <QVulkanDeviceFunctions>

#include "windu.h"

PreMCubes::PreMCubes(Windu* win) {
    this->win = win;
}

void PreMCubes::preinit() {
    
}


void PreMCubes::init() {
    
    
    {
        // CREATE DESCRIPTORS

        
        // POOL
        
        VkDescriptorPoolSize size = {};
        size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        size.descriptorCount = 4;
        
        VkDescriptorPoolCreateInfo dpinfo = {};
        dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpinfo.poolSizeCount = 1;
        dpinfo.pPoolSizes = &size;
        dpinfo.maxSets = 1;
        foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, VK_NULL_HANDLE, &descriptorPool));


        // LAYOUT

        VkDescriptorSetLayoutBinding slb[4];
        slb[0] = {};
        slb[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        slb[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[0].descriptorCount = 1;
        slb[0].binding = 0;
        
        slb[1] = {};
        slb[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        slb[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[1].descriptorCount = 1;
        slb[1].binding = 1;
        
        slb[2] = {};
        slb[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        slb[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[2].descriptorCount = 1;
        slb[2].binding = 2;
        
        slb[3] = {};
        slb[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        slb[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        slb[3].descriptorCount = 1;
        slb[3].binding = 3;
        
        VkDescriptorSetLayoutCreateInfo slinfo = {};
        slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        slinfo.bindingCount = 4;
        slinfo.pBindings = &slb[0];
        
        foAssert(win->vkd->vkCreateDescriptorSetLayout(win->device.logical, &slinfo, VK_NULL_HANDLE, &descriptorSetLayout));

        
        
        // SET
        
        VkDescriptorSetAllocateInfo allocinfo = {};
        allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocinfo.descriptorSetCount = 1;
        allocinfo.pSetLayouts = &descriptorSetLayout;
        allocinfo.descriptorPool = descriptorPool;
        
        foAssert(win->vkd->vkAllocateDescriptorSets(win->device.logical, &allocinfo, &descriptorSet));

        
        
        
        // WRITE
        
        FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER);
        FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
        FoBuffer* indirectDraw = win->resman.getBuffer(FO_RESOURCE_INDIRECT_DRAW);
        FoBuffer* indirectDispatch = win->resman.getBuffer(FO_RESOURCE_INDIRECT_DISPATCH);
        
        VkDescriptorBufferInfo cubesInfo = {};
        cubesInfo.buffer = cubes->buffer;
        cubesInfo.offset = 0;
        cubesInfo.range = cubes->size;         
        
        VkDescriptorBufferInfo densityInfo = {};
        densityInfo.buffer = density->buffer;
        densityInfo.offset = 0;
        densityInfo.range = density->size;
        
        VkDescriptorBufferInfo drawInfo = {};
        drawInfo.buffer = indirectDraw->buffer;
        drawInfo.offset = 0;
        drawInfo.range = indirectDraw->size;
        
        VkDescriptorBufferInfo dispatchInfo = {};
        dispatchInfo.buffer = indirectDispatch->buffer;
        dispatchInfo.offset = 0;
        dispatchInfo.range = indirectDispatch->size;
        
        std::vector<VkWriteDescriptorSet> write(4);
        write[0] = {};
        write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[0].descriptorCount = 1;
        write[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write[0].pBufferInfo = &cubesInfo;
        write[0].dstSet = descriptorSet;
        write[0].dstBinding = 0;
        write[0].dstArrayElement = 0;
        
        write[1] = {};
        write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[1].descriptorCount = 1;
        write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write[1].pBufferInfo = &densityInfo;
        write[1].dstSet = descriptorSet;
        write[1].dstBinding = 1;
        write[1].dstArrayElement = 0;
        
        write[2] = {};
        write[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[2].descriptorCount = 1;
        write[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write[2].pBufferInfo = &dispatchInfo;
        write[2].dstSet = descriptorSet;
        write[2].dstBinding = 2;
        write[2].dstArrayElement = 0;
        
        write[3] = {};
        write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[3].descriptorCount = 1;
        write[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write[3].pBufferInfo = &drawInfo;
        write[3].dstSet = descriptorSet;
        write[3].dstBinding = 3;
        write[3].dstArrayElement = 0;
        
        win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, VK_NULL_HANDLE);
    }
    
    
    
    {
        // CREATE PIPELINE
        
        
        VkPipelineLayoutCreateInfo plinfo = {};
        plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plinfo.setLayoutCount = 1;
        plinfo.pSetLayouts = &descriptorSetLayout;
        
        foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, VK_NULL_HANDLE, &pipelineLayout));

        VkShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        QByteArray code = foLoad("src/shaders/premcubes.comp.spv");
        moduleInfo.codeSize = code.size();
        moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.constData());
        
        
        VkShaderModule shaderModule;
        foAssert(win->vkd->vkCreateShaderModule(win->device.logical, &moduleInfo, VK_NULL_HANDLE, &shaderModule));

        VkPipelineShaderStageCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderInfo.module = shaderModule;
        shaderInfo.pName = "main";
        shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.stage = shaderInfo;
        
        foAssert(win->vkd->vkCreateComputePipelines(win->device.logical, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline));
        
        win->vkd->vkDestroyShaderModule(win->device.logical, shaderModule, VK_NULL_HANDLE);
    
    }
    
    
}


void PreMCubes::setup() {
    
}

void PreMCubes::cleanup() {
    
}

void PreMCubes::reset() {
    cleanup();
    setup();
}


PreMCubes::~PreMCubes() {
    
    win->vkd->vkDestroyPipeline(win->device.logical, pipeline, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, VK_NULL_HANDLE);
    
}
