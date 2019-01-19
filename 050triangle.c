//Harry Tian

#include <stdio.h>
#include <math.h>
#include "000pixel.h"

/* Returns the Y coordinate of a point  */
int YCoordOf(int x0, double slope, double a0, double a1){
    return (int)(slope*x0 - slope*a0 + a1);
}

/* Helper function to rasterize */
void aggregateArray(int attrDim, double points[],const double a[], const double b[],const double c[]){
    int i;
    for(i = 0;i < attrDim; i = i + 1){
        points[i] = a[i];
        points[i+attrDim] = b[i];
        points[i+2*attrDim] = c[i];
    } 
}

/* Helper function to create three new vertix arrays */
void distributeArray(int attrDim, double points[], double aPrime[],  double bPrime[], double cPrime[]){
    int i;
    for(i = 0;i < attrDim; i = i + 1){
        aPrime[i] = points[i];
        bPrime[i] = points[i+attrDim];
        cPrime[i] = points[i+2*attrDim];
    } 
}

/* Sort the three vertices in the correct order */
void rasterize(int attrDim, double points[], const double a[], const double b[],const double c[]){
    if (b[1] > a[1] && b[1]> c[1]){
        aggregateArray(attrDim,points,c,a,b);
    }
    else if(a[1] > b[1] && a[1] > c[1]){
        aggregateArray(attrDim,points,b,c,a);
    }
    else{
        aggregateArray(attrDim,points,a,b,c);
    }
}

double calSlope(double a[],  double b[]){
    if (a[0] == b[0]){
        return 0;
    }
    else{
        return (a[1]-b[1])/(a[0]-b[0]);
    }
}

/* The interpolation steps that happen outside the for loops */
int prepareInterpolate(int attrDim, double MInv[2][2], double aPrime[],  double bPrime[], double cPrime[],
double bMinusA[],double cMinusA[]){
    double M[2][2] = {
        {bPrime[0]-aPrime[0],cPrime[0]-aPrime[0]},
        {bPrime[1]-aPrime[1],cPrime[1]-aPrime[1]}};
    double detM = mat22Invert(M,MInv);
    if(detM <= 0){
        return 1;
    }
    vecSubtract(attrDim,bPrime,aPrime,bMinusA);
    vecSubtract(attrDim,cPrime,aPrime,cMinusA);
    return 0;
}

/* The interpolation steps that happen inside the for loops */
void interpolate(int attrDim,double points[],double x[], double MInv[2][2],const double a[],
double bMinusA[],double cMinusA[]){
    double pTimesBMinusA[attrDim],qTimesCMinusA[attrDim];
    double pq[2],v[2];
    v[0] = x[0]-points[0];
    v[1] = x[1]-points[1];
    mat221Multiply(MInv,v,pq);
    vecScale(attrDim,pq[0],bMinusA,pTimesBMinusA);
    vecScale(attrDim,pq[1],cMinusA,qTimesCMinusA);
    int i;
	for(i = 2;i <attrDim;i = i+1){
        x[i] = a[i] + pTimesBMinusA[i] + qTimesCMinusA[i];
    }
}

/* Assumes that the 0th and 1th elements of a, b, c are the 'x' and 'y' 
coordinates of the vertices, respectively (used in rasterization, and to 
interpolate the other elements of a, b, c). */
void triRender(const shaShading *sha, const double unif[], 
		const texTexture *tex[], const double a[], const double b[], 
		const double c[]){
    //Rasterize
    double points[sha->attrDim*3];
    rasterize(sha->attrDim,points,a,b,c);
    double aPrime[sha->attrDim],bPrime[sha->attrDim],cPrime[sha->attrDim];
    distributeArray(sha->attrDim,points,aPrime,bPrime,cPrime);

    //Deal with slopes
    double slopeLeftRight = calSlope(aPrime,bPrime);
    double slopeRightTop = calSlope(cPrime,bPrime);
    double slopeLeftTop = calSlope(aPrime,cPrime);

    //Preparing to interpolate
    double bMinusA[sha->attrDim],cMinusA[sha->attrDim];
    double MInv[2][2];
    if(prepareInterpolate(sha->attrDim,MInv,aPrime,bPrime,cPrime,bMinusA,cMinusA)!=0)
        return;

    //Actually looping and interpolating 
    double rgb[sha->unifDim];
    double x[sha->attrDim];
    if (cPrime[0] <= bPrime[0]) {
    //Case 1: c is left of b, top slope changes
        for (x[0]=(int)ceil(aPrime[0]); x[0]<=(int)floor(cPrime[0]);x[0]=x[0]+1) {
            for(x[1]=YCoordOf(x[0],slopeLeftRight,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeLeftTop,aPrime[0],aPrime[1]);x[1]=x[1]+1){
                interpolate(sha->attrDim,points,x,MInv,aPrime,bMinusA,cMinusA);
                colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->attrDim,x,rgb);
                pixSetRGB((int)x[0],(int)x[1],rgb[0],rgb[1],rgb[2]);
            }
        }
        for (x[0]=(int)floor(cPrime[0])+1;x[0]<=(int)floor(bPrime[0]);x[0]=x[0]+1){
            for(x[1]=YCoordOf(x[0],slopeLeftRight,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeRightTop,cPrime[0],cPrime[1]);x[1]=x[1]+1){
                interpolate(sha->attrDim,points,x,MInv,aPrime,bMinusA,cMinusA);
                colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->attrDim,x,rgb);
                pixSetRGB((int)x[0],(int)x[1],rgb[0],rgb[1],rgb[2]);
            }
        }
    }
    else{
    //Case 2: c is right of b, bottom slope changes
        for (x[0]=(int)ceil(aPrime[0]); x[0]<=(int)floor(bPrime[0]);x[0]=x[0]+1) {
            for(x[1]=YCoordOf(x[0],slopeLeftRight,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeLeftTop,aPrime[0],aPrime[1]);x[1]=x[1]+1){
                interpolate(sha->attrDim,points,x,MInv,aPrime,bMinusA,cMinusA);
                colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->attrDim,x,rgb);
                pixSetRGB((int)x[0],(int)x[1],rgb[0],rgb[1],rgb[2]);
            }            
        }
        for (x[0]=(int)floor(bPrime[0])+1;x[0]<=(int)floor(cPrime[0]);x[0]=x[0]+1){
            for(x[1]=YCoordOf(x[0],slopeRightTop,bPrime[0],bPrime[1]);x[1]<=YCoordOf(x[0],slopeLeftTop,cPrime[0],cPrime[1]);x[1]=x[1]+1){
                interpolate(sha->attrDim,points,x,MInv,aPrime,bMinusA,cMinusA);
                colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->attrDim,x,rgb);
                pixSetRGB((int)x[0],(int)x[1],rgb[0],rgb[1],rgb[2]);
            }
        }         
    }
}