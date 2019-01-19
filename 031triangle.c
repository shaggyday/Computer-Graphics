//Harry Tian

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

//Returns the Y/x1 coordinate of a point 
double YCoordOf(int x0, double slope, double a0, double a1){
    return slope*x0 - slope*a0 + a1;
}

void checkChi(double chi[3]){
    int i;
    for (i = 0;i <=3; i = i+1){
        if(chi[i] < 0){
            chi[i] = 0.0;
        }
    }
}

void fillArray(double points[],const double a0, const double a1, 
const double b0, const double b1, const double c0, const double c1){
        points[4] = c0;
        points[5] = c1;
        points[0] = a0;
        points[1] = a1;
        points[2] = b0;
        points[3] = b1; 
}

void interpolate(double points[6],int x0,int x1, double MInv[2][2],const double alpha[3],
double betaMinusAlpha[3],double gammaMinusAlpha[3],const double rgb[3]){
    double chi[3],pTimesBetaMinusAlpha[3],qTimesGammaMinusAlpha[3];
    double pq[2],v[2];
    v[0] = x0-points[0];
    v[1] = x1-points[1];
    mat221Multiply(MInv,v,pq);
    vecScale(3,pq[0],betaMinusAlpha,pTimesBetaMinusAlpha);
    vecScale(3,pq[1],gammaMinusAlpha,qTimesGammaMinusAlpha);
    //vecAdd3(3,alpha,pTimesBetaMinusAlpha,qTimesGammaMinusAlpha,chi);
    int i;
	for(i = 0;i <=2;i = i+1){
        chi[i] = alpha[i] + pTimesBetaMinusAlpha[i] + qTimesGammaMinusAlpha[i];
    }
    checkChi(chi);
    pixSetRGB(x0,x1,chi[0]*rgb[0],chi[1]*rgb[1],chi[2]*rgb[2]);
}

void triRender(const double a[2], const double b[2], const double c[2], 
		const double rgb[3], const double alpha[3], const double beta[3], 
		const double gamma[3]){
    //Organize the 6 points of the traingle in the correct order
    double points[6];
    if (b[1] > a[1] && b[1]> c[1]){
        fillArray(points,c[0],c[1],a[0],a[1],b[0],b[1]);
    }
    else if(a[1] > b[1] && a[1] > c[1]){
        fillArray(points,b[0],b[1],c[0],c[1],a[0],a[1]);
    }
    else{
        fillArray(points,a[0],a[1],b[0],b[1],c[0],c[1]);
    }
    
    //deals with undefined slopes
    double slopeLeftRight, slopeRightTop, slopeLeftTop;
    if (points[0] == points[2]){
        slopeLeftRight = 0;
    }
    else{
        slopeLeftRight = (points[1]-points[3])/(points[0]-points[2]);
    }
    if (points[4] == points[2]){
        slopeRightTop = 0;
    }
    else{
        slopeRightTop = (points[5]-points[3])/(points[4]-points[2]);
    }
    if (points[4] == points[0]){
        slopeLeftTop = 0;
    }
    else{
    slopeLeftTop = (points[5]-points[1])/(points[4]-points[0]);
    }
    
    //Matrix for interpolating
    double M[2][2] = {
        {points[2]-points[0],points[4]-points[0]},
        {points[3]-points[1],points[5]-points[1]}};
    double MInv[2][2];
    double detM = mat22Invert(M,MInv);
	int i, j;
    if(detM <= 0){
        return;
    }

    double betaMinusAlpha[3],gammaMinusAlpha[3];
    int x0, x1;
    vecSubtract(3,beta,alpha,betaMinusAlpha);
    vecSubtract(3,gamma,alpha,gammaMinusAlpha);
    if (points[4] <= points[2]) {
    //Case 1: c is left of b, top slope changes
        for (x0=(int)ceil(points[0]); x0<=(int)floor(points[4]);x0=x0+1) {
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[0],points[1]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[0],points[1]);x1=x1+1){
                interpolate(points,x0,x1,MInv,alpha,betaMinusAlpha,gammaMinusAlpha,rgb);
            }
        }
        for (x0=(int)floor(points[4])+1;x0<=(int)floor(points[2]);x0=x0+1){
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[0],points[1]);x1<=(int)YCoordOf(x0,slopeRightTop,points[4],points[5]);x1=x1+1){
                interpolate(points,x0,x1,MInv,alpha,betaMinusAlpha,gammaMinusAlpha,rgb);
            }
        }
    }
    else{
    //Case 2: c is right of b, bottom slope changes
        for (x0=(int)ceil(points[0]); x0<=(int)floor(points[2]);x0=x0+1) {
            for(x1=(int)YCoordOf(x0,slopeLeftRight,points[0],points[1]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[0],points[1]);x1=x1+1){
                interpolate(points,x0,x1,MInv,alpha,betaMinusAlpha,gammaMinusAlpha,rgb);
                printf("%d,%d\n",x0,x1);
            }            
        }
        for (x0=(int)floor(points[2])+1;x0<=(int)floor(points[4]);x0=x0+1){
            for(x1=(int)YCoordOf(x0,slopeRightTop,points[2],points[3]);x1<=(int)YCoordOf(x0,slopeLeftTop,points[4],points[5]);x1=x1+1){
                interpolate(points,x0,x1,MInv,alpha,betaMinusAlpha,gammaMinusAlpha,rgb);
                printf("%d,%d\n",x0,x1);
            }
        }         
    }
}