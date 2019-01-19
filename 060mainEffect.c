


/* On macOS, compile with...
    clang 060mainEffect.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#include "000pixel.h"
#include "030vector.c"
#include "030matrix.c"
#include "040texture.c"
#include "050shading.c"

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
#define mainUNIFR 0
#define mainUNIFG 1
#define mainUNIFB 2
#define mainTEXR 0
#define mainTEXG 1
#define mainTEXB 2

/* attr has already been interpolated from the vertex attributes. tex is an 
array of texNum elements, each of which is of type texTexture *. rgb is the 
output parameter. The other parameters should be self-explanatory. For reasons 
that become clear later in the course, colorPixel is allowed to use (A) compile-
time constants such as mainATTRX, (B) functions such as sin() and texSample(), 
(C) its parameters such as unifDim, and (D) any variables that it declares 
locally. It is not allowed to use any other variables, such as global variables 
that aren't parameters. */
void colorPixel(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int attrDim, const double attr[], 
		double rgb[3]) {
    double sample1[3];
    double sample2[3];
	texSample(tex[0],attr[2],attr[3],sample2);
    texSample(tex[1],sample2[0],sample2[1],sample1);
	rgb[0] = sample1[0]*unif[0];
    rgb[1] = sample1[1]*unif[1];
    rgb[2] = sample1[2]*unif[2];
}

/* We have to include 050triangle.c after defining colorPixel, because it 
refers to colorPixel. (Later in the course we handle this issue better.) */
#include "050triangle.c"
#include "070mesh.c"

/* This struct is initialized in main() below. */
shaShading sha;
/* Here we make an array of one texTexture pointer, in such a way that the 
const qualifier can be enforced throughout the surrounding code. C is confusing 
for stuff like this. Don't worry about mastering C at this level. It doesn't 
come up much in our course. */
texTexture texture1;
texTexture texture2;
const texTexture *textures[2] = {&texture1,&texture2};
const texTexture **tex = textures;

void draw(void) {
	pixClearRGB(0.0, 0.0, 0.0);
	double a[5] = {256,512, 0.5, 1.0};
	double b[5] = {0,0,0,0};
	double c[5] = {512, 0, 1,0};
	double unif[3] = {1.0, 1.0, 1.0};
	triRender(&sha, unif, tex, a, b, c);
}

void handleKeyUp(int key, int shiftIsDown, int controlIsDown, 
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_ENTER) {
		if (texture1.filtering == texLINEAR){
			texSetFiltering(&texture1, texNEAREST);
            texSetFiltering(&texture2, texNEAREST);
        }
		else{
			texSetFiltering(&texture1, texLINEAR);
            texSetFiltering(&texture2, texNEAREST);
        }
		draw();
	}
}

void handleTimeStep(double oldTime, double newTime) {
	// if (floor(newTime) - floor(oldTime) >= 1.0)
	// 	printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if (texInitializeFile(&texture1, "Noether_retusche_nachcoloriert.jpg") != 0)
			return 2;
        if (texInitializeFile(&texture2, "iStock_000023951981Large.jpg") != 0)
			return 3;
		else {
			texSetFiltering(&texture1, texNEAREST);
			texSetLeftRight(&texture1, texREPEAT);
			texSetTopBottom(&texture1, texREPEAT);
            texSetFiltering(&texture2, texNEAREST);
			texSetLeftRight(&texture2, texREPEAT);
			texSetTopBottom(&texture2, texREPEAT);
			sha.unifDim = 3;
			sha.attrDim = 2 + 2;
			sha.texNum = 1;
			draw();
			pixSetKeyUpHandler(handleKeyUp);
			pixSetTimeStepHandler(handleTimeStep);
			pixRun();
			texDestroy(&texture1);
            texDestroy(&texture2);
			return 0;
		}
	}
}


