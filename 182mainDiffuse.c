//Shiyue Zhang
#include <math.h>


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "170engine.h"
/*#include "120vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "130shading.c"
#include "130depth.c"
#include "130triangle.c"
#include "160mesh.c"
#include "140isometry.c"
#include "150camera.c"
#include "140landscape.c"*/
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
#define mainVARYW 3
#define mainVARYS 4
#define mainVARYT 5
#define mainVARYN 6
#define mainVARYO 7
#define mainVARYP 8
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFCAMERA 19
#define mainUNIFDIRX 35
#define mainUNIFDIRY 36
#define mainUNIFDIRZ 37
#define mainUNIFLIGHTR 38
#define mainUNIFLIGHTG 39
#define mainUNIFLIGHTB 40
/*#define mainUNIFMIN 4
#define mainUNIFMEAN 5
#define mainUNIFMAX 6*/

#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum,
                const texTexture *tex[], int varyDim, const double vary[],
                double rgbd[4]) {
    if(tex==NULL){
        rgbd[0] = unif[mainUNIFR];
        rgbd[1] = unif[mainUNIFG];
        rgbd[2] = unif[mainUNIFB];
        rgbd[3] = vary[mainVARYZ];
    }else {
        //When there is texture
        double sample[tex[0]->texelDim];
        double w = 1/vary[mainVARYW];
        double s = vary[mainVARYS] * w;
        double t = vary[mainVARYT] * w;
        texSample(tex[0], s, t, sample);
        rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
        rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
        rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
        rgbd[3] = vary[mainVARYZ];
    }
    //Modify the color according to Lambertian Diffuse
    double nop[3] = {vary[mainVARYN], vary[mainVARYO],vary[mainVARYP]}, unitnop[3];
    vecUnit(3, nop, unitnop);
    double light[3] = {unif[mainUNIFDIRX], unif[mainUNIFDIRY], unif[mainUNIFDIRZ]};
    double idiff = vecDot(3, light, unitnop);
    if(idiff < 0) {
        idiff = 0;}
    rgbd[0] = rgbd[0] * idiff * unif[mainUNIFLIGHTR];
    rgbd[1] = rgbd[1] * idiff * unif[mainUNIFLIGHTG];
    rgbd[2] = rgbd[2] * idiff * unif[mainUNIFLIGHTB];

}

void transformVertex(int unifDim, const double unif[], int attrDim,
                     const double attr[], int varyDim, double vary[]) {
    double attrHom[4] = {attr[0], attr[1], attr[2], 1.0};
    double worldHom[4], varyHom[4];
    vecCopy(4, attrHom, worldHom);
    worldHom[2] += unif[mainUNIFMODELING];
    mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
    vecCopy(4, varyHom, vary);
    vary[mainVARYS] = attr[mainATTRS];
    vary[mainVARYT] = attr[mainATTRT];
    //Insert NOP values into vary
    double nop[4] = {attr[5],attr[6],attr[7], 0};
    double temp[4];
    mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), nop, temp);
    vary[mainVARYN] = temp[0];
    vary[mainVARYO] = temp[1];
    vary[mainVARYP] = temp[2];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
camCamera cam;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 256.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Meshes to be rendered. */
meshMesh grass;
double unifGrass[3 + 16 + 16 + 3 + 3] = {
        0.0, 1.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0,1.0,1.0};
meshMesh rock;
double unifRock[3 + 16 + 16+ 3 + 3] = {
        1.0, 1.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0,1.0,1.0};
meshMesh water;
double unifWater[3 + 16 + 16 + 3 + 3] = {
        0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0,1.0,1.0};
/*** User interface ***/

void render(void) {
    double view[4][4], projInvIsom[4][4];
    camGetProjectionInverseIsometry(&cam, projInvIsom);
    mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
    pixClearRGB(0.0, 0.0, 0.0);
    depthClearDepths(&buf, 1000000000.0);
    vecCopy(16, (double *)projInvIsom, &unifGrass[mainUNIFCAMERA]);
    meshRender(&grass, &buf, view, &sha, unifGrass, tex);
    vecCopy(16, (double *)projInvIsom, &unifRock[mainUNIFCAMERA]);
    meshRender(&rock, &buf, view, &sha, unifRock, NULL);
    vecCopy(16, (double *)projInvIsom, &unifWater[mainUNIFCAMERA]);
    meshRender(&water, &buf, view, &sha, unifWater, NULL);
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
    else if (key == GLFW_KEY_J)
        unifWater[mainUNIFMODELING] -= 0.1;
    else if (key == GLFW_KEY_U)
        unifWater[mainUNIFMODELING] += 0.1;
    camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
                  mainSCREENSIZE);
    camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown){
    if (key == GLFW_KEY_ENTER) {
        if (cam.projectionType == camPERSPECTIVE){
            camSetProjectionType(&cam, camORTHOGRAPHIC);
        }
        else {
            camSetProjectionType(&cam, camPERSPECTIVE);
        }
    }
    camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
                  mainSCREENSIZE);
}
void handleTimeStep(double oldTime, double newTime) {
    if (floor(newTime) - floor(oldTime) >= 1.0)
        printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
    render();
}

