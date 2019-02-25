//Harry Tian

/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(const GLdouble m[2][2]) {
	for (int i = 0; i < 2; i += 1) {
		for (int j = 0; j < 2; j += 1){
			printf("%f    \n", m[i][j]);
		}
		printf("\n");
	}
}

void mat33Print(const GLdouble m[3][3]) {
    for (int i = 0; i < 3; i += 1) {
        for (int j = 0; j < 3; j += 1){
            printf("%f    \n", m[i][j]);
        }
        printf("\n");
    }
}

void mat44Print(const GLdouble m[4][4]) {
    for (int i = 0; i < 4; i += 1) {
        for (int j = 0; j < 4; j += 1){
            printf("%f    \n", m[i][j]);
        }
        printf("\n");
    }
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. The output 
CANNOT safely alias the input. */
GLdouble mat22Invert(const GLdouble m[2][2], GLdouble mInv[2][2]) {
	GLdouble detM = m[0][0]*m[1][1] - m[0][1]*m[1][0];
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
void mat221Multiply(const GLdouble m[2][2], const GLdouble v[2], 
		GLdouble mTimesV[2]) {
	mTimesV[0] = m[0][0]*v[0] + m[0][1]*v[1];
	mTimesV[1] = m[1][0]*v[0] + m[1][1]*v[1];
}

/* Fills the matrix m from its two columns. The output CANNOT safely alias the 
input. */
void mat22Columns(const GLdouble col0[2], const GLdouble col1[2], GLdouble m[2][2]) {
	m[0][0] = col0[0];
	m[1][0] = col0[1];
	m[0][1] = col1[0];
	m[1][1] = col1[1];
}

/* The theta parameter is an angle in radians. Sets the matrix m to the 
rotation matrix corresponding to counterclockwise rotation of the plane through 
the angle theta. */
void mat22Rotation(GLdouble theta, GLdouble m[2][2]){
	m[0][0] = cos(theta);
	m[1][0] = sin(theta);
	m[0][1] = -m[1][0];
	m[1][1] = m[0][0];
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. The output CANNOT safely 
alias the input. */
void mat333Multiply(const GLdouble m[3][3], const GLdouble n[3][3], 
		GLdouble mTimesN[3][3]){
	for (int i = 0;i < 3;i += 1)
		for(int j = 0;j < 3;j += 1)
			mTimesN[i][j] = m[i][0]*n[0][j] + m[i][1]*n[1][j] + m[i][2]*n[2][j];
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. The output CANNOT safely 
alias the input. */
void mat331Multiply(const GLdouble m[3][3], const GLdouble v[3], 
		GLdouble mTimesV[3]){
	for (int i = 0;i < 3;i += 1)
		mTimesV[i] = m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2];
}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector t. */
void mat33Isometry(GLdouble theta, const GLdouble t[2], GLdouble isom[3][3]){
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


void mat33Add(const GLdouble m[3][3], const GLdouble n[3][3], const GLdouble h[3][3], GLdouble mPlusNPlusH[3][3]){
	for (int i = 0;i < 3;i += 1)
		for(int j = 0;j < 3;j += 1){
			mPlusNPlusH[i][j] = m[i][j]+n[i][j]+h[i][j];
		}
}

void mat33Scale(const GLdouble m[3][3], const GLdouble k, GLdouble mScaled[3][3]){
    for (int i = 0;i < 3;i += 1)
        for(int j = 0;j < 3;j += 1){
            mScaled[i][j] = k*m[i][j];
        }
}

void mat33FillByCol(const GLdouble u[3], const GLdouble v[3], const GLdouble w[3], GLdouble M[3][3]){
    for (int i = 0;i < 3;i += 1){
        M[i][0] = u[i];
        M[i][1] = v[i];
        M[i][2] = w[i];
    }
}

void mat33FillByRow(const GLdouble u[3], const GLdouble v[3], const GLdouble w[3], GLdouble M[3][3]){
    for (int i = 0;i < 3;i += 1){
        M[0][i] = u[i];
        M[1][i] = v[i];
        M[2][i] = w[i];
    }
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the
rotation matrix for the rotation about that axis through that angle. */
void mat33AngleAxisRotation(GLdouble theta, const GLdouble axis[3],
							GLdouble rot[3][3]){
	GLdouble U[3][3] = {
			{0,-axis[2],axis[1]},
			{axis[2],0,-axis[0]},
			{-axis[1],axis[0],0}};
	GLdouble U2[3][3],UPrime[3][3],U2Prime[3][3];
	GLdouble sinTheta = sin(theta);
	GLdouble cosTheta = 1-cos(theta);
	mat333Multiply(U,U,U2);
	mat33Scale(U,sinTheta,UPrime);
	mat33Scale(U2,cosTheta,U2Prime);
	GLdouble I3[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
	mat33Add(I3,UPrime,U2Prime,rot);
}

/* Given two length-1 3D vectors u, v that are perpendicular to each other.
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(const GLdouble u[3], const GLdouble v[3],
						const GLdouble a[3], const GLdouble b[3], GLdouble rot[3][3]){
    GLdouble w[3],c[3];
    GLdouble R[3][3],S[3][3];
    vec3Cross(u,v,w);
    vec3Cross(a,b,c);
    mat33FillByCol(a,b,c,S);
    mat33FillByRow(u,v,w,R);
    mat333Multiply(S,R,rot);
}

/* Multiplies m by n, placing the answer in mTimesN. The output CANNOT safely
alias the input. */
void mat444Multiply(const GLdouble m[4][4], const GLdouble n[4][4],
					GLdouble mTimesN[4][4]){
    for (int i = 0;i < 4;i += 1)
        for(int j = 0;j < 4;j += 1)
            mTimesN[i][j] = m[i][0]*n[0][j] + m[i][1]*n[1][j] + m[i][2]*n[2][j] + m[i][3]*n[3][j];
}

/* Multiplies m by v, placing the answer in mTimesV. The output CANNOT safely
alias the input. */
void mat441Multiply(const GLdouble m[4][4], const GLdouble v[4],
					GLdouble mTimesV[4]){
    for (int i = 0;i < 4;i += 1)
        mTimesV[i] = m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2] + m[i][3]*v[3];
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix
representing the rotation followed in time by the translation. */
void mat44Isometry(const GLdouble rot[3][3], const GLdouble trans[3],
				   GLdouble isom[4][4]){
    for (int i = 0;i < 3;i += 1) {
        isom[i][3] = trans[i];
        isom[3][i] = 0;
        for (int j = 0; j < 3; j += 1)
            isom[i][j] = rot[i][j];
    }
    isom[3][3] = 1;
}

/* Multiplies the transpose of the 3x3 matrix m by the 3x1 matrix v. To 
clarify, in math notation it computes M^T v. The output CANNOT safely alias the 
input. */
void mat331TransposeMultiply(const GLdouble m[3][3], const GLdouble v[3], 
		GLdouble mTTimesV[3]){
	for (int i = 0;i < 3;i += 1)
		mTTimesV[i] = m[0][i]*v[0] + m[1][i]*v[1] + m[2][i]*v[2];
}

void mat333TranposeMatrix(const GLdouble m[3][3], GLdouble mInv[3][3]){
    for (int i = 0;i < 3;i += 1)
        for(int j = 0;j < 3;j += 1)
        	mInv[i][j] = m[j][i];
}

/* Sets its argument to the 4x4 zero matrix (which consists entirely of 0s). */
void mat44Zero(GLdouble m[4][4]){
	for(int i = 0;i < 4;i += 1)
		for(int j = 0;j < 4;j += 1)
			m[i][j] = 0;
}

/* Builds a 4x4 matrix for a viewport with lower left (0, 0) and upper right
(width, height). This matrix maps a projected viewing volume
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w] x [0, h] x [0, 1] (each interval
in that order). */
void mat44Viewport(GLdouble width, GLdouble height, GLdouble view[4][4]){
    mat44Zero(view);
    view[0][0] = width / 2.0;
    view[0][3] = width / 2.0;
    view[1][1] = height / 2.0;
    view[1][3] = height / 2.0;
    view[2][2] = 0.5;
    view[2][3] = 0.5;
    view[3][3] = 1.0;
}

/* Inverse to mat44Viewport. */
void mat44InverseViewport(GLdouble width, GLdouble height, GLdouble view[4][4]){
    mat44Zero(view);
    view[0][0] = 2.0 / width;
    view[1][1] = 2.0 / height;
    view[2][2] = 2.0;
    view[3][3] = 1.0;
    view[0][3] = -1.0;
    view[1][3] = -1.0;
    view[2][3] = -1.0;
}