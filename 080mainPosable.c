


/* On macOS, compile with...
    clang 081mainPosable.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "100matrix.c"
#include "040texture.c"
#include "090shading.c"

/* It is important that colorPixel correctly parse the data that we give it. To 
help avoid errors in communication, we set up some compile-time constants. 
Notice that the documentation for the new triRender requires mainATTRX to be 0 
and mainATTRY to be 1. */
#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainATTRR 4
#define mainATTRG 5
#define mainATTRB 6
#define mainVARYX 0
#define mainVARYY 1 
#define mainVARYS 2
#define mainVARYT 3
#define mainVARYR 4
#define mainVARYG 5
#define mainVARYB 6
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFT 3
#define mainUNIFC 4
#define mainUNIFD 5
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2
#define mainTHETA M_PI/4
#define mainT0 30
#define mainT1 30


/* attr has already been interpolated from the vertex attributes. tex is an 
array of texNum elements, each of which is of type texTexture *. rgb is the 
output parameter. The other parameters should be self-explanatory. For reasons 
that become clear later in the course, colorPixel is allowed to use (A) compile-
time constants such as mainATTRX, (B) functions such as sin() and texSample(), 
(C) its parameters such as unifDim, and (D) any variables that it declares 
locally. It is not allowed to use any other variables, such as global variables 
that aren't parameters. */
void colorPixel(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[]) {
	texSample(tex[0],vary[mainVARYS],vary[mainVARYT],rgb);
	int i;
	for(i = 0;i < unifDim;i = i + 1)
		rgb[i] *= vary[i+4]*unif[i]; 
}

void transformVertex(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {
    double v[2] = {attr[mainATTRX],attr[mainATTRY]};
	double m[2][2]; 
	mat22Rotation(unif[mainUNIFT],m);
	double vTimesM[2],varyXY[2];
	mat221Multiply(m,v,vTimesM);
    vary[mainVARYX] = vTimesM[0] + unif[mainUNIFC];
    vary[mainVARYY] = vTimesM[1] + unif[mainUNIFD];
    vary[mainVARYS] = attr[mainATTRS];
    vary[mainVARYT] = attr[mainATTRT];
}

/* We have to include 050triangle.c after defining colorPixel, because it 
refers to colorPixel. (Later in the course we handle this issue better.) */
#include "090triangle.c"
#include "090mesh.c"

shaShading sha;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh theMesh;
meshMesh *mesh = &theMesh;

double time = 0;

void draw() {
	pixClearRGB(0.0, 0.0, 0.0);
	double unif[6] = {1.0, 1.0, 1.0,mainTHETA,mainT0,mainT1};
	unif[mainUNIFT] *= time;
	unif[mainUNIFC] *= time;
	unif[mainUNIFD] *= time;
	meshRender(mesh,&sha,unif,tex);
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
}

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
        time += (newTime - oldTime);
		draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if (texInitializeFile(&texture, "../Noether_retusche_nachcoloriert.jpg") != 0)
			return 2;
		else {
			if(meshInitializeEllipse(mesh,256,256,200,200,40)!=0)
				return 3;
            else{
				texSetFiltering(&texture, texNEAREST);
				texSetLeftRight(&texture, texCLAMP);
				texSetTopBottom(&texture, texCLAMP);
				sha.unifDim = 3 + 3;
				sha.attrDim = 2 + 2;
				sha.varyDim = 2 + 2;
				sha.texNum = 1;
				sha.colorPixel = colorPixel;
				sha.transformVertex = transformVertex;
				draw();
				pixSetKeyUpHandler(handleKeyUp);
				pixSetTimeStepHandler(handleTimeStep);
				pixRun();
				texDestroy(&texture);
				meshDestroy(mesh);
				return 0;
			}
		}
	}
}


