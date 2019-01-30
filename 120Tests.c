//
// Created by tianh on 2019-01-25.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "120vector.c"
#include "120matrix.c"

int main(void) {
    // Vector Tests
//    double v1[3] = {1,2,3};
    double v2[3] = {4,5,6};
//    double v3[3] = {0,0,0};
//    double v1times2 = vecDot(3,v1,v2);
//    if (v1times2!=32)
//        printf("vecDot fucked up: %f\n",v1times2);
//
//    double v3[2] = {3,4};
//    double v3L = vecLength(2,v3);
//    if(v3L!=5)
//        printf("vecLength fucked up:%f\n",v3L);
//
//    double v3U[2];
//    v3L = vecUnit(2,v3,v3U);
//    if(v3L!=5.00000 || v3U[0]!=0.600000 || v3U[1]!=0.800000) {
//        printf("vecUnit fucked up\n");
//        vecPrint(2, v3U);
//    }
//
//    double v1cross2[3];
//    vec3Cross(v1,v2,v1cross2);
//    if(v1cross2[0]!=-3.00000 || v1cross2[1]!=6.00000 || v1cross2[2]!=-3.00000) {
//        printf("vec3Cross fucked up");
//        vecPrint(3,v1cross2);
//    }
//
//    double vSph[3];
//    vec3Spherical(5,M_PI/2,M_PI,vSph);
//    if(vSph[0]!=-5.00000 || vSph[1]!=0.00000 || vSph[2]!=0.00000){
//        printf("vec3Spherical fucked up");
//        vecPrint(3,vSph);
//    }

    //  Matrix tests
//    double m1[3][3] = {{1,2,3},{1,2,3},{0,0,1}};
//    double m2[3][3] = {{1,2,3},{1,2,3},{0,1,0}};
//    double m3[3][3] = {{1,2,3},{1,2,3},{2,0,0}};
//    double m4[3][3],m5[3][3],m6[3][3][3],m7[3][3][3];
//    mat33Add(m1,m2,m3,m4);
//    mat33Print(m4);
//
//
//    mat33Scale(m1,2,m5);
//    mat33Print(m5);
//
//    mat33FillByCol(v1,v2,v3,m6);
//    mat33FillByRow(v1,v2,v3,m7);
//    mat33Print(m6);
//    mat33Print(m7);

//    double m8[4][4] = {{1,1,2,3},{4,4,5,6},{7,7,8,9},{1,1,2,3}};
//    double m8[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
//    double m9[3][3] = {{7,8,9},{4,5,6},{1,2,3}};
//    double m10[3][3];
//    mat333Multiply(m8,m9,m10);
//    mat33Print(m10);

//    double v4[4] = {1,2,3,4};
//    double v5[4];
//    mat441Multiply(m8,v4,v5);
//    vecPrint(4,v5);
//    double isom[4][4];
//    mat44Isometry(m9,v2,isom);
//    mat44Print(isom);
//    double m11[3][3];
//    double v6[3] = {0,0,1};
//    double t = 0;
//    mat33AngleAxisRotation(t,v6,m11);
//    mat33Print(m11);

    return 0;
}