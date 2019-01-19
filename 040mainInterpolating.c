//Harry Tian

/* On macOS, compile with...
    clang 040mainInterpolating.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include "030vector.c" 
#include "030matrix.c"
#include "040texture.c"
#include "040triangle.c"


texTexture texture;

void texSwitchFiltering(texTexture *tex){
	
	if (tex->filtering == 0){
		
		texSetFiltering(tex,1);
		printf("swap to %d\n",tex->filtering);
	}
	else{
		texSetFiltering(tex,0);
		printf("swap to %d\n",tex->filtering);
	}
}

void display(double t_a[2],double t_b[2],double t_c[2],double rgb[3],double alpha[2],
double beta[2], double gamma[2]){
	
		
		triRender(t_a,t_b,t_c,rgb,tex,alpha,beta,gamma);
		return 0;
}
/* This function allows the user to change colors by pressing certain keys*/
void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown) {
    if(key==257){
			/* Clear the window to black. */
			pixClearRGB(0.0, 0.0, 0.0);
			double white[3] = {1,1,1};
			double t_a[2] = {30,30};
			double t_b[2] = {400,120};
			double t_c[2] = {500,500};
			double alpha[2] = {0.0,0.0};
			double beta[2] = {1.0,1.0};
			double gamma[2] = {0.0,1.0};
			texSwitchFiltering(tex);
			display(t_a,t_b,t_c,white,alpha,beta,gamma);
	}
}

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Clear the window to black. */
		if(texInitializeFile(tex,"unnamed.jpg")!=0)
			return 2;
		else{
		pixClearRGB(0.0, 0.0, 0.0);
		double white[3] = {1,1,1};
		double t_a[2] = {30,30};
		double t_b[2] = {400,120};
		double t_c[2] = {500,500};
		double alpha[2] = {0.0,0.0};
		double beta[2] = {1.0,1.0};
		double gamma[2] = {0.0,1.0};
		pixSetKeyDownHandler(handleKeyDown);
		display(t_a,t_b,t_c,white,alpha,beta,gamma)
		pixRun();
		texDestroy(tex);
		return 0;
		}
	}
}