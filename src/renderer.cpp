#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/gtx/string_cast.hpp>

#include "renderer.h"
#include "helper.h"
#include "windu.h"


struct Transform {
    glm::mat4 modelviewproj;
};

Renderer::Renderer(Windu *win) {
    this->win = win;
}





void Renderer::render(uint32_t i) {
    sync();

    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &commandBuffers[i];
    std::vector<VkSemaphore> wait = waitSemaphores;
    info.pWaitSemaphores = wait.data();
    info.waitSemaphoreCount = waitCount;
    info.pWaitDstStageMask = waitStages.data();
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores = signalSemaphores.data();
    
    
    
    Transform ubo = {};
    ubo.modelviewproj = win->camera.getViewProj();
    
    FoBuffer* uniform = win->resman.getBuffer(FO_RESOURCE_UNIFORM_BUFFER);
    
    foAssert(win->vkd->vkWaitForFences(win->device.logical, 1, &fence, true, 1000000000000000L));
    
    
    
    void* data;
    foAssert(win->vkd->vkMapMemory(win->device.logical, uniform->memory, uniform->offset, uniform->size, 0, &data));
    memcpy(data, &ubo, uniform->size);
    win->vkd->vkUnmapMemory(win->device.logical, uniform->memory);
    
    
    
    foAssert(win->vkd->vkResetFences(win->device.logical, 1, &fence));
    
    foAssert(win->vkd->vkQueueSubmit(win->device.graphics, 1, &info, fence));

    postsync();
}


void Renderer::preinit() {

    win->resman.allocateResource(FO_RESOURCE_UNIFORM_BUFFER, sizeof(Transform));
    
}


void Renderer::init() {
    // Création de ressources statiques, qui ne dépendent pas de la swapchain, qui ne sont donc pas recréés
    
    // RENDERPASS
    
    initDescriptors();
    
    initPipeline();
    
    initRest();
    
    setup();
    prepare(&win->sync);
}




void Renderer::initDescriptors() {
    
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo dpinfo = {};
    dpinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpinfo.poolSizeCount = 1;
    dpinfo.pPoolSizes = &poolSize;
    dpinfo.maxSets = 1;
    foAssert(win->vkd->vkCreateDescriptorPool(win->device.logical, &dpinfo, VK_NULL_HANDLE, &descriptorPool));




    VkDescriptorSetLayoutBinding slb = {};
    slb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    slb.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    slb.descriptorCount = 1;
    slb.binding = 0;
    
    VkDescriptorSetLayoutCreateInfo slinfo = {};
    slinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    slinfo.bindingCount = 1;
    slinfo.pBindings = &slb;
    
    foAssert(win->vkd->vkCreateDescriptorSetLayout(win->device.logical, &slinfo, VK_NULL_HANDLE, &descriptorSetLayout));
    
    
    
    VkDescriptorSetAllocateInfo allocinfo = {};
    allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocinfo.descriptorSetCount = 1;
    allocinfo.pSetLayouts = &descriptorSetLayout;
    allocinfo.descriptorPool = descriptorPool;
    
    foAssert(win->vkd->vkAllocateDescriptorSets(win->device.logical, &allocinfo, &descriptorSet));
    
    
    FoBuffer* uniform = win->resman.getBuffer(FO_RESOURCE_UNIFORM_BUFFER);
    
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniform->buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = uniform->size;
    
    VkWriteDescriptorSet write = {};
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &bufferInfo;
    write.dstSet = descriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    
    win->vkd->vkUpdateDescriptorSets(win->device.logical, 1, &write, 0, VK_NULL_HANDLE);
}


