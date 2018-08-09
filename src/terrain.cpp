#include "terrain.h"

#include <iostream>
#include <string>
#include <bitset>

#include "compute.h"

#define UNIFORM 25565

void Terrain::init(Compute* comp) {
    this->comp = comp;
    
    assert(CHAR_BIT == 8 && CHAR_BIT * sizeof (float) == 32);
    
    densities = new float[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    
    for(int x = 0; x < CHUNK_SIZE; x++) {
        for(int y = 0; y < CHUNK_SIZE; y++) {
            for(int z = 0; z < CHUNK_SIZE; z++) {
                densities[(x * CHUNK_SIZE + y) * CHUNK_SIZE + z] = -3.3+1.8+y;//-8 + y + 5.*sin(x/5.) + 5.*cos(x/5.);
                //std::cout << densities[(x * CHUNK_SIZE + y) * CHUNK_SIZE + z] << "\n";
            }
        }
    }
    
    chunks = (Chunk*) this->comp->allocate(OCTREE_SIZE*sizeof(Chunk));
    
    int pos[3] {0,0,0};
    
    createChunk(pos, 0);
    
    delete[] densities;
    
    readChunk(0, 0);
    
    this->comp->upload(0, (offset+1)*sizeof(Chunk));
    
}

void Terrain::step(qint64 dt) {
    
}

int intpow(int x, int n) {
    int wow = 1;
    for(int i = 0; i < n; i++) {
        wow*=x;
    }
    return wow;
}

float Terrain::createChunk(int pos[3], int depth) {
    Chunk node = {};
    int loc = offset;
    
    
    int adv = intpow(2, MAX_DEPTH - depth);
    
    bool uniform = true, last;
    float mean = 0;
    
    for(int i = 0; i < 8; i++) {
        
        // Get new position
        int npos[3] {pos[0], pos[1], pos[2]};
        npos[0] += (i%2==1) * adv; // Move x
        npos[1] += i > 3 ? adv : 0; // Move y
        npos[2] += (i == 2 || i == 3 || i == 6 || i == 7) ? adv : 0; // Move z
        
        if(depth >= MAX_DEPTH) {
            
            float value = densities[(npos[0] * CHUNK_SIZE + npos[1]) * CHUNK_SIZE + npos[2]];
            if(i == 0) last = value >= 0;
            if((value >=0) != last) uniform = false;
            
            mean += value;
            
            node.ptrs[i] = value;
            
        } else {
            
            offset++;
            
            int ptr = offset;
            
            float value = createChunk(npos, depth + 1);
            
            if(value != 0.) {
                
                if(i == 0) last = value >= 0;
                if((value > 0) != last) uniform = false;
                
                mean += value;
                
                node.ptrs[i] = value;
                
                offset = ptr - 1;
                
                node.leaves ^= (0 ^ node.leaves) & (1UL << i);
                
            } else {
                
                uniform = false;
                
                node.ptrs[i] = ptr;
                
                node.leaves ^= (-1 ^ node.leaves) & (1UL << i);
                
            }
            
            
        }
    }
    
    //std::cout << loc << " ---> \n";
    //std::cout << node.leaves << "\n";
    
    chunks[loc] = node;
    return uniform ? mean/8. : 0.;
}

void Terrain::readChunk(int ptr, int depth) {
    
    std::string bar (depth*2, ' ');
    
    Chunk ch = chunks[ptr];
    
    std::bitset<8> x(ch.leaves);
    std::cout << x << "\n";
    
    for(int i = 0; i<8; i++) {
        
        if(depth > MAX_DEPTH) {
            
            std::cout << bar << i << " : " << ch.ptrs[i] << " --| \n";
            
        } else {
            
            if(!((ch.leaves >> i) & 1)) {
                std::cout << bar << i << " : " << ch.ptrs[i] << " --| \n";
            } else {
                std::cout << bar << i << " : " << ch.ptrs[i] << " --> \n";
                readChunk(ch.ptrs[i], depth + 1);
            }
        
        }
    }
}

Terrain::~Terrain() {
    this->comp->deallocate();
}
