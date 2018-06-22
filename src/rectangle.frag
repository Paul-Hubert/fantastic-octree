#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(binding = 0, rgba8) uniform readonly image2D sourceImage;

layout(location = 1) in vec2 v_position;

void main() {
    outColor = imageLoad(sourceImage, ivec2(v_position * imageSize(sourceImage)));
}
