#ifndef TERRAIN_H
#define TERRAIN_H

#include <qglobal.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define CHUNK_SIZE 16
#define MAX_DEPTH 3

#define OCTREE_SIZE 2048

class Compute;

struct alignas(16) Chunk {
    float ptrs[8];
    uint32_t leaves;
};

class Terrain {
public:
    void init(Compute* comp);
    void step(qint64 dt);
    ~Terrain();
    
private:
    float createChunk(int pos[3], int depth);
    void readChunk(int ptr, int depth);
    
    float* densities;
    Chunk* chunks;
    int offset = 0;
    
    Compute* comp;
    
};
#endif
