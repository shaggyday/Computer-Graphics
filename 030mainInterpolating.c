//Harry Tian

/* On macOS, compile with...
    clang 030mainInterpolating.c 000pixel.o -lglfw -framework OpenGL
*/
#include "030vector.c" 
#include "030matrix.c"
#include "030triangle.c"

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);
		double white[3] = {1.0,1.0,1.0};
		double magenta[3] = {1.0,0.0,1.0};
		double yellow[3] = {1.0,1.0,0.0};
		double cyan[3] = {0.0,1.0,1.0};

		double red[3] = {1.0,0.0,0.0};
		double green[3] = {0.0,1.0,0.0};
		double blue[3] = {0.0,0.0,1.0};
		//6 different counterclockwise cases
		double t_a[2] = {30,30};
		double t_b[2] = {400,120};
		double t_c[2] = {500,500};
		triRender(t_a,t_b,t_c,white,blue,red,green);
		pixRun();
		return 0;
	}
}