int main(void) {
    /* Design landscape and water. */
    int landNum = 100;
    double landData[landNum][landNum];
    double landMin, landMean, landMax;
    time_t t;
    int i;
    srand((unsigned)time(&t));
    landFlat(landNum, landNum, (double *)landData, 0.0);
    for (i = 0; i < 32; i += 1)
        landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
    for (i = 0; i < 4; i += 1)
        landBlur(landNum, landNum, (double *)landData);
    landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean,
                   &landMax);
    double waterData[4] = {landMin, landMin, landMin, landMin};
    meshMesh land;
    /* Begin configuring scene. */
    if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
        return 1;
    else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
        return 2;
    else if (texInitializeFile(&texture, "../26710550-idyllic-seamless-grass-texture.jpg") != 0)
        return 3;
    else if (meshInitializeLandscape(&land, landNum, landNum, 1.0,
                                     (double *)landData) != 0)
        return 4;
    else if (meshInitializeDissectedLandscape(&grass, &land, M_PI / 4.0,
                                              1) != 0)
        return 5;
    else if (meshInitializeDissectedLandscape(&rock, &land, M_PI / 4.0,
                                              0) != 0)
        return 6;
    else if (meshInitializeLandscape(&water, 2, 2, landNum - 1.0,
                                     (double *)waterData) != 0)
        return 7;
    else {
        meshDestroy(&land);
        /* Continue configuring scene. */
        sha.unifDim = 3 + 16 + 16 + 3 + 3;
        sha.attrDim = 3 + 2 + 3;
        sha.varyDim = 2 + 2 + 2 + 3;
        sha.colorPixel = colorPixel;
        sha.transformVertex = transformVertex;
        sha.texNum = 1;
        texSetFiltering(&texture, texNEAREST);
        texSetLeftRight(&texture, texREPEAT);
        texSetTopBottom(&texture, texREPEAT);
        camSetProjectionType(&cam, camORTHOGRAPHIC);
        camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
                      mainSCREENSIZE);
        vec3Set(landNum / 2.0, landNum / 2.0,
                landData[landNum / 2][landNum / 2], cameraTarget);
        camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
        //change the direction of the light into unit vectors
        double dirg[3] = {unifGrass[mainUNIFDIRX], unifGrass[mainUNIFDIRY],unifGrass[mainUNIFDIRZ]}, dg[3];
        vecUnit(3, dirg, dg);
        unifGrass[mainUNIFDIRX] = dg[0];
        unifGrass[mainUNIFDIRY] = dg[1];
        unifGrass[mainUNIFDIRZ] = dg[2];
        double dirr[3] = {unifRock[mainUNIFDIRX], unifRock[mainUNIFDIRY],unifRock[mainUNIFDIRZ]}, dr[3];
        vecUnit(3, dirr, dr);
        unifRock[mainUNIFDIRX] = dr[0];
        unifRock[mainUNIFDIRY] = dr[1];
        unifRock[mainUNIFDIRZ] = dr[2];
        double dirw[3] = {unifWater[mainUNIFDIRX], unifWater[mainUNIFDIRY],unifWater[mainUNIFDIRZ]}, dw[3];
        vecUnit(3, dirw, dw);
        unifWater[mainUNIFDIRX] = dw[0];
        unifWater[mainUNIFDIRY] = dw[1];
        unifWater[mainUNIFDIRZ] = dw[2];
        /* User interface. */
        pixSetKeyDownHandler(handleKeyAny);
        pixSetKeyRepeatHandler(handleKeyAny);
        pixSetKeyUpHandler(handleKeyUp);
        pixSetTimeStepHandler(handleTimeStep);
        pixRun();
        /* Clean up. */
        meshDestroy(&grass);
        meshDestroy(&rock);
        meshDestroy(&water);
        depthDestroy(&buf);
        return 0;
    }
}
