


/* On macOS, compile with...
    clang 620mainLighting.c 000pixel.o -lglfw -framework OpenGL
*/
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include "000pixel.h"
#include <GLFW/glfw3.h>

#include "610vector.c"
#include "140matrix.c"
#include "600isometry.c"
#include "600camera.c"
#include "040texture.c"

#define SCREENWIDTH 512
#define SCREENHEIGHT 512

#define EPSILON 0.00000001
//#define INFINITY 100000000

camCamera camera;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

/* These are our two sphere bodies. */
isoIsometry isomA, isomB;
double radiusA = 1.0, radiusB = 1.5;
double colorA[3] = {1.0, 0.0, 1.0}, colorB[3] = {1.0, 1.0, 0.0};

/* Texture */
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;

/* Lighting */
double cLight[3] = {1.0, 1.0, 1.0};
double dLight[3] = {1.0, 1.0, 1.0};
double cSpecular[3] = {1.0, 1.0, 1.0};
int shininess = 20;
double cAmbient[3] = {0.5, 0.5, 0.5};

/* Rendering ******************************************************************/

/* The intersected member contains a code for how a ray (first) intersects the 
surface of a body: 0 if it doesn't intersect (or is tangent), 1 if it 
intersects leaving the body, or -1 if it intersects entering the body. If the 
intersected code is not 0, then time records the time of the intersection. */
typedef struct rayRecord rayRecord;
struct rayRecord {
	int intersected;
	double t;
};

/* Given the isometry and radius of a spherical body. Given a starting time and 
an ending time. Returns the ray record describing how the ray first intersects 
the sphere. */
rayRecord sphereIntersection(const isoIsometry *isom, double radius,
		const double e[3], const double d[3], double tStart, double tEnd) {
	rayRecord result;
	double center[3], eMinusCenter[3];
	vecCopy(3, isom->translation, center);
	double a = pow(vecLength(3, d), 2);
	vecSubtract(3, e, center, eMinusCenter);
	double b = 2 * vecDot(3, d, eMinusCenter);
	double c = vecDot(3, eMinusCenter, eMinusCenter) - radius * radius;
	double discriminant = b * b - 4 * a * c;
	if (discriminant <= 0){
		result.intersected = 0;
		result.t = 0;
	}
	else{
        double t1 = ( -b - sqrt(discriminant) ) / (2 * a);
        double t2 = ( -b + sqrt(discriminant) ) / (2 * a);
		if (t1 >= tStart && t1 <= tEnd){
			result.intersected = -1;
			result.t = t1;
		}
		else if (t2 >= tStart && t2 <= tEnd){
			result.intersected = 1;
			result.t = t2;
		}
		else{
			result.intersected = 0;
			result.t = 0;
		}
	}
	return result;
}

/* Fills the RGB color with the color sampled from the specified texture. */
void sphereColor(const isoIsometry *isom, double radius, const double e[3],
				 const double d[3], double tEnd, const texTexture *tex, double rgb[3]){
	double tTimesD[3], world[3], local[3];
	double s, t;
	vecScale(3, tEnd, d, tTimesD);
	vecAdd(3, e, tTimesD, world);
	isoUntransformPoint(isom, world, local);
	vec3Rectangular(local, &radius, &s, &t);
	texSample(tex, s / M_PI, t/(2 * M_PI), rgb);
}

void addLighting(const isoIsometry *isom, const double e[3],
				 const double d[3], double tEnd, double rgb[3]){
	double tTimesD[3], world[3], center[3], worldMinusCenter[3], temp[3];
	double dNormal[3];
	vecCopy(3, isom->translation, center);
	vecScale(3, tEnd, d, tTimesD);
	vecAdd(3, e, tTimesD, world);
	vecSubtract(3, world, center, worldMinusCenter);
	vecUnit(3, worldMinusCenter, dNormal);
	double diffuse[3] = {0.0, 0.0, 0.0}, specular[3] = {0.0, 0.0, 0.0}, ambient[3] = {0.0, 0.0, 0.0};
	/* Lambertian diffuse reflection */
//	double cDiffuse[3];
//	vecCopy(3, rgb, cDiffuse);
//	vecMultiply(3, cDiffuse, cLight, diffuse);
//	double iDIFF = vecDot(3, dNormal, dLight);
//	if (iDIFF < 0.0)
//		iDIFF = 0.0;
//	vecScale(3, iDIFF, diffuse, diffuse);
//	/* Specular reflection that reflects camera instead of light */
//	double dCamera[3], dReflect[3];
//	vecUnit(3, e, dCamera); // hmmmm
//	double scalar = 2 * vecDot(3, dCamera, dNormal);
//	vecScale(3, scalar, dNormal, temp);
//	vecSubtract(3, temp, dCamera, dReflect);
//	double iSPEC = vecDot(3, dReflect, dLight);
//	if (iDIFF == 0.0 || iSPEC < 0.0)
//		iSPEC = 0.0;
//	iSPEC = pow(iSPEC, shininess);
//	vecMultiply(3, cSpecular, cLight, specular);
//	vecScale(3, iSPEC, specular, specular);
//	/* Ambient reflection */
//	vecMultiply(3, cDiffuse, cAmbient, ambient);
	/* Adding things up */
	vecAdd3(3, diffuse, specular, ambient, temp);
	vecAdd(3, temp, rgb, rgb);

}

