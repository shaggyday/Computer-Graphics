
/* On macOS, compile with...
    clang 150mainPerspective.c 000pixel.o -lglfw -framework OpenGL
*/
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "170engine.h"

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
#define mainVARYWORLDZ 4
#define mainVARYS 5
#define mainVARYT 6
#define mainVARYN 7
#define mainVARYO 8
#define mainVARYP 9
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFMIN 4
#define mainUNIFMEAN 5
#define mainUNIFMAX 6
#define mainUNIFCAMERA 7
#define mainUNIFcLIGHT 23
#define mainUNIFdLIGHT 26
#define mainuNIFdCamera 29
#define mainUNIFSHININESS 32
#define mainUNIFcSPECULAR 33
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum,
				const texTexture *tex[], int varyDim, const double vary[],
				double rgbd[4]) {
	double frac = (vary[mainVARYWORLDZ] - unif[mainUNIFMIN])
				  / (unif[mainUNIFMAX] - unif[mainUNIFMIN]);
	double cDiffuse[3];
	cDiffuse[mainUNIFR] = unif[mainUNIFR] * (frac + 1.0) / 2.0;
	cDiffuse[mainUNIFG] = unif[mainUNIFG] * (frac + 1.0) / 2.0;
	cDiffuse[mainUNIFB] = unif[mainUNIFB] * (frac + 1.0) / 2.0;
	/* Texture stuff */
	if (tex != NULL){
		double sample[tex[0]->texelDim], thisTex[2] = {vary[mainVARYS],vary[mainVARYT]};
		vecScale(2, 1/vary[mainVARYW], thisTex, thisTex);
		texSample(tex[0], thisTex[0], thisTex[1], sample);
		cDiffuse[mainUNIFR] *= sample[mainTEXR];
		cDiffuse[mainUNIFG] *= sample[mainTEXG];
		cDiffuse[mainUNIFB] *= sample[mainTEXB];
	}
	/* Lambertian diffuse reflection */
    double diffuse[3];
	vecMultiply(3,cDiffuse,&unif[mainUNIFcLIGHT],diffuse);
	double dNormal[3] = {vary[mainVARYN],vary[mainVARYO],vary[mainVARYP]};
	double dNormalUnit[3];
	vecUnit(3,dNormal,dNormalUnit);
	double iDIFF = vecDot(3, dNormalUnit, &unif[mainUNIFdLIGHT]);
	if (iDIFF < 0.0)
        iDIFF = 0.0;
	vecScale(3, iDIFF, diffuse, diffuse);
    /* Specular reflection */
    double dReflect[3], temp[3], specular[3];// = {0.0, 0.0, 0.0};
    double scalar = 2 * vecDot(3, &unif[mainUNIFdLIGHT], dNormalUnit);
    vecScale(3, scalar, dNormalUnit, temp);
    vecSubtract(3, temp, &unif[mainUNIFdLIGHT], dReflect);
    double iSPEC = vecDot(3, dReflect, &unif[mainuNIFdCamera]);
    if (iDIFF == 0.0 || iSPEC < 0.0)
        iSPEC = 0.0;
    iSPEC = pow(iSPEC, unif[mainUNIFSHININESS]);
    vecMultiply(3, &unif[mainUNIFcSPECULAR], &unif[mainUNIFcLIGHT], specular);
    vecScale(3, iSPEC, specular, specular);
    /* Ambient reflection */
//    double [3] ambient;
//    vecMultiply(3, cDiffuse, )
    /* Adding things up */
    vecAdd(3, diffuse, specular, rgbd);
	rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim,
					 const double attr[], int varyDim, double vary[]) {
	double attrHom[4] = {attr[mainATTRX], attr[mainATTRY], attr[mainATTRZ], 1.0};
	double worldHom[4], varyHom[4], varyNormalHom[4];
	/* The modeling transformation is just Z-translation. So this code is much
    simpler than the usual matrix multiplication. */
	vecCopy(4, attrHom, worldHom);
	worldHom[2] += unif[mainUNIFMODELING];
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
	vecCopy(4, varyHom, vary);
	vary[mainVARYWORLDZ] = worldHom[mainATTRZ];
	/* Rotating but not translating normal vector varyings */
	double normalHom[4] = {attr[mainATTRN],attr[mainATTRO],attr[mainATTRP],0.0};
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), normalHom, varyNormalHom);
	/* Copying stuff */
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
	vary[mainVARYN] = varyNormalHom[0];
	vary[mainVARYO] = varyNormalHom[1];
	vary[mainATTRP] = varyNormalHom[2];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
texTexture texture;
camCamera cam;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
/* Camera control. */
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 256.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;
/* Meshes to be rendered. */
meshMesh grass;
double unifGrass[3 + 1 + 3 + 16 + 3 + 3 + 3 + 1 + 3] = {
		0.0, 1.0, 0.0,
		0.0,
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
        1.0,
        1.0, 1.0, 1.0};
meshMesh rock;
double unifRock[3 + 1 + 3 + 16 + 3 + 3 + 3 + 1 + 3] = {
		1.0, 1.0, 1.0,
		0.0,
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
        1.0,
		1.0, 1.0, 1.0};
