#version 450

out gl_PerVertex {
    vec4 gl_Position;
};
layout (location = 1) out vec3 v_pos;
layout (location = 2) out float v_depth;

layout (location = 0) in vec4 inPos;

layout (binding = 0) uniform Transform {
    mat4 modelviewproj;
} ubo;

void main() {
    v_pos = inPos.xyz;
    gl_Position = ubo.modelviewproj * inPos;
    v_depth = gl_Position.z;
}
