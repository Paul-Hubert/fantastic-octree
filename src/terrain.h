#ifndef TERRAIN_H
#define TERRAIN_H

#include <qglobal.h>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define CHUNK_SIZE 64

#define MAX_CUBES 64*64*2

class Compute;

struct Value {
    float density;
};

struct Cube {
    glm::vec4 pos;
};

class Terrain {
public:
    void init(Compute* comp);
    void step(qint64 dt);
    ~Terrain();
    
private:
    
    Value* densities;
    
    Compute* comp;
    
};
#endif
