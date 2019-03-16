


#define rayEPSILON 0.00000001
#define rayINFINITY 100000000

typedef struct rayQuery rayQuery;
struct rayQuery {
	/* The ray is parametrized as x(t) = e + t d for t in [tStart, tEnd]. The 
	direction d is non-zero but not necessarily unit. */
	double e[3], d[3], tStart, tEnd;
};

typedef struct rayResponse rayResponse;
struct rayResponse {
	/* 0 for no intersection (or just tangency), -1 for entering the body, or 1 
	for exiting the body. */
	int intersected;
	/* If the intersection code is non-zero, then the t member contains the 
	time of intersection. */
	double t;
};

typedef struct rayClass rayClass;
struct rayClass {
	/* If the ray e + t d intersects the body for some t in [tStart, tEnd], 
	then returns the least such t, along with an intersection code: -1 for 
	entering the body and 1 for exiting it. If the ray does not intersect the 
	body in [tStart, tEnd] (or is tangent to it), then returns an intersection 
	code of 0. */
	rayResponse (*intersection)(const void *body, const rayQuery *query);
	/* Fills the rgb argument with the color of the body at a certain 
	intersection point. The query that produced the intersection and the body's 
	reponse to that query are included in the parameters. */
	void (*color)(const void *body, rayQuery *query, 
		const rayResponse *response, int bodyNum, const void *bodies[], 
		int lightNum, const void *lights[], const double cAmbient[3], 
		int recursionNum, double rgb[3]);
};

/* If the ray does not intersect the scene, then sets index to -1. If the ray
intersects the scene, then updates query->tEnd, outputs the index of the body
in the list of bodies, and returns the corresponding rayResponse that corresponds
to the body that the ray intersects (first).  */
rayResponse rayIntersection(int bodyNum, const void *bodies[], rayQuery *query,
                            int *index){
    rayResponse candidate, response;
    *index = -1;
    rayClass **class;
    for (int k = 0; k < bodyNum; k += 1) {
        class = (rayClass **)(bodies[k]);
        candidate = (*class)->intersection(bodies[k], query);
        if (candidate.intersected) {
            query->tEnd = candidate.t;
            response = candidate;
            *index = k;
        }
    }
    return response;
}

/* Outputs the RGB color of the specified ray. If the ray hits nothing in the 
scene, then outputs some fixed background color. Also returns the response to 
the given query. */
rayResponse rayColor(int bodyNum, const void *bodies[], int lightNum,
		const void *lights[], const double cAmbient[3], rayQuery *query,
		int recursionNum, double rgb[3]){
	int index;
	rayResponse response = rayIntersection(bodyNum, bodies, query, &index);
	/* Color the pixel. */
	rayClass **class;
	if (index >= 0) {
		class = (rayClass **)(bodies[index]);
		(*class)->color(bodies[index], query, &response, bodyNum, bodies,
				lightNum, lights, cAmbient, recursionNum, rgb);
	}
	else
		vec3Set(0.0, 0.0, 0.0, rgb);
	return response;
}

/* Helper function for shadow calculations */
int shadowTest(lightResponse lightResponse1,  double x[3], int bodyNum, const void *bodies[]){
	rayQuery shadowQuery;
	rayResponse shadowResponse;
	vecCopy(3, x, shadowQuery.e);
	vecCopy(3, lightResponse1.dLight, shadowQuery.d);
	shadowQuery.tStart = rayEPSILON;
	shadowQuery.tEnd = lightResponse1.distance;
	if (shadowQuery.tEnd != rayINFINITY){
	    double dLength = vecLength(3, shadowQuery.d);
	    shadowQuery.tEnd = shadowQuery.tEnd / dLength;
	}
	int index;
	shadowResponse = rayIntersection(bodyNum, bodies, &shadowQuery, &index);
	return index;
}

/* dRefl = 2 (dCam . dNorm) dNorm - dCam. */
void reflection(const double dIn[3], const double dNormal[3], double dRefl[3]){
    double twiceDot = 2.0 * vecDot(3, dIn, dNormal);
    vecScale(3, twiceDot, dNormal, dRefl);
    vecSubtract(3, dRefl, dIn, dRefl);
}

/* Directions can be local or global, as long as they're consistent. */
void rayDiffuseAndSpecular(const double dNormal[3], const double dLight[3], 
		const double dCamera[3], const double cDiff[3], const double cLight[3], 
		const double cSpec[3], const double shininess, double rgb[3]) {
	/* Diffuse reflection. */
	double diffuse[3];
	double iDiff = vecDot(3, dNormal, dLight);
	if (iDiff <= 0.0)
		iDiff = 0.0;
	diffuse[0] = iDiff * cDiff[0] * cLight[0];
	diffuse[1] = iDiff * cDiff[1] * cLight[1];
	diffuse[2] = iDiff * cDiff[2] * cLight[2];
	double dRefl[3];
	reflection(dCamera, dNormal, dRefl);
	double iSpec = vecDot(3, dRefl, dLight);
	if (iDiff <= 0.0 || iSpec <= 0.0)
		iSpec = 0.0;
	iSpec = pow(iSpec, shininess);
	double specular[3];
	specular[0] = iSpec * cSpec[0] * cLight[0];
	specular[1] = iSpec * cSpec[1] * cLight[1];
	specular[2] = iSpec * cSpec[2] * cLight[2];
	/* Output. */
	vecAdd(3, diffuse, specular, rgb);
}

/* Given a unit outward-pointing normal, an incident ray passing through a
medium with index of refraction indexInc, and an index of refraction indexRefr
for the medium, through which the refracted ray should pass. Computes the
refracted (or internally reflected) ray. To clarify the sign convention, all
vectors are based at the point where the incident ray hits the interface; the
incident and refracted rays point in approximately opposite directions; they
are truly opposite when dInc = dNormal. */
void rayRefraction(const double dNormal[3], double indexInc,
                   const double dInc[3], double indexRefr, double dRefr[3]) {
    /* Gram-Schmidt to get dTan perpendicular to dNormal. */
    double dIncDNormal, proj[3], dTan[3];
    dIncDNormal = vecDot(3, dInc, dNormal);
    vecScale(3, dIncDNormal, dNormal, proj);
    vecSubtract(3, dInc, proj, dTan);
    if (vecUnit(3, dTan, dTan) == 0.0) {
        /* dInc is parallel to dNormal. The ray doesn't refract. */
        vecScale(3, -1.0, dInc, dRefr);
        return;
    }
    double sinTheta1 = vecDot(3, dInc, dTan);
    double sinTheta2 = sinTheta1 * indexInc / indexRefr;
    if (sinTheta2 > 1.0) {
        /* Total internal reflection. */
        vecScale(3, 2.0 * dIncDNormal, dNormal, dRefr);
        vecSubtract(3, dRefr, dInc, dRefr);
    } else {
        /* Refraction. */
        double cosTheta2 = sqrt(1.0 - sinTheta2 * sinTheta2);
        if (dIncDNormal < 0.0)
            cosTheta2 = -cosTheta2;
        vecScale(3, -cosTheta2, dNormal, dRefr);
        vecScale(3, -sinTheta2, dTan, dTan);
        vecAdd(3, dTan, dRefr, dRefr);
    }
}
