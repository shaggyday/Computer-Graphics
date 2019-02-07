//Harry Tian

/* Returns the Y coordinate of a point  */
int YCoordOf(int x0, double slope, double a0, double a1){
    return (int)(slope*x0 - slope*a0 + a1);
}

int XCoordOf(int x1, double slope, double a0, double a1){
    return (int)((x1-a1)/slope + a0);
}

/* Sort the three vertices in the correct order */
void rasterize(int varyDim, const double a[], const double b[], const double c[],double aPrime[],double bPrime[],double cPrime[]){
    if((c[0]<a[0] && c[0]<b[0]) || (a[0]==b[0]&&c[0]<a[0]) || (b[0]==c[0]&&b[0]<a[0])){
        vecCopy(varyDim,c,aPrime);
        vecCopy(varyDim,a,bPrime);
        vecCopy(varyDim,b,cPrime);
    }
    else if((b[0]<a[0] && b[0]<c[0]) || (a[0]==b[0] && c[0]>a[0]) || (a[0]==c[0] && b[0]<a[0])) {
        vecCopy(varyDim, b, aPrime);
        vecCopy(varyDim, c, bPrime);
        vecCopy(varyDim, a, cPrime);
    }
    else{
        vecCopy(varyDim, a, aPrime);
        vecCopy(varyDim, b, bPrime);
        vecCopy(varyDim, c, cPrime);
    }
}

double calSlope(double a[],  double b[]){
    if (a[0] == b[0] || a[1] == b[1]){
        return 0;
    }
    else{
        return (a[1]-b[1])/(a[0]-b[0]);
    }
}

/* The interpolation steps that happen outside the for loops */
int prepareInterpolate(int varyDim, double MInv[2][2], double aPrime[],  double bPrime[], double cPrime[],
double bMinusA[],double cMinusA[]){
    double M[2][2] = {
        {bPrime[0]-aPrime[0],cPrime[0]-aPrime[0]},
        {bPrime[1]-aPrime[1],cPrime[1]-aPrime[1]}};
    double detM = mat22Invert(M,MInv);
    if(detM <= 0)
        return 1;
    else{
        vecSubtract(varyDim,bPrime,aPrime,bMinusA);
        vecSubtract(varyDim,cPrime,aPrime,cMinusA);
        return 0;
    }
}

/* The interpolation steps that happen inside the for loops */
void interpolate(int varyDim,double x[], double MInv[2][2],const double a[],
double bMinusA[],double cMinusA[]){
    double pTimesBMinusA[varyDim],qTimesCMinusA[varyDim];
    double pq[2],v[2];
    v[0] = x[0]-a[0];
    v[1] = x[1]-a[1];
    mat221Multiply(MInv,v,pq);
    vecScale(varyDim,pq[0],bMinusA,pTimesBMinusA);
    vecScale(varyDim,pq[1],cMinusA,qTimesCMinusA);
    for(int i = 2;i < varyDim; i += 1)
        x[i] = a[i] + pTimesBMinusA[i] + qTimesCMinusA[i];
}


/* Assumes that the 0th and 1th elements of a, b, c are the 'x' and 'y'
coordinates of the vertices, respectively (used in rasterization, and to
interpolate the other elements of a, b, c). */
void triRender(const shaShading *sha, depthBuffer *buf, const double unif[],
               const texTexture *tex[], const double a[], const double b[],
               const double c[]){
    //rasterize
    double aPrime[sha->varyDim],bPrime[sha->varyDim],cPrime[sha->varyDim];
    rasterize(sha->varyDim,a,b,c,aPrime,bPrime,cPrime);

    //Deal with slopes
    double slopeAB = calSlope(aPrime,bPrime);
    double slopeBC = calSlope(cPrime,bPrime);
    double slopeAC = calSlope(aPrime,cPrime);

    //Preparing to interpolate
    double bMinusA[sha->varyDim],cMinusA[sha->varyDim];
    double MInv[2][2];
    if(prepareInterpolate(sha->varyDim,MInv,aPrime,bPrime,cPrime,bMinusA,cMinusA)!=0){
        return;
    }

    //Actually looping and interpolating
    double rgbd[4];
    double x[sha->varyDim];
    if (cPrime[0] < bPrime[0]) {
    //Case 1: c is left of b, top slope changes
        for (x[0]=(int)ceil(aPrime[0]); x[0]<=(int)floor(cPrime[0]);x[0]=x[0]+1) {
            for(x[1]=YCoordOf(x[0],slopeAB,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeAC,aPrime[0],aPrime[1]);x[1]=x[1]+1){
                interpolate(sha->varyDim,x,MInv,aPrime,bMinusA,cMinusA);
                sha->colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->varyDim,x,rgbd);
                if (depthGetDepth(buf,(int)x[0],(int)x[1]) >= rgbd[3]) {
                    depthSetDepth(buf,(int)x[0],(int)x[1],rgbd[3]);
                    pixSetRGB((int)x[0], (int)x[1], rgbd[0], rgbd[1], rgbd[2]);
                }
            }
        }
        for (x[0]=(int)floor(cPrime[0])+1;x[0]<=(int)floor(bPrime[0]);x[0]=x[0]+1){
            for(x[1]=YCoordOf(x[0],slopeAB,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeBC,cPrime[0],cPrime[1]);x[1]=x[1]+1){
                interpolate(sha->varyDim,x,MInv,aPrime,bMinusA,cMinusA);
                sha->colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->varyDim,x,rgbd);
                if (depthGetDepth(buf,(int)x[0],(int)x[1]) >= rgbd[3]) {
                    depthSetDepth(buf,(int)x[0],(int)x[1],rgbd[3]);
                    pixSetRGB((int)x[0], (int)x[1], rgbd[0], rgbd[1], rgbd[2]);
                }
            }
        }
    }
    else{
    //Case 2: c is right of b, bottom slope changes
        for (x[0]=(int)ceil(aPrime[0]); x[0]<=(int)floor(bPrime[0]);x[0]=x[0]+1) {
            for(x[1]=YCoordOf(x[0],slopeAB,aPrime[0],aPrime[1]);x[1]<=YCoordOf(x[0],slopeAC,aPrime[0],aPrime[1]);x[1]=x[1]+1){
                interpolate(sha->varyDim,x,MInv,aPrime,bMinusA,cMinusA);
                sha->colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->varyDim,x,rgbd);
                if (depthGetDepth(buf,(int)x[0],(int)x[1]) >= rgbd[3]) {
                    depthSetDepth(buf,(int)x[0],(int)x[1],rgbd[3]);
                    pixSetRGB((int)x[0], (int)x[1], rgbd[0], rgbd[1], rgbd[2]);
                }
            }
        }
        for (x[0]=(int)floor(bPrime[0])+1;x[0]<=(int)floor(cPrime[0]);x[0]=x[0]+1){
            for(x[1]=YCoordOf(x[0],slopeBC,bPrime[0],bPrime[1]);x[1]<=YCoordOf(x[0],slopeAC,cPrime[0],cPrime[1]);x[1]=x[1]+1){
                interpolate(sha->varyDim,x,MInv,aPrime,bMinusA,cMinusA);
                sha->colorPixel(sha->unifDim,unif,sha->texNum,tex,sha->varyDim,x,rgbd);
                if (depthGetDepth(buf,(int)x[0],(int)x[1]) >= rgbd[3]) {
                    depthSetDepth(buf,(int)x[0],(int)x[1],rgbd[3]);
                    pixSetRGB((int)x[0], (int)x[1], rgbd[0], rgbd[1], rgbd[2]);
                }
            }
        }         
    }
}