void render(void) {
	double homog[4][4], screen[4], world[4], e[3], d[3], rgb[3];
	double tStart, tEnd;
	rayRecord recA, recB;
	int i, j;
	pixClearRGB(0.0, 0.0, 0.0);
	/* Get camera world position e and transformation from screen to world. */
	vecCopy(3, camera.isometry.translation, e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);
	/* Each screen point is arbitrarily chosen on the near plane. */
	screen[2] = 0.0;
	screen[3] = 1.0;
	for (i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (j = 0; j < SCREENHEIGHT; j += 1) {
			screen[1] = j;
			/* Compute the direction d from the camera to the pixel. */
			mat441Multiply(homog, screen, world);
			vecScale(4, 1 / world[3], world, world);
			vecSubtract(3, world, e, d);
			/* Prepare to loop over all bodies. */
			tStart = EPSILON;
			tEnd = INFINITY;
			/* Test the first sphere. */
			recA = sphereIntersection(&isomA, radiusA, e, d, tStart, tEnd);
			if (recA.intersected != 0)
				tEnd = recA.t;
			/* Test the second sphere. */
			recB = sphereIntersection(&isomB, radiusB, e, d, tStart, tEnd);
			if (recB.intersected != 0 && recB.t < tEnd)
				tEnd = recB.t;
			/* Choose the winner. */
			if (recA.intersected != 0 && recB.intersected != 0) {
				if (recB.t <= recA.t) {
					sphereColor(&isomB, radiusB, e, d, tEnd, tex[0], rgb);
					addLighting(&isomB, e, d, tEnd, rgb);
				}
				else {
					sphereColor(&isomA, radiusA, e, d, tEnd, tex[0], rgb);
					addLighting(&isomA, e, d, tEnd, rgb);
				}
			}
			else if (recA.intersected != 0 && recB.intersected == 0) {
				sphereColor(&isomA, radiusA, e, d, tEnd, tex[0], rgb);
				addLighting(&isomA, e, d, tEnd, rgb);
			}
			else if (recA.intersected == 0 && recB.intersected != 0) {
				sphereColor(&isomB, radiusB, e, d, tEnd, tex[0], rgb);
				addLighting(&isomB, e, d, tEnd, rgb);
			}
			else
				vec3Set(0.0, 0.0, 0.0, rgb);
			pixSetRGB(i, j, rgb[0], rgb[1], rgb[2]);
		}
	}
}

/* User interface *************************************************************/
void handleKeyCamera(int key) {
	if (key == GLFW_KEY_W)
		cameraPhi -= 0.1;
	else if (key == GLFW_KEY_A)
		cameraTheta -= 0.1;
	else if (key == GLFW_KEY_S)
		cameraPhi += 0.1;
	else if (key == GLFW_KEY_D)
		cameraTheta += 0.1;
	else if (key == GLFW_KEY_E)
		cameraRho *= 0.9;
	else if (key == GLFW_KEY_C)
		cameraRho *= 1.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}
void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_C)
		handleKeyCamera(key);
}
void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_Q)
		cameraMode = 1 - cameraMode;
	else
		handleKeyAny(key, shiftIsDown, controlIsDown, altOptionIsDown, 
			superCommandIsDown);
}
void handleTimeStep(double oldTime, double newTime) {
	if (floor(newTime) - floor(oldTime) >= 1.0)
		printf("handleTimeStep: %f frames/s\n", 1.0 / (newTime - oldTime));
	render();
}

int main(void) {
	if (pixInitialize(SCREENWIDTH, SCREENHEIGHT, "Ray Tracing") != 0)
		return 1;
	else if (texInitializeFile(&texture, "Noether_retusche_nachcoloriert.jpg") != 0)
		return 2;
	else {
		/* Initialize the scene. */
		camSetProjectionType(&camera, camPERSPECTIVE);
		camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
			SCREENHEIGHT);
		camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		double center[3] = {0.0, 0.0, 0.0}, axis[3] = {1.0, 0.0, 0.0}, r[3][3];
		mat33AngleAxisRotation(0.0, axis, r);
		isoSetTranslation(&isomA, center);
		isoSetRotation(&isomA, r);
		vec3Set(1.0, 0.0, 1.0, center);
		isoSetTranslation(&isomB, center);
		isoSetRotation(&isomB, r);
		/* Initialize texture */
		texSetFiltering(&texture, texLINEAR);
		texSetLeftRight(&texture, texREPEAT);
		texSetTopBottom(&texture, texREPEAT);
		texture.texelDim = 3;
		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Cleaning things up */
		texDestroy(&texture);
		return 0;
	}
}