void Renderer::initPipeline() {
    
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = win->swap.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    foAssert(win->vkd->vkCreateRenderPass(win->device.logical, &renderPassInfo, VK_NULL_HANDLE, &renderPass));
    
    
    // PIPELINE INFO
    
    auto vertShaderCode = foLoad("src/shaders/rectangle.vert.spv");
    auto fragShaderCode = foLoad("src/shaders/rectangle.frag.spv");
    
    VkShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    
    moduleInfo.codeSize = vertShaderCode.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.constData());
    VkShaderModule vertShaderModule;
    foAssert(win->vkd->vkCreateShaderModule(win->device.logical, &moduleInfo, VK_NULL_HANDLE, &vertShaderModule));
    
    moduleInfo.codeSize = fragShaderCode.size();
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.constData());
    VkShaderModule fragShaderModule;
    foAssert(win->vkd->vkCreateShaderModule(win->device.logical, &moduleInfo, VK_NULL_HANDLE, &fragShaderModule));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    
    // VERTEX INPUT
    
    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Inpute attribute bindings describe shader attribute locations and memory layouts
    VkVertexInputAttributeDescription vertexInputAttributs;
    
    vertexInputAttributs.binding = 0;
    vertexInputAttributs.location = 0;
    vertexInputAttributs.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs.offset = offsetof(Vertex, position);

    
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputState.vertexAttributeDescriptionCount = 1;
    vertexInputState.pVertexAttributeDescriptions = &vertexInputAttributs;
    
    

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) win->swap.extent.width;
    viewport.height = (float) win->swap.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = win->swap.extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    
    VkDynamicState states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynInfo = {};
    dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynInfo.dynamicStateCount = 2;
    dynInfo.pDynamicStates = &states[0];
    
    
    
    
    VkPipelineLayoutCreateInfo plinfo = {};
    plinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plinfo.setLayoutCount = 1;
    plinfo.pSetLayouts = &descriptorSetLayout;
    
    foAssert(win->vkd->vkCreatePipelineLayout(win->device.logical, &plinfo, VK_NULL_HANDLE, &pipelineLayout));
    
    
    
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (win->vkd->vkCreateGraphicsPipelines(win->device.logical, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    win->vkd->vkDestroyShaderModule(win->device.logical, fragShaderModule, VK_NULL_HANDLE);
    win->vkd->vkDestroyShaderModule(win->device.logical, vertShaderModule, VK_NULL_HANDLE);
    
}



void Renderer::initRest() {
    
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = win->device.g_i;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    foAssert(win->vkd->vkCreateCommandPool(win->device.logical, &poolInfo, VK_NULL_HANDLE, &commandPool));




    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = win->swap.NUM_FRAMES;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    commandBuffers.resize(win->swap.NUM_FRAMES);
    foAssert(win->vkd->vkAllocateCommandBuffers(win->device.logical, &allocInfo, commandBuffers.data()));



    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    win->vkd->vkCreateFence(win->device.logical, &info, VK_NULL_HANDLE, &fence);
    
}




void Renderer::setup() {
    
    // Création et update ressources qui sont recréés à chaque récréation de la swapchain
    framebuffers.resize(win->swap.NUM_FRAMES);
    
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = win->swap.extent.width;
    framebufferInfo.height = win->swap.extent.height;
    framebufferInfo.layers = 1;
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = win->swap.extent;

    VkClearValue clearColor = {};
    clearColor.color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) win->swap.extent.width;
    viewport.height = (float) win->swap.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = win->swap.extent;
    
    FoBuffer* vertexBuffer = win->resman.getBuffer(FO_RESOURCE_VERTEX_BUFFER);
    
    for (size_t i = 0; i < framebuffers.size(); i++) {

        framebufferInfo.pAttachments = &(win->swap.imageViews[i]);
        foAssert(win->vkd->vkCreateFramebuffer(win->device.logical, &framebufferInfo, VK_NULL_HANDLE, &framebuffers[i]));
        
        
        renderPassInfo.framebuffer = framebuffers[i];
        foAssert(win->vkd->vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        win->vkd->vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        win->vkd->vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        win->vkd->vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, 0);

        win->vkd->vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);
        
        win->vkd->vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);
        
        // Bind triangle vertex buffer (contains position and colors)
        VkDeviceSize offsets[1] = { 0 };
        win->vkd->vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(vertexBuffer->buffer), offsets);
        
        win->vkd->vkCmdDraw(commandBuffers[i], 9003*15*3, 1, 0, 0);

        win->vkd->vkCmdEndRenderPass(commandBuffers[i]);

        foAssert(win->vkd->vkEndCommandBuffer(commandBuffers[i]));
        
    }
    
}

void Renderer::cleanup() {
    // Free/Destroy ce qui est créé dans setup
    for (size_t i = 0; i < framebuffers.size(); i++) {
        win->vkd->vkDestroyFramebuffer(win->device.logical, framebuffers[i], VK_NULL_HANDLE);
        
    }
}

void Renderer::reset() {
    cleanup();
    setup();
}





Renderer::~Renderer() {
    cleanup();
    // Free/Destroy ce qui est créé dans init();
    
    
    win->vkd->vkDestroyFence(win->device.logical, fence, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyCommandPool(win->device.logical, commandPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipeline(win->device.logical, graphicsPipeline, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyPipelineLayout(win->device.logical, pipelineLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorSetLayout(win->device.logical, descriptorSetLayout, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyDescriptorPool(win->device.logical, descriptorPool, VK_NULL_HANDLE);
    
    win->vkd->vkDestroyRenderPass(win->device.logical, renderPass, VK_NULL_HANDLE);
}
