


/* On macOS, compile with...
    clang 120main3D.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "120vector.c"
#include "140matrix.c"
#include "140isometry.c"
#include "140camera.c"
#include "040texture.c"
#include "130shading.c"
#include "130depth.c"
#include "130triangle.c"
#include "130mesh.c"

#define mainSCREENSIZE 512

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRZ 2
#define mainATTRS 3
#define mainATTRT 4
#define mainATTRN 5
#define mainATTRO 6
#define mainATTRP 7
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYZ 2
#define mainVARYS 3
#define mainVARYT 4
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFCAMERA 19
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

void colorPixel(int unifDim, const double unif[], int texNum,
                const texTexture *tex[], int varyDim, const double vary[],
                double rgbd[4]) {
    double sample[tex[0]->texelDim];
    texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
    rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
    rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
    rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
    rgbd[3] = -vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
                     const double attr[], int varyDim, double vary[]) {
    double attrHomog[4] = {attr[0], attr[1], attr[2], 1.0};
    double varyHomog[4];
//	vecPrint(attrDim,attr);
//	fflush(stdout);
    mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), attrHomog, varyHomog);
//	vecPrint(varyDim,vary);
//	fflush(stdout);
    mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), varyHomog, vary);
    vary[mainVARYS] = attr[mainATTRS];
    vary[mainVARYT] = attr[mainATTRT];
}

shaShading sha;
texTexture texture;
depthBuffer depth;
isoIsometry iso;
camCamera cam;

const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh mesh1;
meshMesh mesh2;
double unif[3 + 32] = {1.0, 1.0, 1.0,
                       1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0,
                       1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0.0, 0.0, 0.0, 1.0,};
double rotationAngle = 0.0;
double rotationAxis[3];
double translationVector[3] = {256.0, 256.0, 256.0};
double rho = 0;
double phi = 0;
double theta = 0;
double target[3] = {100,100,100};

void draw(void) {
    depthClearDepths(&depth,1000000000000);
    pixClearRGB(0.0, 0.0, 0.0);
    meshRender(&mesh1, &depth, &sha, unif, tex);
    meshRender(&mesh2, &depth, &sha, unif, tex);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown) {
    if (key == GLFW_KEY_ENTER) {
        if (texture.filtering == texLINEAR)
            texSetFiltering(&texture, texNEAREST);
        else
            texSetFiltering(&texture, texLINEAR);
        draw();
    }
    else {
        if (key == GLFW_KEY_W)
            phi-= M_PI / 10;
        if (key == GLFW_KEY_S)
            phi += M_PI / 10;
        if (key == GLFW_KEY_A)
            theta -= M_PI / 10;
        if (key == GLFW_KEY_D)
            theta += M_PI / 10;
        camLookAt(&cam, target, rho, phi, theta);
        double invIsom[4][4];
        isoGetInverseHomogeneous(&cam.isometry,invIsom);
        vecCopy(16, (double *)invIsom, &unif[mainUNIFCAMERA]);
        draw();
    }
}

void handleTimeStep(double oldTime, double newTime) {
    if (floor(newTime) - floor(oldTime) >= 1.0)
        printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
    unif[mainUNIFR] = sin(newTime);
    unif[mainUNIFG] = cos(oldTime);
//	rotationAngle += (newTime - oldTime);
    double rot[3][3], isom[4][4];
    vec3Set(1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0), rotationAxis);
    mat33AngleAxisRotation(rotationAngle, rotationAxis, rot);
    isoSetRotation(&iso,rot);
    isoGetHomogeneous(&iso,isom);
    vecCopy(16, (double *)isom, &unif[mainUNIFMODELING]);

//	phi = rotationAngle;
//	rho -= 1;
//	theta = rho;
    camLookAt(&cam, target, rho, phi, theta);

    double view[4][4], projInvIsom[4][4], viewProjInvIsom[4][4];
    camGetProjectionInverseIsometry(&cam, projInvIsom);
    mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
    mat444Multiply(view, projInvIsom, viewProjInvIsom);
//    double invIsom[4][4];
//    isoGetInverseHomogeneous(&cam.isometry,invIsom);
    vecCopy(16, (double *)viewProjInvIsom, &unif[mainUNIFCAMERA]);
    draw();
}

int main(void) {
    if (pixInitialize(512, 512, "Pixel Graphics") != 0)
        return 1;
    else if (texInitializeFile(&texture, "../Noether_retusche_nachcoloriert.jpg") != 0)
        return 2;
    else if (meshInitializeBox(&mesh1, -128.0, 128.0, -64.0, 64.0, -65.0, -10) != 0)
        //else if (meshInitializeSphere(&mesh, 64.0, 16, 32) != 0)
        return 3;
    else if (meshInitializeSphere(&mesh2, 64.0, 16, 32) != 0)
        return 4;
    else if(depthInitialize(&depth,512,512)!=0)
        return 5;
    else {
        texSetFiltering(&texture, texNEAREST);
        texSetLeftRight(&texture, texREPEAT);
        texSetTopBottom(&texture, texREPEAT);
        sha.unifDim = 3 + 32;
        sha.attrDim = 3 + 2 + 3;
        sha.varyDim = 3 + 2;
        sha.colorPixel = colorPixel;
        sha.transformVertex = transformVertex;
        sha.texNum = 1;
        camSetProjectionType(&cam, camORTHOGRAPHIC);
        camSetFrustum(&cam, M_PI / 6.0, rho, 10.0, mainSCREENSIZE,
                      mainSCREENSIZE);
        camLookAt(&cam, target, rho, phi, theta);
        isoSetTranslation(&iso,translationVector);
        draw();
        pixSetKeyUpHandler(handleKeyUp);
        pixSetTimeStepHandler(handleTimeStep);
        pixRun();
        meshDestroy(&mesh1);
        meshDestroy(&mesh2);
        texDestroy(&texture);
        depthDestroy(&depth);
        return 0;
    }
}


