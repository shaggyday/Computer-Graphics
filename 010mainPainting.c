/* Harry Tian*/

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

double R = 255.0,  G = 255.0, B = 255.0;
int mouseDown = 1;
/* This function is a user interface 'callback'. Once we register it with the 
user interface infrastructure, it is called whenever the user releases a 
keyboard key. For details, see the pixSetKeyUpHandler documentation. */
void handleKeyUp(int key, int shiftIsDown, int controlIsDown,
		int altOptionIsDown, int superCommandIsDown) {
}

/* Translate the keys that the user presses to correct RGB values*/
void convertColorToRGB(int key){
    char color = key;
    switch (color) {
        case 'R':
            R = 255.0;
            G = 0.0;
            B = 0.0;
            break;
        case 'G':
            R = 0.0;
            G = 255.0;
            B = 0.0;
            break;
        case 'B':
            R = 0.0;
            G = 0.0;
            B = 255.0;
            break;
        case 'C':
            R = 0.0;
            G = 255.0;
            B = 255.0;
            break;
        case 'M':
            R = 255.0;
            G = 0;
            B = 255.0;
            break;
        case 'Y':
            R = 255.0;
            G = 255.0;
            B = 0.0;
            break;
        case 'K':
            R = 0.0;
            G = 0.0;
            B = 0.0;
            break;
        case 'W':
            R = 255.0;
            G = 255.0;
            B = 255.0;
        default:
            break;
    }
}

/* This function allows the user to change colors by pressing certain keys*/
void handleKeyDown(int key, int shiftIsDown, int controlIsDown,
                 int altOptionIsDown, int superCommandIsDown) {
    convertColorToRGB(key);
}

/* Similarly, the following callbacks handle some mouse interactions. */
void handleMouseUp(double x, double y, int button, int shiftIsDown, 
		int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
    mouseDown = 1;
}

void handleMouseDown(double x, double y, int button, int shiftIsDown,
                   int controlIsDown, int altOptionIsDown, int superCommandIsDown) {
    if(button == 0)
        mouseDown = 0;
    pixSetRGB(x,y,R,G,B);
}

void handleMouseMove(double x, double y) {
    if(mouseDown == 0)
        pixSetRGB(x,y,R,G,B);
}

void handleMouseScroll(double xOffset, double yOffset) {
}

/* This callback is called once per animation frame. As parameters it receives 
the time for the current frame and the time for the previous frame. Both times 
are measured in seconds since some distant past time. */ 
void handleTimeStep(double oldTime, double newTime) {
}

/* You can also set callbacks for key down, key repeat, and mouse down. See 
000pixel.h for details. */

int main(void) {
	/* Make a 512 x 512 window with the title 'Pixel Graphics'. This function 
	returns 0 if no error occurred. */
	if (pixInitialize(512, 512, "Pixel Graphics") != 0)
		return 1;
	else {
		/* Register the callbacks (defined above) with the user interface, so 
		that they are called as needed during pixRun (invoked below). */
        pixSetKeyDownHandler(handleKeyDown);
        pixSetKeyUpHandler(handleKeyUp);
        pixSetMouseDownHandler(handleMouseDown);
        pixSetMouseMoveHandler(handleMouseMove);
		pixSetMouseUpHandler(handleMouseUp);
		pixSetMouseScrollHandler(handleMouseScroll);
		pixSetTimeStepHandler(handleTimeStep);
		/* Clear the window to black. */
		pixClearRGB(0.0, 0.0, 0.0);
		/* Run the event loop. The callbacks that were registered above are 
		invoked as needed. At the end, the resources supporting the window are 
		deallocated. */
		pixRun();
		return 0;
	}
}


