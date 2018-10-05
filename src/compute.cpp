#include <QVulkanDeviceFunctions>
#include <iostream>
#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "compute.h"
#include "windu.h"
#include "terrain.h"
#include "helper.h"

Compute::Compute(Windu *win) {
    this->win = win;
}

void Compute::preinit() {
    
    win->resman.allocateResource(FO_RESOURCE_CUBES_BUFFER, MAX_CUBES * sizeof(Cube));
    
    win->resman.allocateResource(FO_RESOURCE_STAGING_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_DENSITY_BUFFER, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(Value));
    
    win->resman.allocateResource(FO_RESOURCE_VERTEX_BUFFER, MAX_CUBES * 15 * sizeof(Vertex));
    
    win->resman.allocateResource(FO_RESOURCE_LOOKUP_BUFFER, 4096*sizeof(char));
    
}


void Compute::init() {
    
    {
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        VkBufferViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        info.offset = 0;
        info.range = lookup->size*sizeof(char);
        info.buffer = lookup->buffer;
        info.format = VK_FORMAT_R8_SINT;
        
        
        foAssert(win->vkd->vkCreateBufferView(win->device.logical, &info, VK_NULL_HANDLE, &lookupView));
    }
    
    initDescriptors();
    
    initPipeline();
    
    initRest();
    
    setup();
    
    prepare(&win->sync);
}

void Compute::setup() {
    
    
    
}









bool Compute::isActive() {
    return true;
}


void Compute::render(uint32_t i) {
    sync();

    t++;
    
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &commandBuffer;
    std::vector<VkSemaphore> wait(waitSemaphores);
    info.waitSemaphoreCount = waitCount;
    info.pWaitSemaphores = wait.data();
    info.pWaitDstStageMask = waitStages.data();
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores = signalSemaphores.data();
    
    win->vkd->vkWaitForFences(win->device.logical, 1, &fence, true, 1000000000000000L);
    
    
    win->vkd->vkResetFences(win->device.logical, 1, &fence);
    
    foAssert(win->vkd->vkQueueSubmit(win->device.compute, 1, &info, fence));

    postsync();
    
}






void* Compute::startWriteDensity() {
    
    FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER);
    
    void* ptr;
    foAssert(win->vkd->vkMapMemory(win->device.logical, density->memory, density->offset, density->size, 0, &ptr));
    
    return ptr;
}

void Compute::finishWriteDensity() {
    
    win->vkd->vkUnmapMemory(win->device.logical, win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER)->memory);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    foAssert(win->vkd->vkBeginCommandBuffer(transferCmd, &beginInfo));
    
    VkBufferCopy copy = {};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER)->size;
    
    win->vkd->vkCmdCopyBuffer(transferCmd, win->resman.getBuffer(FO_RESOURCE_STAGING_DENSITY_BUFFER)->buffer, win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER)->buffer, 1, &copy);
    
    foAssert(win->vkd->vkEndCommandBuffer(transferCmd));
    
    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &transferCmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &transferSem;
    
    this->prepareSignal(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferSem);
    
    win->vkd->vkQueueSubmit(win->device.transfer, 1, &submit, VK_NULL_HANDLE);
    
}

void * Compute::startWriteCubes() {
    
    FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
    
    void* ptr;
    foAssert(win->vkd->vkMapMemory(win->device.logical, cubes->memory, cubes->offset, cubes->size, 0, &ptr));
    
    return ptr;
    
}

void Compute::finishWriteCubes(int num) {
    
    std::cout << num << std::endl;
    
    win->vkd->vkUnmapMemory(win->device.logical, win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER)->memory);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    
    foAssert(win->vkd->vkBeginCommandBuffer(commandBuffer, &beginInfo));
    
    win->vkd->vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    win->vkd->vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, 0);
    
    win->vkd->vkCmdDispatch(commandBuffer, num, 1, 1); // TO CHANGE
    
    foAssert(win->vkd->vkEndCommandBuffer(commandBuffer));
    
}






