//Harry Tian

/* On macOS, compile with...
    clang 041mainInterpolating.c 000pixel.o -lglfw -framework OpenGL
*/

#include <stdio.h>
#include "030vector.c" 
#include "030matrix.c"
#include "040texture.c"
#include "040triangle.c"

texTexture texTexTexture;
texTexture *tex = &texTexTexture;

void display(void){
	pixClearRGB(0.0, 0.0, 0.0);
	double white[3] = {1,1,1};
	double t_a[2] = {30,30};
	double t_b[2] = {400,120};
	double t_c[2] = {500,500};
	double alpha[2] = {0.0,0.0};
	double beta[2] = {1.0,1.0};
	double gamma[2] = {0.0,1.0};	
	triRender(t_a,t_b,t_c,white,tex,alpha,beta,gamma);
}

/* This function allows the user to change colors by pressing certain keys*/
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown) {
    if(key==257){
		if (tex->filtering == texLINEAR)
			texSetFiltering(tex, texNEAREST);
		else
			texSetFiltering(tex, texLINEAR);
		display();
	}
}

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		if(texInitializeFile(tex,"unnamed.jpg")!=0)
			return 2;
		else{
			display();
			pixSetKeyUpHandler(handleKeyUp);
			pixRun();
			texDestroy(tex);
			return 0;
		}
	}	
}