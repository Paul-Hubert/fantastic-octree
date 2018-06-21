#include "renderer.h"
#include "helper.h"
#include "windu.h"

Renderer::Renderer(Windu *win) {
    this->win = win;
}

void Renderer::init() {
    // Création de ressources statiques, qui ne dépendent pas de la swapchain, qui ne sont donc pas recréé
    setup();
    prepare(&win->sync);
}

void Renderer::setup() {
    // Création et update ressources qui sont recréés à chaque récréation de la swapchain
}

void Renderer::cleanup() {
    // Free/Destroy ce qui est créé dans setup
}

void Renderer::reset() {
    cleanup();
    setup();
}

void Renderer::render(uint32_t) {
    sync();
}

Renderer::~Renderer() {
    cleanup();
    // Free/Destroy ce qui est créé dans init();
}

VkShaderModule Renderer::createShaderFromFile(QString fileName) {
    QByteArray bytes = foLoad(fileName);
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bytes.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bytes.constData());
    VkShaderModule shaderModule;
    if(win->vkd->vkCreateShaderModule(win->device.logical, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return shaderModule;
    }
    return nullptr;
}
