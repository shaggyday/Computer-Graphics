//Harry Tian
#include "020triangle.c"

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(1024, 1024, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);
		double white[3] = {1.0,1.0,1.0};
		double magenta[3] = {1.0,0.0,1.0};
		double yellow[3] = {1.0,1.0,0};
		double cyan[3] = {0.0,1.0,1.0};
		double red[3] = {1.0,0.0,0.0};
		double green[3] = {0.0,1.0,0.0};
		//6 different counterclockwise cases
		double t1_a[2] = {100,850};
		double t1_b[2] = {400,800};
		double t1_c[2] = {250,950};
		double t2_a[2] = {400,600};
		double t2_b[2] = {250,750};
		double t2_c[2] = {100,650};
		double t3_a[2] = {250,550};
		double t3_b[2] = {100,450};
		double t3_c[2] = {400,400};
		double t4_a[2] = {600,850};
		double t4_b[2] = {800,800};
		double t4_c[2] = {900,950};
		double t5_a[2] = {800,600};
		double t5_b[2] = {900,750};
		double t5_c[2] = {600,650};
		double t6_a[2] = {900,550};
		double t6_b[2] = {600,450};
		double t6_c[2] = {800,400};
		triRender(t1_a,t1_b,t1_c,white);
		triRender(t2_a,t2_b,t2_c,magenta);
		triRender(t3_a,t3_b,t3_c,yellow);
		triRender(t4_a,t4_b,t4_c,cyan);
		triRender(t5_a,t5_b,t5_c,red);
		triRender(t6_a,t6_b,t6_c,green);
		// //8 other edge cases with slopes = 0 or undefined slopes
		double t7_a[2] = {100,320};
		double t7_b[2] = {400,320};
		double t7_c[2] = {200,400};
		double t8_a[2] = {100,280};
		double t8_b[2] = {200,200};
		double t8_c[2] = {400,280};
		double t9_a[2] = {100,100};
		double t9_b[2] = {200,20};
		double t9_c[2] = {200,180};
		double t10_a[2] = {300,20};
		double t10_b[2] = {400,100};
		double t10_c[2] = {300,180};
		double t11_a[2] = {600,220};
		double t11_b[2] = {700,220};
		double t11_c[2] = {700,380};
		double t12_a[2] = {800,220};
		double t12_b[2] = {900,220};
		double t12_c[2] = {800,380};
		double t13_a[2] = {600,180};
		double t13_b[2] = {700,20};
		double t13_c[2] = {700,180};
		double t14_a[2] = {800,20};
		double t14_b[2] = {900,180};
		double t14_c[2] = {800,180};
		triRender(t7_a,t7_b,t7_c,white);
		triRender(t8_a,t8_b,t8_c,white);
		triRender(t9_a,t9_b,t9_c,white);
		triRender(t10_a,t10_b,t10_c,white);
		triRender(t11_a,t11_b,t11_c,white);
		triRender(t12_a,t12_b,t12_c,white);
		triRender(t13_a,t13_b,t13_c,white);
		triRender(t14_a,t14_b,t14_c,white);
		pixRun();
		return 0;
	}
}