


/* The planeinder is infinitely long. It is centered on its local z-axis and has 
the prescribed radius. Its class should be set to planeClass. */
typedef struct plaPlane plaPlane;
struct plaPlane {
	rayClass *class;
	isoIsometry isometry;
	texTexture *texture;
};

rayResponse planeIntersection(const void *body, const rayQuery *query) {
	const plaPlane *plane = (const plaPlane *)body;
	rayResponse result;
	/* Transform to local coordinates. */
	double eLocal[3], dLocal[3];
	isoUntransformPoint(&(plane->isometry), query->e, eLocal);
	isoUnrotateVector(&(plane->isometry), query->d, dLocal);
	/* Ignore the first and second dimensions. */
	if (dLocal[2] == 0.0){
		result.intersected = 0;
		return result;
	}
	double t = -eLocal[2] / dLocal[2];
	if (t <= query->tStart || t >= query->tEnd){
		result.intersected = 0;
		return result;
	}
	if (dLocal[2] < 0.0){
		result.intersected = -1;
		result.t = t;
		return result;
	}
	if (dLocal[2] > 0.0){
		result.intersected = 1;
		result.t = t;
		return result;
	}
	result.intersected = 0;
	return result;
}


void planeColor(const void *body, const rayQuery *query,
			  const rayResponse *response, double rgb[3]) {
	const plaPlane *plane = (const plaPlane *) body;
	/* x = e + t d. */
	double tTimesD[3], x[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, tTimesD);
	vecAdd(3, query->e, tTimesD, x);
	isoUntransformPoint(&(plane->isometry), x, xLocal);
	/* Sample texture to get diffuse surface color. */
	double texCoords[2] = {xLocal[0], xLocal[1]};
	vecScale(3, 0.5, texCoords, texCoords);
	/* Do lighting calculations in global coordinates */
	double cDiffuse[plane->texture->texelDim];
	texSample(plane->texture, texCoords[0], texCoords[1], cDiffuse);
	{
		/* Calculating the normals */
		double dNormalLocal[3] = {0.0, 0.0, -response->intersected}, dLightLocal[3], dCameraLocal[3];
        isoUnrotateVector(&(plane->isometry), dLight, dLightLocal);
		/* Specular reflection that reflects camera instead of light */
		double cSpecular[3] = {0.5, 0.5, 0.5}, shininess = 16.0, dCamera[3];
		vecScale(3, -1.0, query->d, dCamera);
        isoUntransformPoint(&(plane->isometry), dCamera, dCameraLocal);
		vecUnit(3, dCameraLocal, dCameraLocal);
		diffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiffuse,
						   cSpecular, shininess, cLight, rgb);
	}
	/* Ambient light. */
	rgb[0] += cDiffuse[0] * cAmbient[0];
	rgb[1] += cDiffuse[1] * cAmbient[1];
	rgb[2] += cDiffuse[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};


