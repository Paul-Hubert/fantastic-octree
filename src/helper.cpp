#include "helper.h"

void vkAssert(VkResult res) {
    if(res != VK_SUCCESS) {
        printf("u ded bro");
        assert(res == VK_SUCCESS);
    }
}
