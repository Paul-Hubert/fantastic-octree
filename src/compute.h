#ifndef COMPUTE_H
#define COMPUTE_H

#include <QVulkanInstance>

class Windu;

class Compute {
public :
    Compute(Windu *win);
    ~Compute();
    void init();
    
    Windu *win;
    
};

#endif
