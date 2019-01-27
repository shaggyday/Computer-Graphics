


/* On macOS, compile with...
    clang 090mainAbstracted.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "120vector.c"
#include "120matrix.c"
#include "040texture.c"
#include "130shading.c"
/* New. We no longer need to include these files after colorPixel and 
transformVertex. So instead we include them up here. It's good C style to have 
all #includes in one section near the top of the file. */
#include "130triangle.c"
#include "090mesh.c"

#define mainATTRX 0
#define mainATTRY 1
#define mainATTRS 2
#define mainATTRT 3
#define mainVARYX 0
#define mainVARYY 1
#define mainVARYS 2
#define mainVARYT 3
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainUNIFMODELING 3
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

void colorPixel1(int unifDim, const double unif[], int texNum,
		const texTexture *tex[], int varyDim, const double vary[], 
		double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFR];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFB];
}

void colorPixel2(int unifDim, const double unif[], int texNum,
				const texTexture *tex[], int varyDim, const double vary[],
				double rgb[3]) {
	double sample[tex[0]->texelDim];
	texSample(tex[0], vary[mainVARYS], vary[mainVARYT], sample);
	rgb[0] = sample[mainTEXR] * unif[mainUNIFB];
	rgb[1] = sample[mainTEXG] * unif[mainUNIFG];
	rgb[2] = sample[mainTEXB] * unif[mainUNIFR];
}

void transformVertex(int unifDim, const double unif[], int attrDim, 
		const double attr[], int varyDim, double vary[]) {
	double attrHomog[3] = {attr[0],attr[1],1};
	mat331Multiply((double(*)[3])(&unif[mainUNIFMODELING]), attrHomog, vary);
	vary[mainVARYS] = attr[mainATTRS];
	vary[mainVARYT] = attr[mainATTRT];
}

shaShading sha1;
shaShading sha2;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;
meshMesh mesh1;
meshMesh mesh2;
double rotationAngle1 = 0;
double rotationAngle2 = 0;
double translationVector[2] = {0,0};
double unif1[3 + 9] = {1.0, 1.0, 1.0};
double unif2[3 + 9] = {1.0, 1.0, 1.0};
double I3[3][3] = {{1,0,0},{0,1,0},{0,0,1}};

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	meshRender(&mesh1, &sha1, unif1, tex);
	meshRender(&mesh2, &sha2, unif2, tex);
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
	unif1[mainUNIFR] = sin(newTime);
	unif1[mainUNIFG] = cos(oldTime);
	unif2[mainUNIFB] = sin(newTime);
	unif2[mainUNIFR] = cos(oldTime);
	rotationAngle1 += M_PI/25;
	rotationAngle2 -= M_PI/50;
	double isom[3][3];
	mat33Isometry(rotationAngle1, translationVector, isom);
	vecCopy(9, (double *)isom, &unif1[mainUNIFMODELING]);
	mat33Isometry(rotationAngle2, translationVector, isom);
	vecCopy(9, (double *)isom, &unif2[mainUNIFMODELING]);
	draw();
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else if (texInitializeFile(&texture, "../1507558141-3098.jpg") != 0)
		return 2;
	//else if (meshInitializeRectangle(&mesh, 0.0, 512.0, 0.0, 512.0) != 0) 
	else if (meshInitializeEllipse(&mesh1, 350, 350, 128.0, 128.0, 40) != 0)
		return 3;
	else if (meshInitializeRectangle(&mesh2, 100, 300, 100, 300.0) != 0)
		return 4;
	else {
		texSetFiltering(&texture, texNEAREST);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		sha1.unifDim = 3 + 9;
		sha1.attrDim = 2 + 2;
		sha1.varyDim = 2 + 2;
		sha1.texNum = 1;
		sha1.colorPixel = colorPixel1;
		sha1.transformVertex = transformVertex;
		sha2.unifDim = 3 + 9;
		sha2.attrDim = 2 + 2;
		sha2.varyDim = 2 + 2;
		sha2.texNum = 1;
		sha2.colorPixel = colorPixel2;
		sha2.transformVertex = transformVertex;
		vecCopy(9, (double *)I3, &unif1[mainUNIFMODELING]);
		vecCopy(9, (double *)I3, &unif2[mainUNIFMODELING]);
		draw();
		pixSetKeyUpHandler(handleKeyUp);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		meshDestroy(&mesh1);
		meshDestroy(&mesh2);
		texDestroy(&texture);
		return 0;
	}
}