meshMesh water;
double unifWater[3 + 1 + 3 + 16 + 3 + 3 + 3 + 1 + 3] = {
		0.0, 0.0, 1.0,
		0.0,
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0,
        1.0,
		1.0, 1.0, 1.0};
double dLight[3] = {256.0, 256.0, 256.0};

/*** User interface ***/

void fixUNIFdCAMERA(const double unif[], camCamera *cam){
	double isom[4][4], dCameraHomog[4] = {0.0, 0.0, 0.0, 1.0}, temp[4];
	vecCopy(3, &unif[mainuNIFdCamera], dCameraHomog);
	isoGetHomogeneous(&cam->isometry,isom);
	mat441Multiply(isom, dCameraHomog, temp);
	vecCopy(3, temp, &unif[mainuNIFdCamera]);
}

void render(void) {
	double view[4][4], projInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	vecCopy(16, (double *)projInvIsom, &unifGrass[mainUNIFCAMERA]);
	fixUNIFdCAMERA(unifGrass, &cam);
	meshRender(&grass, &buf, view, &sha, unifGrass, tex);
	vecCopy(16, (double *)projInvIsom, &unifRock[mainUNIFCAMERA]);
	fixUNIFdCAMERA(unifRock, &cam);
	meshRender(&rock, &buf, view, &sha, unifRock, NULL);
	vecCopy(16, (double *)projInvIsom, &unifWater[mainUNIFCAMERA]);
	fixUNIFdCAMERA(unifWater, &cam);
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
	else if (key == GLFW_KEY_Z)
		cameraTarget[0] -= 5;
	else if (key == GLFW_KEY_C)
		cameraTarget[0] += 5;
	else if (key == GLFW_KEY_RIGHT)
		cameraTarget[1] -= 1;
	else if (key == GLFW_KEY_LEFT)
		cameraTarget[1] += 1;
	else if (key == GLFW_KEY_UP)
		cameraTarget[2] -= 1;
	else if (key == GLFW_KEY_DOWN)
		cameraTarget[2] += 1;
	else if (key == GLFW_KEY_O)
		unifWater[mainUNIFMODELING] -= 0.1;
	else if (key == GLFW_KEY_P)
		unifWater[mainUNIFMODELING] += 0.1;
	camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
				  mainSCREENSIZE);
	camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
				 int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		if (cam.projectionType == camORTHOGRAPHIC)
			camSetProjectionType(&cam, camPERSPECTIVE);
		else if(cam.projectionType == camPERSPECTIVE)
			camSetProjectionType(&cam,camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
					  mainSCREENSIZE);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
	}
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
	unifGrass[mainUNIFMIN] = landMin;
	unifGrass[mainUNIFMEAN] = landMean;
	unifGrass[mainUNIFMAX] = landMax;
	unifRock[mainUNIFMIN] = landMin;
	unifRock[mainUNIFMEAN] = landMean;
	unifRock[mainUNIFMAX] = landMax;
	unifWater[mainUNIFMIN] = landMin;
	unifWater[mainUNIFMEAN] = landMean;
	unifWater[mainUNIFMAX] = landMax;
	double dLightUnit[3];
	vecUnit(3, dLight, dLightUnit);
	vecCopy(3, dLightUnit, &unifGrass[mainUNIFdLIGHT]);
	vecCopy(3, dLightUnit, &unifRock[mainUNIFdLIGHT]);
	vecCopy(3, dLightUnit, &unifWater[mainUNIFdLIGHT]);
	meshMesh land;
	/* Begin configuring scene. */
	if (pixInitialize(mainSCREENSIZE, mainSCREENSIZE, "Pixel Graphics") != 0)
		return 1;
	else if (depthInitialize(&buf, mainSCREENSIZE, mainSCREENSIZE) != 0)
		return 2;
	else if (meshInitializeLandscape(&land, landNum, landNum, 1.0,(double *)landData) != 0)
		return 3;
	else if (meshInitializeDissectedLandscape(&grass, &land, M_PI / 4.0,1) != 0)
		return 4;
	else if (meshInitializeDissectedLandscape(&rock, &land, M_PI / 4.0,0) != 0)
		return 5;
	else if (meshInitializeLandscape(&water, 2, 2, landNum - 1.0,(double *)waterData) != 0)
		return 6;
	else if (texInitializeFile(&texture, "../26710550-idyllic-seamless-grass-texture.jpg") != 0)
		return 7;
	else {
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		meshDestroy(&land);
		/* Continue configuring scene. */
		sha.unifDim = 3 + 1 + 3 + 16 + 3 + 3 + 3 + 1 + 3;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 4 + 1 + 2 + 3;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 0;
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
					  mainSCREENSIZE);
		vec3Set(landNum / 2.0, landNum / 2.0,
				landData[landNum / 2][landNum / 2], cameraTarget);
		camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		render();
		/* User interface. */
		pixSetKeyDownHandler(handleKeyAny);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyUp);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		meshDestroy(&grass);
		meshDestroy(&rock);
		meshDestroy(&water);
		texDestroy(&texture);
		depthDestroy(&buf);
		return 0;
	}
}

