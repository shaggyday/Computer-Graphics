//Harry Tian

/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const double m[2][2]) {
	int i, j;
	for (i = 0; i < 2; i += 1) {
		for (j = 0; j < 2; j += 1){
			printf("%f    \n", m[i][j]);
		}
		printf("\n");
	}
}

void mat33Print(const double m[3][3]) {
    int i, j;
    for (i = 0; i < 3; i += 1) {
        for (j = 0; j < 3; j += 1){
            printf("%f    \n", m[i][j]);
        }
        printf("\n");
    }
}

void mat44Print(const double m[4][4]) {
    int i, j;
    for (i = 0; i < 4; i += 1) {
        for (j = 0; j < 4; j += 1){
            printf("%f    \n", m[i][j]);
        }
        printf("\n");
    }
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. The output 
CANNOT safely alias the input. */
double mat22Invert(const double m[2][2], double mInv[2][2]) {
	double detM = m[0][0]*m[1][1] - m[0][1]*m[1][0];
	if(m != 0){
		mInv[0][0] = m[1][1]/detM;
		mInv[0][1] = -m[0][1]/detM;
		mInv[1][0] = -m[1][0]/detM;
		mInv[1][1] = m[0][0]/detM;
	}
	return detM;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. 
The output CANNOT safely alias the input. */
void mat221Multiply(const double m[2][2], const double v[2], 
		double mTimesV[2]) {
	mTimesV[0] = m[0][0]*v[0] + m[0][1]*v[1];
	mTimesV[1] = m[1][0]*v[0] + m[1][1]*v[1];
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the 
input. */
void mat22Columns(const double col0[2], const double col1[2], double m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the 
rotation matrix corresponding to counterclockwise rotation of the plane through 
the angle theta. */
void mat22Rotation(double theta, double m[2][2]){
	m[0][0] = cos(theta);
	m[1][0] = sin(theta);
	m[0][1] = -m[1][0];
	m[1][1] = m[0][0];
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely 
alias the input. */
void mat333Multiply(const double m[3][3], const double n[3][3], 
		double mTimesN[3][3]){
	int i,j;
	for (i = 0;i < 3;i = i + 1)
		for(j = 0;j < 3;j = j + 1){
			mTimesN[i][j] = m[i][0]*n[0][j] + m[i][1]*n[1][j] + m[i][2]*n[2][j];
		}
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. The output CANNOT safely 
alias the input. */
void mat331Multiply(const double m[3][3], const double v[3], 
		double mTimesV[3]){
	for (int i = 0;i < 3;i = i + 1)
		mTimesV[i] = m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2];
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(double theta, const double t[2], double isom[3][3]){
	isom[0][0] = cos(theta);
	isom[0][1] = -sin(theta);
	isom[0][2] = t[0];
	isom[1][0] = sin(theta);
	isom[1][1] = cos(theta);
	isom[1][2] = t[1];
	isom[2][0] = 0;
	isom[2][1] = 0;
	isom[2][2] = 1;
}


void mat33Add(const double m[3][3], const double n[3][3], const double h[3][3], double mPlusNPlusH[3][3]){
	int i,j;
	for (i = 0;i < 3;i = i + 1)
		for(j = 0;j < 3;j = j + 1){
			mPlusNPlusH[i][j] = m[i][j]+n[i][j]+h[i][j];
		}
}

void mat33Scale(const double m[3][3], const double k, double mScaled[3][3]){
    int i,j;
    for (i = 0;i < 3;i = i + 1)
        for(j = 0;j < 3;j = j + 1){
            mScaled[i][j] = k*m[i][j];
        }
}

void mat33FillByCol(const double u[3], const double v[3], const double w[3], double M[3][3]){
    int i;
    for (i = 0;i < 3;i = i + 1){
        M[i][0] = u[i];
        M[i][1] = v[i];
        M[i][2] = w[i];
    }
}

void mat33FillByRow(const double u[3], const double v[3], const double w[3], double M[3][3]){
    int i;
    for (i = 0;i < 3;i = i + 1){
        M[0][i] = u[i];
        M[1][i] = v[i];
        M[2][i] = w[i];
    }
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the
rotation matrix for the rotation about that axis through that angle. */
void mat33AngleAxisRotation(double theta, const double axis[3],
							double rot[3][3]){
	double U[3][3] = {
			{0,-axis[2],axis[1]},
			{axis[2],0,-axis[0]},
			{-axis[1],axis[0],0}};
	double U2[3][3],UPrime[3][3],U2Prime[3][3];
	double sinTheta = sin(theta);
	double cosTheta = 1-cos(theta);
	mat333Multiply(U,U,U2);
	mat33Scale(U,sinTheta,UPrime);
	mat33Scale(U2,cosTheta,U2Prime);
	double I3[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	mat33Add(I3,UPrime,U2Prime,rot);
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other.
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(const double u[3], const double v[3],
						const double a[3], const double b[3], double rot[3][3]){
    double w[3],c[3];
    double R[3][3],S[3][3];
    vec3Cross(u,v,w);
    vec3Cross(a,b,c);
    mat33FillByCol(u,v,w,R);
    mat33FillByRow(a,b,c,S);
    mat333Multiply(S,R,rot);
}

/* Multiplies m by n, placing the answer in mTimesN. The output CANNOT safely
alias the input. */
void mat444Multiply(const double m[4][4], const double n[4][4],
					double mTimesN[4][4]){
    int i,j;
    for (i = 0;i < 4;i = i + 1)
        for(j = 0;j < 4;j = j + 1){
            mTimesN[i][j] = m[i][0]*n[0][j] + m[i][1]*n[1][j] + m[i][2]*n[2][j] + m[i][3]*n[3][j];
        }
}

/* Multiplies m by v, placing the answer in mTimesV. The output CANNOT safely
alias the input. */
void mat441Multiply(const double m[4][4], const double v[4],
					double mTimesV[4]){
    for (int i = 0;i < 4;i += 1)
        for (int j = 0;j < 4;j += 1)
            mTimesV[i] += m[i][j]*v[j];
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix
representing the rotation followed in time by the translation. */
void mat44Isometry(const double rot[3][3], const double trans[3],
				   double isom[4][4]){
    int i,j;
    for (i = 0;i < 3;i = i + 1) {
        isom[i][3] = trans[i];
        isom[3][i] = 0;
        for (j = 0; j < 3; j = j + 1)
            isom[i][j] = rot[i][j];
    }
    isom[3][3] = 1;
}

/* Multiplies the transpose of the 3x3 matrix m by the 3x1 matrix v. To 
clarify, in math notation it computes M^T v. The output CANNOT safely alias the 
input. */
void mat331TransposeMultiply(const double m[3][3], const double v[3], 
		double mTTimesV[3]){
	for (int i = 0;i < 3;i = i + 1)
		mTTimesV[i] = m[0][i]*v[0] + m[1][i]*v[1] + m[2][i]*v[2];
}

/* Sets its argument to the 4x4 zero matrix (which consists entirely of 0s). */
void mat44Zero(double m[4][4]){
	for(int i = 0;i < 4;i += 1)
		for(int j = 0;j < 4;j += 1)
			m[i][j] = 0;
}
