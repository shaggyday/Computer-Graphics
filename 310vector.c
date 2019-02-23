//Harry Tian


/*** In general dimensions ***/

void vecClear3(int dim, GLdouble v[] , GLdouble w[], GLdouble u[]){
    for (int i = 0; i < dim; i += 1) {
        v[i] = 0.0;
        w[i] = 0.0;
        u[i] = 0.0;
    }
}

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. The 
output can safely alias the input. */
void vecCopy(int dim, const GLdouble v[], GLdouble copy[]) {
	for (int i = 0; i < dim; i += 1)
		copy[i] = v[i];
}

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void vecPrint(int dim, const GLdouble v[]) {
	for (int i = 0; i < dim; i += 1) {
		printf("%f\n", v[i]);
	}
	printf("\n");
}

/* Adds the dim-dimensional vectors v and w. The output can safely alias the 
input. */
void vecAdd(int dim, const GLdouble v[], const GLdouble w[], GLdouble vPlusW[]) {
	for(int i = 0;i < dim;i = i+1){
		vPlusW[i] = v[i] + w[i];
	}
}

/* Adds the dim-dimensional vectors v,w and u. The output can safely alias the 
input. */
void vecAdd3(int dim, const GLdouble v[], const GLdouble w[], const GLdouble u[], GLdouble vPlusW[]) {
	for(int i = 0;i < dim;i += 1){
		vPlusW[i] = v[i] + w[i] + u[i];
	}
}

/* Subtracts the dim-dimensional vectors v and w. The output can safely alias 
the input. */
void vecSubtract(int dim, const GLdouble v[], const GLdouble w[], 
		GLdouble vMinusW[]) {
	for(int i = 0;i < dim;i += 1){
		vMinusW[i] = v[i] - w[i];
	}
}

/* Scales the dim-dimensional vector w by the number c. The output can safely 
alias the input.*/
void vecScale(int dim, GLdouble c, const GLdouble w[], GLdouble cTimesW[]) {
	for(int i = 0;i < dim;i += 1){
		cTimesW[i] = w[i]*c;
	}	
}

/* Returns the dot product of the vectors v and w. */
GLdouble vecDot(int dim, const GLdouble v[], const GLdouble w[]){
	GLdouble dot;
	for(int i = 0;i < dim;i += 1)
		dot += v[i]*w[i];
	return dot;
}

/* Returns the "multiply product" of the vectors v and w. */
void vecMultiply(int dim, const GLdouble v[], const GLdouble w[], GLdouble vTimesw[]){
    for(int i = 0; i < dim; i += 1){
        vTimesw[i] = v[i]*w[i];
    }
}

/* Returns the length of the vector v. */
GLdouble vecLength(int dim, const GLdouble v[]){
	GLdouble length;
	for(int i = 0;i < dim;i += 1)
		length += v[i]*v[i];
	return sqrt(length);
}

/* Returns the length of the vector v. If the length is non-zero, then also
places a normalized (length-1) version of v into unit. The output can safely
alias the input. */
GLdouble vecUnit(int dim, const GLdouble v[], GLdouble unit[]){
	GLdouble length = vecLength(dim,v);
	if(length != 0)
		vecScale(dim,1/length,v,unit);
	return length;
}

/* Computes the cross product of v and w, and places it into vCrossW. The
output CANNOT safely alias the input. */
void vec3Cross(const GLdouble v[3], const GLdouble w[3], GLdouble vCrossW[3]){
	vCrossW[0] = v[1]*w[2] - v[2]*w[1];
	vCrossW[1] = v[2]*w[0] - v[0]*w[2];
	vCrossW[2] = v[0]*w[1] - v[1]*w[0];
}

/* Computes the vector v from its spherical coordinates. rho >= 0.0 is the
radius. 0 <= phi <= pi is the co-latitude. -pi <= theta <= pi is the longitude
or azimuth. */
void vec3Spherical(GLdouble rho, GLdouble phi, GLdouble theta, GLdouble v[3]){
	GLdouble sinPhi = sin(phi);
	v[0] = rho*sinPhi*cos(theta);
	v[1] = rho*sinPhi*sin(theta);
	v[2] = rho*cos(phi);
}


/*** In specific dimensions ***/

/* By the way, there is a way to write a single vecSet function that works in 
all dimensions. The module stdarg.h, which is part of the C standard library, 
lets you write variable-arity functions. The general vecSet would look like
	void vecSet(int dim, GLdouble a[], ...)
where the '...' represents dim numbers to be loaded into a. We're not going to 
take this approach for two reasons. First, I try not to burden you with 
learning a lot of C that isn't strictly necessary. Second, the variable-arity 
feature is a bit dangerous, in that it provides no type checking. */

/* Copies three numbers into a three-dimensional vector. */
void vec3Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a[3]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
}


/* Copies four numbers into a four-dimensional vector. */
void vec4Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a[4]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	a[3] = a3;	
}

/* Copies eight numbers into a eight-dimensional vector. */
void vec8Set(GLdouble a0, GLdouble a1, GLdouble a2, GLdouble a3, GLdouble a4, GLdouble a5, 
		GLdouble a6, GLdouble a7, GLdouble a[8]) {
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	a[3] = a3;	
	a[4] = a4;
	a[5] = a5;
	a[6] = a6;
	a[7] = a7;		
}


