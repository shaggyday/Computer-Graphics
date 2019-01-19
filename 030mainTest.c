


/* On macOS, compile with...
    clang 030mainTest.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

#include "030vector.c"
#include "030matrix.c"
#include "020triangle.c"

double a[2] = {144.0, -156.0};
double b[2] = {244.0, 244.0};
double c[2] = {-226.0, -226.0};
double rgb[3] = {1.0, 1.0, 0.0};
double angle = 0.0;

void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/sec\n", 1.0 / (newTime - oldTime));
	double transl[2] = {256.0, 256.0};
	double aa[2], bb[2], cc[2], rot[2][2];
	angle += (newTime - oldTime) / 10.0;
	mat22Rotation(angle, rot);
	mat221Multiply(rot, a, aa);
	mat221Multiply(rot, b, bb);
	mat221Multiply(rot, c, cc);
	vecAdd(2, transl, aa, aa);
	vecAdd(2, transl, bb, bb);
	vecAdd(2, transl, cc, cc);
	pixClearRGB(0.0, 0.0, 0.0);
	triRender(aa, bb, cc, rgb);
}

int main(void) {
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	pixSetTimeStepHandler(handleTimeStep);
	pixRun();
	return 0;
}


