#version 450

layout(location = 1) in vec3 v_pos;
layout(location = 2) in float v_depth;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(sqrt(dot(normalize(cross(dFdx(v_pos), dFdy(v_pos))) * 0.5 + vec3(0.5), normalize(vec3(1,1,1))) * vec3(1,0,0)),1);
    outColor = vec4(vec3(1,0,0) * 1/(v_depth/100),1);
}
