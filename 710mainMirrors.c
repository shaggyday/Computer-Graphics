


/* On macOS, compile with...
    clang 710mainMirrors.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "000pixel.h"

#include "610vector.c"
#include "140matrix.c"
#include "040texture.c"
#include "610isometry.c"
#include "600camera.c"
#include "680light.c"
#include "710ray.c"

#include "680omnidirectional.c"
#include "690directional.c"

#define SCREENWIDTH 512
#define SCREENHEIGHT 512
#define BODYNUM 6
#define LIGHTNUM 2
#define RECURSIONNUM 1

double omniPLight[3] = {3.0, 3.0, 3.0};
double omniCLight[3] = {1.0, 0.0, 0.0};
double directDLight[3] = {-1.0, -1.0, 1.0};
double directCLight[3] = {0.0, 0.0, 0.5};
double cAmbient[3] = {0.1, 0.1, 0.1};

#include "710cylinder.c"
#include "710sphere.c"
#include "710plane.c"

camCamera camera;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 10.0, cameraPhi = M_PI / 3.0, cameraTheta = -M_PI / 3.0;
int cameraMode = 0;

/* Texture */
texTexture textureEmmy, textureRed, textureGreen, textureBlue;
cylCylinder cylRed, cylGreen, cylBlue;
sphereSphere sphA, sphB;
plaPlane plane;
void *bodies[BODYNUM] = {&cylRed, &cylGreen, &cylBlue, &sphA, &sphB, &plane};

/* Lighting */
omniLight omni;
directLight direct;
void *lights[LIGHTNUM] = {&direct, &omni};

/* Rendering ******************************************************************/

void render(void) {
	double homog[4][4], world[4];
	double screen[4] = {0.0, 0.0, 0.0, 1.0};
	rayQuery query;
	rayResponse response;
	/* Compute the position e of the camera. */
	vecCopy(3, camera.isometry.translation, query.e);
	camWorldFromScreenHomogeneous(&camera, SCREENWIDTH, SCREENHEIGHT, homog);
	for (int i = 0; i < SCREENWIDTH; i += 1) {
		screen[0] = i;
		for (int j = 0; j < SCREENHEIGHT; j += 1) {
			double rgb[3] = {0.0, 0.0, 0.0};
			screen[1] = j;
			/* Compute the direction d from the camera to the pixel. */
			mat441Multiply(homog, screen, world);
			vecScale(4, 1.0 / world[3], world, world);
			vecSubtract(3, world, query.e, query.d);
			/* Query the scene to find the intersection, if any. */
			query.tStart = rayEPSILON;
			query.tEnd = rayINFINITY;
			/* Color the pixel. */
			response = rayColor(BODYNUM, bodies, LIGHTNUM, lights, cAmbient, &query, RECURSIONNUM, rgb);
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
	else if (key == GLFW_KEY_Q)
		cameraRho *= 1.1;
	camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
		SCREENHEIGHT);
	camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
}

void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S ||
			key == GLFW_KEY_D || key == GLFW_KEY_E || key == GLFW_KEY_Q)
		handleKeyCamera(key);
}
void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
	if (key == GLFW_KEY_C)
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

void initializeCylinder(cylCylinder *cyl, double radius, 
		const double rot[3][3], const double transl[3], texTexture *texture) {
	cyl->class = &cylClass;
	isoSetRotation(&(cyl->isometry), rot);
	isoSetTranslation(&(cyl->isometry), transl);
	cyl->radius = radius;
	cyl->texture = texture;
}
void initializeSphere(sphereSphere *sph, double radius,
		const double rot[3][3], const double transl[3], texTexture *texture) {
    sph->class = &sphClass;
	isoSetRotation(&(sph->isometry), rot);
	isoSetTranslation(&(sph->isometry), transl);
    sph->radius = radius;
    sph->texture = texture;
}
void initializePlane(plaPlane *plane,
		const double rot[3][3], const double transl[3], texTexture *texture){
	plane->class = &planeClass;
	isoSetRotation(&(plane->isometry), rot);
	isoSetTranslation(&(plane->isometry), transl);
	plane->texture = texture;
}
void initializeOmni(omniLight *omni, double pLight[3], double cLight[3]){
	omni->class = &omniClass;
	vecCopy(3, cLight, omni->cLight);
	vecCopy(3, pLight, omni->pLight);
}
void initializeDirect(directLight *direct, double dLight[3], double cLight[3]){
	direct->class = &directClass;
	vecCopy(3, cLight, direct->cLight);
	vecUnit(3, dLight, dLight);
	vecCopy(3, dLight, direct->dLight);
}

int main(void) {
	if (pixInitialize(SCREENWIDTH, SCREENHEIGHT, "Ray Tracing") != 0)
		return 1;
    else if (texInitializeFile(&textureEmmy, "Noether_retusche_nachcoloriert.jpg") != 0)
        return 2;
	else {
		/* Initialize the scene. */
		camSetProjectionType(&camera, camPERSPECTIVE);
		camSetFrustum(&camera, M_PI / 6.0, cameraRho, 10.0, SCREENWIDTH, 			
			SCREENHEIGHT);
		camLookAt(&camera, cameraTarget, cameraRho, cameraPhi, cameraTheta);
		double rgb[3] = {1.0, 0.0, 0.0};
		if (texInitializeSolid(&textureRed, 4, 4, 3, rgb) != 0)
			return 2;
		vec3Set(0.0, 1.0, 0.0, rgb);
		if (texInitializeSolid(&textureGreen, 4, 4, 3, rgb) != 0)
			return 3;
		vec3Set(0.0, 0.0, 1.0, rgb);
		if (texInitializeSolid(&textureBlue, 4, 4, 3, rgb) != 0)
			return 4;
		/* Initialize bodies */
		double center[3] = {0.0, 0.0, 0.0}, rot[3][3];
		double axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
		mat33AngleAxisRotation(2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylRed, 0.1, rot, center, &textureRed);
		mat33AngleAxisRotation(-2.0 * M_PI / 3.0, axis, rot);
		initializeCylinder(&cylGreen, 0.1, rot, center, &textureGreen);
		mat33AngleAxisRotation(0.0, axis, rot);
		initializeCylinder(&cylBlue, 0.1, rot, center, &textureBlue);
		initializePlane(&plane, rot, center, &textureEmmy);
		initializeSphere(&sphA, 1.0, rot, center, &textureEmmy);
        vec3Set(2.0, 0.0, 2.0, center);
        initializeSphere(&sphB, 1.5, rot, center, &textureEmmy);
        /* Initialize texture */
        texSetFiltering(&textureEmmy, texLINEAR);
        texSetLeftRight(&textureEmmy, texREPEAT);
        texSetTopBottom(&textureEmmy, texREPEAT);
		textureEmmy.texelDim = 3;
		/* Initialize lighting */
		initializeOmni(&omni, omniPLight, omniCLight);
		initializeDirect(&direct, directDLight, directCLight);
		/* Initialize and run the user interface. */
		pixSetKeyDownHandler(handleKeyDown);
		pixSetKeyRepeatHandler(handleKeyAny);
		pixSetKeyUpHandler(handleKeyAny);
		pixSetTimeStepHandler(handleTimeStep);
		pixRun();
		/* Clean up. */
		texDestroy(&textureRed);
		texDestroy(&textureGreen);
		texDestroy(&textureBlue);
        texDestroy(&textureEmmy);
		return 0;
	}
}