void Compute::initDescriptors() {

    VkDescriptorPoolSize size[2] = {{},{}};
    size[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    size[0].descriptorCount = 3;
    size[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    size[1].descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo dpinfo = {};
    dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpinfo.poolSizeCount = 2;
    dpinfo.pPoolSizes = &size[0];
    dpinfo.maxSets = 1;
    foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, VK_NULL_HANDLE, &descriptorPool));




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
    slb[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    slb[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    slb[3].descriptorCount = 1;
    slb[3].binding = 3;
    
    VkDescriptorSetLayoutCreateInfo slinfo = {};
    slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    slinfo.bindingCount = 4;
    slinfo.pBindings = &slb[0];
    
    foAssert(win->vkd->vkCreateDescriptorSetLayout(win->device.logical, &slinfo, VK_NULL_HANDLE, &descriptorSetLayout));

    VkDescriptorSetAllocateInfo allocinfo = {};
    allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocinfo.descriptorSetCount = 1;
    allocinfo.pSetLayouts = &descriptorSetLayout;
    allocinfo.descriptorPool = descriptorPool;
    
    foAssert(win->vkd->vkAllocateDescriptorSets(win->device.logical, &allocinfo, &descriptorSet));

    FoBuffer* density = win->resman.getBuffer(FO_RESOURCE_DENSITY_BUFFER);
    FoBuffer* vertex = win->resman.getBuffer(FO_RESOURCE_VERTEX_BUFFER);
    FoBuffer* cubes = win->resman.getBuffer(FO_RESOURCE_CUBES_BUFFER);
    
    VkDescriptorBufferInfo cubesInfo = {};
    cubesInfo.buffer = cubes->buffer;
    cubesInfo.offset = 0;
    cubesInfo.range = cubes->size;         
    
    VkDescriptorBufferInfo densityInfo = {};
    densityInfo.buffer = density->buffer;
    densityInfo.offset = 0;
    densityInfo.range = density->size;
    
    VkDescriptorBufferInfo vertexInfo = {};
    vertexInfo.buffer = vertex->buffer;
    vertexInfo.offset = 0;
    vertexInfo.range = vertex->size;
    
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
    write[2].pBufferInfo = &vertexInfo;
    write[2].dstSet = descriptorSet;
    write[2].dstBinding = 2;
    write[2].dstArrayElement = 0;
    
    write[3] = {};
    write[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[3].descriptorCount = 1;
    write[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    write[3].pTexelBufferView = &lookupView;
    write[3].dstSet = descriptorSet;
    write[3].dstBinding = 3;
    write[3].dstArrayElement = 0;
    
    win->vkd->vkUpdateDescriptorSets(win->device.logical, write.size(), write.data(), 0, VK_NULL_HANDLE);
    
}








void Compute::initPipeline() {
    
    VkPipelineLayoutCreateInfo plinfo = {};
    plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plinfo.setLayoutCount = 1;
    plinfo.pSetLayouts = &descriptorSetLayout;
    
    foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, VK_NULL_HANDLE, &pipelineLayout));

    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    QByteArray code = foLoad("src/shaders/marchingcubes.comp.spv");
    moduleInfo.codeSize = code.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(code.constData());
    
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







void Compute::initRest() {
    
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = win->device.c_i;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, VK_NULL_HANDLE, &commandPool));

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, &commandBuffer));

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        foAssert(win->vkd->vkCreateFence(win->device.logical, &fenceInfo, VK_NULL_HANDLE, &fence));
    }
    
    
    {
        VkCommandPoolCreateInfo transferInfo = {};
        transferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transferInfo.queueFamilyIndex = win->device.t_i;
        transferInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &transferInfo, VK_NULL_HANDLE, &transferPool));
        
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = transferPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, &transferCmd));

        VkSemaphoreCreateInfo semInfo = {};
        semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;        
        foAssert(win->vkd->vkCreateSemaphore(win->device.logical, &semInfo, VK_NULL_HANDLE, &transferSem));
    }
    
    {
        
        FoBuffer* lookup = win->resman.getBuffer(FO_RESOURCE_LOOKUP_BUFFER);
        void* ptr;
        QByteArray bytes = foLoad("src/resources/mclookup.bin");
        foAssert(win->vkd->vkMapMemory(win->device.logical, lookup->memory, lookup->offset, lookup->size, 0, &ptr));
        memcpy(ptr, bytes.constData(), lookup->size);
        win->vkd->vkUnmapMemory(win->device.logical, lookup->memory);
        
    }
    
}





void Compute::cleanup() {
    
}

void Compute::reset() {
    cleanup();
    setup();
}


Compute::Compute::~Compute() {
    
    cleanup();
    
    win->vkd->vkDestroyBufferView(win->device.logical, lookupView, VK_NULL_HANDLE);
    
    win->vkd->vkDestroySemaphore(win->device.logical, transferSem, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, transferPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyFence(win->device.logical, fence, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipeline(win->device.logical, pipeline, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, VK_NULL_HANDLE);
    
}
