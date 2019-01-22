#version 450

#extension GL_KHR_shader_subgroup_arithmetic: enable
#extension GL_KHR_shader_subgroup_ballot: enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

const vec3 offsets[24] = {
    // 0
    vec3(0,0,0),
    vec3(1,0,0),
    // 1
    vec3(1,0,0),
    vec3(1,0,1),
    // 2
    vec3(1,0,1),
    vec3(0,0,1),
    // 3
    vec3(0,0,1),
    vec3(0,0,0),
    // 4
    vec3(0,1,0),
    vec3(1,1,0),
    // 5
    vec3(1,1,0),
    vec3(1,1,1),
    // 6
    vec3(1,1,1),
    vec3(0,1,1),
    // 7
    vec3(0,1,1),
    vec3(0,1,0),
    // 8
    vec3(0,0,0),
    vec3(0,1,0),
    // 9
    vec3(1,0,0),
    vec3(1,1,0),
    // 10
    vec3(1,0,1),
    vec3(1,1,1),
    // 11
    vec3(0,0,1),
    vec3(0,1,1)
};

const uint trinum[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0};

const int CHUNK_SIZE = 64;

layout(std430, binding = 0) writeonly buffer Tris {
    vec4 tril[];
};

layout(std430, binding = 1) buffer Values {
    float values[];
};

layout(std430, binding = 2) coherent buffer IndirectDraw {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

layout(binding = 3) uniform isamplerBuffer triTable;

void main() {
    
    if(gl_GlobalInvocationID.x == 0 && gl_GlobalInvocationID.y == 0 && gl_GlobalInvocationID.z == 0) {
        vertexCount = 0;
        instanceCount = 1;
        firstVertex = 0;
        firstInstance = 0;
    }
    barrier();
    
    uint val =    (values[((gl_GlobalInvocationID.x  ) * CHUNK_SIZE + gl_GlobalInvocationID.y  ) * CHUNK_SIZE + gl_GlobalInvocationID.z  ]>0 ? 1:0)
                | (values[((gl_GlobalInvocationID.x+1) * CHUNK_SIZE + gl_GlobalInvocationID.y  ) * CHUNK_SIZE + gl_GlobalInvocationID.z  ]>0 ? 2:0)
                | (values[((gl_GlobalInvocationID.x+1) * CHUNK_SIZE + gl_GlobalInvocationID.y  ) * CHUNK_SIZE + gl_GlobalInvocationID.z+1]>0 ? 4:0)
                | (values[((gl_GlobalInvocationID.x  ) * CHUNK_SIZE + gl_GlobalInvocationID.y  ) * CHUNK_SIZE + gl_GlobalInvocationID.z+1]>0 ? 8:0)
                | (values[((gl_GlobalInvocationID.x  ) * CHUNK_SIZE + gl_GlobalInvocationID.y+1) * CHUNK_SIZE + gl_GlobalInvocationID.z  ]>0 ? 16:0)
                | (values[((gl_GlobalInvocationID.x+1) * CHUNK_SIZE + gl_GlobalInvocationID.y+1) * CHUNK_SIZE + gl_GlobalInvocationID.z  ]>0 ? 32:0)
                | (values[((gl_GlobalInvocationID.x+1) * CHUNK_SIZE + gl_GlobalInvocationID.y+1) * CHUNK_SIZE + gl_GlobalInvocationID.z+1]>0 ? 64:0)
                | (values[((gl_GlobalInvocationID.x  ) * CHUNK_SIZE + gl_GlobalInvocationID.y+1) * CHUNK_SIZE + gl_GlobalInvocationID.z+1]>0 ? 128:0);
            
    uint num = trinum[val];
    if(num > 0 && !(gl_GlobalInvocationID.x >= CHUNK_SIZE-1 || gl_GlobalInvocationID.y >= CHUNK_SIZE-1 || gl_GlobalInvocationID.z >= CHUNK_SIZE-1)) {
    
        uint local_tri_index = subgroupExclusiveAdd(num*3);
        
        // Find out which active invocation has the highest ID
        uint highestActiveID = subgroupBallotFindMSB(subgroupBallot(true));

        uint global_tri_index = 0;

        // If we're the highest active ID
        if (highestActiveID == gl_SubgroupInvocationID) {
            // We need to carve out a slice of out_triangles for our triangle
            global_tri_index = atomicAdd(vertexCount, local_tri_index + num*3);
        }

        global_tri_index = subgroupMax(global_tri_index);
        
        for(int i = 0; i<num*3; i++) {
            int edge = texelFetch(triTable, int(val) * 16 + i).r * 2;
            vec3 a1 = gl_GlobalInvocationID.xyz + offsets[edge], a2 = gl_GlobalInvocationID.xyz + offsets[edge+1];
            float density1 = values[int(a1.x * CHUNK_SIZE*CHUNK_SIZE + a1.y * CHUNK_SIZE + a1.z)];
            vec3 vertex = edge >= 0 ? 5.*mix(a1, a2, (-density1)/(values[int(a2.x * CHUNK_SIZE*CHUNK_SIZE + a2.y * CHUNK_SIZE + a2.z)] - density1)) : vec3(0);
            tril[global_tri_index + local_tri_index+i] = vec4(vertex, 1.0);
        }
        
    }
    
}
