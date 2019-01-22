#include "raymarcher.h"

#include <QVulkanDeviceFunctions>
#include <vector>

#include "windu.h"
#include "terrain.h"

Raymarcher::Raymarcher(Windu* win) {
    this->win = win;
}

void Raymarcher::record(vk::CommandBuffer commandBuffer) {
    
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout, 0, {descriptorSet}, {});
    
    commandBuffer.dispatch(5*192/8+1, 5*108/8+1, 1);
    
}


void Raymarcher::presubmit() {
    
    FoBuffer* ubo = win->resman.getBuffer(FO_RESOURCE_RAY_UBO);
    
    Ubo tf = {};
    tf.viewinvproj = win->camera.getRotation() * glm::inverse(win->camera.getProj());
    tf.pos = win->camera.getPos();

    void* data = win->device.logical.mapMemory(ubo->memory, ubo->offset, ubo->size);
    memcpy(data, &tf, sizeof(tf));
    win->device.logical.unmapMemory(ubo->memory);
    
}



void Raymarcher::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_RAY_UBO, sizeof(Ubo));
    
}

void Raymarcher::init() {
    
    
    {
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        lookupView = win->device.logical.createBufferView(vk::BufferViewCreateInfo({}, lookup->buffer, vk::Format::eR8Sint, 0, lookup->size*sizeof(char)));
    }
    
    {
        // CREATE DESCRIPTORS
        
        
        // POOL
        std::vector<vk::DescriptorPoolSize> size = {{vk::DescriptorType::eStorageBuffer, 3}, {vk::DescriptorType::eUniformTexelBuffer, 1}, {vk::DescriptorType::eUniformBuffer, 1}};
        descriptorPool = win->device.logical.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, size.size(), size.data()));
        
        // LAYOUT
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
            {0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute},
            {1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute},
            {2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute},
            {3, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute},
            {4, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute}
        };
        descriptorSetLayout = win->device.logical.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings.size(), bindings.data()));

        // SET
        descriptorSet = win->device.logical.allocateDescriptorSets({descriptorPool, 1, &descriptorSetLayout})[0];
        
        
        // WRITE
        FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER);
        FoBuffer* indirectDraw = win->resman.getBuffer(FO_RESOURCE_INDIRECT_DRAW);
        FoBuffer* vertex = win->resman.getBuffer(FO_RESOURCE_VERTEX_BUFFER);
        FoBuffer* ubo = win->resman.getBuffer(FO_RESOURCE_RAY_UBO);
        
        vk::DescriptorBufferInfo densityInfo(density->buffer, 0, density->size);
        vk::DescriptorBufferInfo drawInfo(indirectDraw->buffer, 0, indirectDraw->size);
        vk::DescriptorBufferInfo vertexInfo(vertex->buffer, 0, vertex->size);
        vk::DescriptorBufferInfo uboInfo(ubo->buffer, 0, ubo->size);
        
        win->device.logical.updateDescriptorSets({
            vk::WriteDescriptorSet(descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer, 0, &vertexInfo, 0),
            vk::WriteDescriptorSet(descriptorSet, 1, 0, 1, vk::DescriptorType::eStorageBuffer, 0, &densityInfo, 0),
            vk::WriteDescriptorSet(descriptorSet, 2, 0, 1, vk::DescriptorType::eStorageBuffer, 0, &drawInfo, 0),
            vk::WriteDescriptorSet(descriptorSet, 3, 0, 1, vk::DescriptorType::eUniformTexelBuffer, 0, 0, &lookupView),
            vk::WriteDescriptorSet(descriptorSet, 4, 0, 1, vk::DescriptorType::eUniformBuffer, 0, &uboInfo, 0)
        }, {});
    }
    
    
    
    {
        // CREATE PIPELINE
        pipelineLayout = win->device.logical.createPipelineLayout(vk::PipelineLayoutCreateInfo({}, 1, &descriptorSetLayout));
        
        QByteArray code = foLoad("src/shaders/raymarcher.comp.glsl.spv");
        VkShaderModule shaderModule = win->device.logical.createShaderModule(vk::ShaderModuleCreateInfo({}, code.size(), reinterpret_cast<const uint32_t*>(code.constData())));
        
        pipeline = win->device.logical.createComputePipeline({}, vk::ComputePipelineCreateInfo({}, {{}, vk::ShaderStageFlagBits::eCompute, shaderModule, "main"}, pipelineLayout));
        
        win->device.logical.destroy(shaderModule);
        
    }
    
    
}


void Raymarcher::setup() {
    
}

void Raymarcher::cleanup() {
    
}

void Raymarcher::reset() {
    cleanup();
    setup();
}


Raymarcher::~Raymarcher() {
    
    win->device.logical.destroy(lookupView);
    
    win->device.logical.destroy(pipeline);
    
    win->device.logical.destroy(pipelineLayout);
    
    win->device.logical.destroy(descriptorSetLayout);
    
    win->device.logical.destroy(descriptorPool);
    
}
