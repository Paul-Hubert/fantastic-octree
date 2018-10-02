#include "terrain.h"

#include <iostream>
#include <string>
#include <bitset>

#include "compute.h"

#define UNIFORM 25565

void Terrain::init(Compute* comp) {
    this->comp = comp;
    
    assert(CHAR_BIT == 8 && CHAR_BIT * sizeof (float) == 32);
    
    densities = new Value[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    
    for(int x = 0; x < CHUNK_SIZE; x++) {
        for(int y = 0; y < CHUNK_SIZE; y++) {
            for(int z = 0; z < CHUNK_SIZE; z++) {
                densities[(x * CHUNK_SIZE + y) * CHUNK_SIZE + z] = {static_cast<float>(-12.f + y + 5.f*sin(x/5.f) + 5.f*cos(z/5.f))};
            }
        }
    }
    
    void* ptr = comp->startWriteDensity();
    
    memcpy(ptr, densities, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Value));
    
    comp->finishWriteDensity();
    
    comp->finishWriteCubes(createCubes(static_cast<Cube*>(comp->startWriteCubes())));
    
}

void Terrain::step(qint64 dt) {
    dt++;
}

int intpow(int x, int n) {
    int wow = 1;
    for(int i = 0; i < n; i++) {
        wow*=x;
    }
    return wow;
}

int Terrain::createCubes(Cube* cubes) {
    
    int offset = 0;
    
    for(int x = 0; x < CHUNK_SIZE-1; x++) {
        for(int y = 0; y < CHUNK_SIZE-1; y++) {
            for(int z = 0; z < CHUNK_SIZE-1; z++) {
                int sum = (densities[((x  ) * CHUNK_SIZE + y  ) * CHUNK_SIZE + z  ].density>0 ? 1:0)
                        | (densities[((x+1) * CHUNK_SIZE + y  ) * CHUNK_SIZE + z  ].density>0 ? 2:0)
                        | (densities[((x+1) * CHUNK_SIZE + y  ) * CHUNK_SIZE + z+1].density>0 ? 4:0)
                        | (densities[((x  ) * CHUNK_SIZE + y  ) * CHUNK_SIZE + z+1].density>0 ? 8:0)
                        | (densities[((x  ) * CHUNK_SIZE + y+1) * CHUNK_SIZE + z  ].density>0 ? 16:0)
                        | (densities[((x+1) * CHUNK_SIZE + y+1) * CHUNK_SIZE + z  ].density>0 ? 32:0)
                        | (densities[((x+1) * CHUNK_SIZE + y+1) * CHUNK_SIZE + z+1].density>0 ? 64:0)
                        | (densities[((x  ) * CHUNK_SIZE + y+1) * CHUNK_SIZE + z+1].density>0 ? 128:0);
                if(sum != 255 && sum != 0) {
                    cubes[offset] = {glm::vec4(x,y,z,sum)};
                    offset++;
                    if(offset >= MAX_CUBES) {
                        return MAX_CUBES;
                    }
                }
            }
        }
    }
    
    return offset;
}

Terrain::~Terrain() {
    delete[] densities;
}
