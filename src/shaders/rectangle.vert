#version 450

out gl_PerVertex {
    vec4 gl_Position;
};

layout (location = 0) in vec4 inPos;

layout (binding = 0) uniform Transform {
    mat4 modelviewproj;
} ubo;

void main() {
    gl_Position = ubo.modelviewproj * inPos;
}
