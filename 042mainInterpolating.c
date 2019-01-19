//Harry Tian

/* On macOS, compile with...
    clang 042mainInterpolating.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include "030vector.c" 
#include "030matrix.c"
#include "040texture.c"
#include "050shading.c"

void colorPixel(int unifDim, const double unif[], int texNum, 
		const texTexture *tex[], int attrDim, const double attr[], 
		double rgb[3]) {
			texSample(tex[0],attr[2],attr[3],rgb);
	// int i;
	// for(i = 0;i < unifDim;i = i + 1)
	// 	rgb[i] = attr[i+2]*unif[i]; 
}

#include "050triangle.c"

// shaShading shaShaShading;
// const shaShading *sha = &shaShaShading;
shaShading sha;
texTexture texture;
const texTexture *textures[1] = {&texture};
const texTexture **tex = textures;

void display(void){
	pixClearRGB(0.0, 0.0, 0.0);
	double a[4] = {400.0, 100.0,1,1};
	double b[4] = {500.0, 500.0,0,1};
	double c[4] = {30.0, 30.0,0,0};
	// double b[4] = {400.0, 100.0,1,1};// 1.0, 0.0, 0.0};
	// double c[4] = {500.0, 500.0,0,1};// 0.0, 1.0, 0.0};
	// double a[4] = {30.0, 30.0,0,0};// 0.0, 0.0, 1.0};
	double unif[3] = {1,1,1};	
	triRender(&sha,unif,tex,a,b,c);
}

/* This function allows the user to change colors by pressing certain keys*/
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown) {
    if(key==257){
		if (texture.filtering == texLINEAR)
			texSetFiltering(&texture, texNEAREST);
		else
			texSetFiltering(&texture, texLINEAR);
		display();
	}
}

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if(texInitializeFile(&texture,"Noether_retusche_nachcoloriert.jpg")!=0)
			return 2;
		else{
			sha.unifDim = 3;
			sha.attrDim = 2 + 2;
			sha.texNum = 1;
			display();
			pixSetKeyUpHandler(handleKeyUp);
			pixRun();
			texDestroy(&texture);
			return 0;
		}
	}	
}