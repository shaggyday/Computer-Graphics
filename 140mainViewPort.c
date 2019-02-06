


/* On macOS, compile with...
    clang 140mainViewport.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "120vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "130shading.c"
#include "130depth.c"
#include "130triangle.c"
#include "160mesh.c"
#include "140isometry.c"
#include "150camera.c"

#define mainSCREENSIZE 512

/*** Shaders ***/

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
    rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
                     const double attr[], int varyDim, double vary[]) {
    double attrHom[4] = {attr[0], attr[1], attr[2], 1.0}, worldHom[4];
    mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), attrHom, worldHom);
    mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, vary);
    vary[mainVARYS] = attr[mainATTRS];
    vary[mainVARYT] = attr[mainATTRT];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
camCamera cam;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Meshes to be rendered. */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh box;
double unifBox[3 + 16 + 16] = {
        0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0};
meshMesh sph;
double unifSph[3 + 16 + 16 + 16] = {
        1.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0};
/* Animation of sphere mesh. */
double angleSph = 0.0;
double axisSph[3] = {1.0 / M_SQRT2, 0.0, 1.0 / M_SQRT2};

/*** User interface ***/

void render(void) {
    double view[4][4], projInvIsom[4][4], viewProjInvIsom[4][4];
    camGetProjectionInverseIsometry(&cam, projInvIsom);
    mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
    mat444Multiply(view, projInvIsom, viewProjInvIsom);
//    printf("view\n");
//    mat44Print(view);
    pixClearRGB(0.0, 0.0, 0.0);
    depthClearDepths(&buf, 1000000000.0);
    vecCopy(16, (double *)viewProjInvIsom, &unifBox[mainUNIFCAMERA]);
    meshRender(&box, &buf, &sha, unifBox, tex);
    vecCopy(16, (double *)viewProjInvIsom, &unifSph[mainUNIFCAMERA]);
    meshRender(&sph, &buf, &sha, unifSph, tex);
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
                  int altOptionIsDown, int superCommandIsDown) {
    if (key == GLFW_KEY_A)
        cameraTheta -= M_PI / 100;
    else if (key == GLFW_KEY_D)
        cameraTheta += M_PI / 100;
    else if (key == GLFW_KEY_W)
        cameraPhi -= M_PI / 100;
    else if (key == GLFW_KEY_S)
        cameraPhi += M_PI / 100;
    else if (key == GLFW_KEY_Q)
        cameraRho *= 0.9;
    else if (key == GLFW_KEY_E)
        cameraRho *= 1.1;
    else if (key == GLFW_KEY_K)
        cameraTarget[0] -= 0.5;
    else if (key == GLFW_KEY_SEMICOLON)
        cameraTarget[0] += 0.5;
    else if (key == GLFW_KEY_L)
        cameraTarget[1] -= 0.5;
    else if (key == GLFW_KEY_O)
        cameraTarget[1] += 0.5;
    else if (key == GLFW_KEY_I)
        cameraTarget[2] -= 0.5;
    else if (key == GLFW_KEY_P)
        cameraTarget[2] += 0.5;
    camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
                  mainSCREENSIZE);
    camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleTimeStep(double oldTime, double newTime) {
    /* Animate the sphere. */
    double rot[3][3], transl[3] = {0.0, 0.0, 0.0}, isom[4][4];
    angleSph += (newTime - oldTime);
    mat33AngleAxisRotation(angleSph, axisSph, rot);
    mat44Isometry(rot, transl, isom);
    vecCopy(16, (double *)isom, &unifSph[mainUNIFMODELING]);
    /* Do the usual. */
    if (floor(newTime) - floor(oldTime) >= 1.0)
        printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
    render();
}

int main(void) {
    /* Begin configuring scene. */
    if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
        return 1;
    else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
        return 5;
    else if (texInitializeFile(&texture, "../Noether_retusche_nachcoloriert.jpg") != 0)
        return 2;
    else if (meshInitializeBox(&box, -4.0, 4.0, -2.0, 2.0, -2.0, -1.0) != 0)
        return 3;
    else if (meshInitializeSphere(&sph, 2.0, 16, 32) != 0)
        return 4;
    else {
        /* Continue configuring scene. */
        sha.unifDim = 3 + 16 + 16;
        sha.attrDim = 3 + 2 + 3;
        sha.varyDim = 3 + 2;
        sha.colorPixel = colorPixel;
        sha.transformVertex = transformVertex;
        sha.texNum = 1;
        texSetFiltering(&texture, texNEAREST);
        texSetLeftRight(&texture, texREPEAT);
        texSetTopBottom(&texture, texREPEAT);
        camSetProjectionType(&cam, camORTHOGRAPHIC);
        camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
                      mainSCREENSIZE);
        camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
        /* User interface. */
        pixSetKeyDownHandler(handleKeyAny);
        pixSetKeyRepeatHandler(handleKeyAny);
        pixSetKeyUpHandler(handleKeyAny);
        pixSetTimeStepHandler(handleTimeStep);
        pixRun();
        /* Clean up. */
        meshDestroy(&box);
        meshDestroy(&sph);
        texDestroy(&texture);
        depthDestroy(&buf);
        return 0;
    }
}

