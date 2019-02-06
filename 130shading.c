//Harry Tian
#include <stdio.h>

typedef struct shaShading shaShading;

struct shaShading {
    int unifDim, texNum, attrDim, varyDim;
    void (*colorPixel)(int, const double[], int, const texTexture *[], int,
            const double[],double[4]);
    void (*transformVertex)(int, const double[], int,
                         const double[], int, double[]);
};


