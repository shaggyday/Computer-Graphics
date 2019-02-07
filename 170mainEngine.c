/* 1. There should be 8 vertices in a box
 * 2. the vectors formed by XYZ attributes are perpendicular to vectors formed by NOP
 * 3. The relationship would not change
 */


/* On macOS, compile with...
    clang 160mainInterpolating.c 000pixel.o 170engine.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "170engine.h"
//#include "000pixel.h"
//#include "120vector.c"
//#include "140matrix.c"
//#include "140isometry.c"
//#include "150camera.c"
//#include "040texture.c"
//#include "130shading.c"
//#include "130depth.c"
//#include "130triangle.c"
//#include "160mesh.c"

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
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainUNIFCAMERA 19
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* Solid colors, tinted from dark (low saturation at low elevation) to light 
(high saturation at high elevation). */
void colorPixel(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[],
		double rgbd[4]) {
	double sample[tex[0]->texelDim], thisTex[2];
	thisTex[0] = vary[mainVARYS] / vary[mainVARYW];
	thisTex[1] = vary[mainVARYT] / vary[mainVARYW];
	texSample(tex[0], thisTex[0], thisTex[1], sample);
	rgbd[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgbd[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgbd[2] = sample[mainTEXB] * unif[mainUNIFB];
	rgbd[3] = vary[mainVARYZ];
}

void transformVertex(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {
	double attrHom[4] = {attr[0], attr[1], attr[2], 1.0};
	double worldHom[4], varyHom[4];
    mat441Multiply((double(*)[4])(&unif[mainUNIFMODELING]), attrHom, worldHom);
	mat441Multiply((double(*)[4])(&unif[mainUNIFCAMERA]), worldHom, varyHom);
	vecCopy(4, varyHom, vary);
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
}

/*** Globals ***/

/* Crucial infrastructure. */
depthBuffer buf;
shaShading sha;
texTexture texture;
isoIsometry iso;
camCamera cam;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;

/* Meshes to be rendered. */
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
/* Camera control. */
double cameraTarget[3] = {256.0, 256.0, 256.0};
double cameraRho = 356.0, cameraPhi = M_PI / 4.0, cameraTheta = 0.0;

/* Modeling */
double translationVector[3] = {256.0, 256.0, 256.0};
double rotationAngle = 0.0;

/*** User interface ***/

void render(void) {
	double view[4][4], projInvIsom[4][4];
	camGetProjectionInverseIsometry(&cam, projInvIsom);
	mat44Viewport(mainSCREENSIZE, mainSCREENSIZE, view);
    double rot[3][3], isom[4][4], rotationAxis[3];;
    vec3Set(1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0), rotationAxis);
    mat33AngleAxisRotation(rotationAngle, rotationAxis, rot);
    isoSetRotation(&iso,rot);
    isoGetHomogeneous(&iso,isom);
    vecCopy(16, (double *)isom, &unif[mainUNIFMODELING]);
    vecCopy(16, (double *)projInvIsom, &unif[mainUNIFCAMERA]);
	pixClearRGB(0.0, 0.0, 0.0);
	depthClearDepths(&buf, 1000000000.0);
	meshRender(&mesh1, &buf, view, &sha, unif, tex);
	meshRender(&mesh2, &buf, view, &sha, unif, tex);
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
		cameraTarget[0] -= 5;
	else if (key == GLFW_KEY_SEMICOLON)
		cameraTarget[0] += 5;
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
    rotationAngle += (newTime - oldTime);
	render();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&texture, "../Noether_retusche_nachcoloriert.jpg") != 0)
		return 2;
	else if (meshInitializeBox(&mesh1, -128.0, 128.0, -64.0, 64.0, -65.0, -10) != 0)
		return 3;
	else if (meshInitializeSphere(&mesh2, 64.0, 16, 32) != 0)
		return 4;
	else if(depthInitialize(&buf,512,512)!=0)
		return 5;
	else {
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		/* Continue configuring scene. */
		sha.unifDim = 3 + 32;
		sha.attrDim = 3 + 2 + 3;
		sha.varyDim = 4 + 2;
		sha.colorPixel = colorPixel;
		sha.transformVertex = transformVertex;
		sha.texNum = 1;
		camSetProjectionType(&cam, camORTHOGRAPHIC);
		camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE, 
			mainSCREENSIZE);
        isoSetTranslation(&iso,translationVector);
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
		meshDestroy(&mesh1);
		meshDestroy(&mesh2);
		texDestroy(&texture);
		depthDestroy(&buf);
		return 0;
	}
}


