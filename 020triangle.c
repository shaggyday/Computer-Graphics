//Harry Tian

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

//Returns the Y/x1 coordinate of a point 
double YCoordOf(int x0, double slope, double a0, double a1){
    return slope*x0 - slope*a0 + a1;
}

void triRender(const double a[2], const double b[2], const double c[2], 
		const double rgb[3]){
    //Organize the 6 points of the traingle in the correct order
    double points[6];
    if (b[1] > a[1] && b[1]> c[1]){
        points[0] = b[0];
        points[1] = b[1];
        points[2] = c[0];
        points[3] = c[1];
        points[4] = a[0];
        points[5] = a[1];
    }
    else if(a[1] > b[1] && a[1] > c[1]){
        points[0] = a[0];
        points[1] = a[1];
        points[2] = b[0];
        points[3] = b[1];
        points[4] = c[0];
        points[5] = c[1];
    }
    else{
        points[0] = c[0];
        points[1] = c[1];
        points[2] = a[0];
        points[3] = a[1];
        points[4] = b[0];
        points[5] = b[1];
    }
    
    //deals with undefined slopes
    double slopeLeftRight, slopeRightTop, slopeLeftTop;
    if (points[4] == points[2]){
        slopeLeftRight = 0;
    }
    else{
        slopeLeftRight = (points[5]-points[3])/(points[4]-points[2]);
    }
    if (points[4] == points[0]){
        slopeRightTop = 0;
    }
    else{
        slopeRightTop = (points[5]-points[1])/(points[4]-points[0]);
    }
    if (points[2] == points[0]){
        slopeLeftTop = 0;
    }
    else{
    slopeLeftTop = (points[3]-points[1])/(points[2]-points[0]);
    }
    
    int x0, x1;
    if (points[0] <= points[4]) {
    //Case 1: c is left of b, top slope changes
        for (x0=(int)ceil(points[2]); x0<=(int)floor(points[0]);x0=x0+1) {
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[2],points[3]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[2],points[3]);x1=x1+1){
                pixSetRGB(x0,x1,rgb[0],rgb[1],rgb[2]);
            }
        }
        for (x0=(int)floor(points[0])+1;x0<=(int)floor(points[4]);x0=x0+1){
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[2],points[3]);x1<=(int)YCoordOf(x0,slopeRightTop,points[0],points[1]);x1=x1+1){
                pixSetRGB(x0,x1,rgb[0],rgb[1],rgb[2]);
            }

        }
    }
    else{
    //Case 2: c is right of b, bottom slope changes
        for (x0=(int)ceil(points[2]); x0<=(int)floor(points[4]);x0=x0+1) {
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[2],points[3]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[2],points[3]);x1=x1+1){
                pixSetRGB(x0,x1,rgb[0],rgb[1],rgb[2]);
            }            
        }
        for (x0=(int)floor(points[4])+1;x0<=(int)floor(points[0]);x0=x0+1){
            for(x1=(int)YCoordOf(x0,slopeRightTop,points[4],points[5]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[0],points[1]);x1=x1+1){
                pixSetRGB(x0,x1,rgb[0],rgb[1],rgb[2]);
            }
        }         
    }
}