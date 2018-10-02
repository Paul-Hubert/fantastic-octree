#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
    vec4 gl_Position;
};

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform Transform {
    mat4 modelviewproj;
} ubo;


vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0)
);


void main() {
    gl_Position = ubo.modelviewproj * vec4(inPos, 1.0);
}
