#version 450

#extension GL_KHR_shader_subgroup_arithmetic: enable
#extension GL_KHR_shader_subgroup_ballot: enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

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

const ivec3 off[8] = {
    ivec3(0,0,0),
    ivec3(0,0,1),
    ivec3(0,1,0),
    ivec3(0,1,1),
    ivec3(1,0,0),
    ivec3(1,0,1),
    ivec3(1,1,0),
    ivec3(1,1,1)
};

const int CHUNK_SIZE = 64;
const int NUM_SAMPLES = 200;

layout (std140, binding = 4) uniform Transform {
    mat4 viewinvproj;
    vec3 pos;
} ubo;

layout(std430, binding = 1) buffer Values {
    float values[];
};

layout(std430, binding = 0) writeonly buffer Tris {
    vec4 tril[];
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
    
    
    vec2 size = vec2(5*192, 5*108);
    if(gl_GlobalInvocationID.x >= size.x || gl_GlobalInvocationID.y >= size.y) return;

    vec3 ray = normalize((ubo.viewinvproj * vec4(gl_GlobalInvocationID.x / size.x * -2 + 1, gl_GlobalInvocationID.y / size.y * -2 + 1,  1., 1.)).xyz);

    vec3 position = -ubo.pos;
    
    /*
    {
        vec3 tmin = (vec3(0)          - position)/ray;
        vec3 tmax = (vec3(CHUNK_SIZE) - position)/ray;
        
        float mint = max(max( min(tmin.x, tmax.x), min(tmin.y, tmax.y)), min(tmin.z, tmax.z) );
        float maxt = min(min( max(tmin.x, tmax.x), max(tmin.y, tmax.y)), max(tmin.z, tmax.z) );
        
        if(maxt < mint || maxt < 0.) {
            //return;
        }
        position += ray*max(mint, 0.);
    }
    */
    
    for(int i = 0; i < NUM_SAMPLES; i++) {
        position += ray*0.1;
        //if(position.x < 0 || position.x > CHUNK_SIZE || position.y < 0 || position.y > CHUNK_SIZE || position.z < 0 || position.z > CHUNK_SIZE) break;
        if(-12. + position.y + 5.*sin(position.x/5.) + 5.*cos(position.z/5.) > 0) break;
    }
    
    
    uint local_tri_index = subgroupExclusiveAdd(3);
        
    uint highestActiveID = subgroupBallotFindMSB(subgroupBallot(true));

    uint global_tri_index = 0;

    if (highestActiveID == gl_SubgroupInvocationID) {
        global_tri_index = atomicAdd(vertexCount, local_tri_index + 3);
    }

    global_tri_index = subgroupMax(global_tri_index);
    
    tril[global_tri_index + local_tri_index] = vec4(position, 1);
    tril[global_tri_index + local_tri_index+1] = vec4(position + vec3(1,0,0)*0.01, 1);
    tril[global_tri_index + local_tri_index+2] = vec4(position + vec3(0,0,1)*0.01, 1);
    
    return;
    
    
    for(int n = 0; n<8; n++) {
    
        ivec3 pos = ivec3(position + vec3(0.5)) - off[n];
        
        if(min(pos.x, min(pos.y, pos.z)) < 0) continue;
    
        uint val =    (values[((pos.x  ) * CHUNK_SIZE + pos.y  ) * CHUNK_SIZE + pos.z  ]>0 ?   1:0)
                    | (values[((pos.x+1) * CHUNK_SIZE + pos.y  ) * CHUNK_SIZE + pos.z  ]>0 ?   2:0)
                    | (values[((pos.x+1) * CHUNK_SIZE + pos.y  ) * CHUNK_SIZE + pos.z+1]>0 ?   4:0)
                    | (values[((pos.x  ) * CHUNK_SIZE + pos.y  ) * CHUNK_SIZE + pos.z+1]>0 ?   8:0)
                    | (values[((pos.x  ) * CHUNK_SIZE + pos.y+1) * CHUNK_SIZE + pos.z  ]>0 ?  16:0)
                    | (values[((pos.x+1) * CHUNK_SIZE + pos.y+1) * CHUNK_SIZE + pos.z  ]>0 ?  32:0)
                    | (values[((pos.x+1) * CHUNK_SIZE + pos.y+1) * CHUNK_SIZE + pos.z+1]>0 ?  64:0)
                    | (values[((pos.x  ) * CHUNK_SIZE + pos.y+1) * CHUNK_SIZE + pos.z+1]>0 ? 128:0);
                
        uint num = trinum[val];
        if(num > 0 && !(pos.x >= CHUNK_SIZE-1 || pos.y >= CHUNK_SIZE-1 || pos.z >= CHUNK_SIZE-1)) {
        
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
                vec3 a1 = pos.xyz + offsets[edge], a2 = pos.xyz + offsets[edge+1]; 
                float density1 = values[int(a1.x * CHUNK_SIZE*CHUNK_SIZE + a1.y * CHUNK_SIZE + a1.z)];
                vec3 vertex = edge >= 0 ? 5.*mix(a1, a2, (-density1)/(values[int(a2.x * CHUNK_SIZE*CHUNK_SIZE + a2.y * CHUNK_SIZE + a2.z)] - density1)) : vec3(0);
                tril[global_tri_index + local_tri_index + i] = vec4(vertex, 1.0);
            }
            
        }
        
    }
    
}
