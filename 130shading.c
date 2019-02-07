//Harry Tian
#include <stdio.h>

typedef struct shaShading shaShading;
struct shaShading {
    int unifDim, attrDim, texNum, varyDim;
    void (*colorPixel)(int unifDim, const double unif[], int texNum,
                       const texTexture *tex[], int varyDim, const double vary[],
                       double rgbd[4]);
    void (*transformVertex)(int unifDim, const double unif[], int attrDim,
                            const double attr[], int varyDim, double vary[]);
};


