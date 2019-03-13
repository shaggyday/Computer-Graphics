


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


void planeColor(const void *body, rayQuery *query,
                const rayResponse *response, int bodyNum, const void *bodies[],
                int lightNum, const void *lights[], const double cAmbient[3],
                int recursionNum, double rgb[3]) {
	const plaPlane *plane = (const plaPlane *) body;
	/* x = e + t d. */
	double tTimesD[3], x[3], xLocal[3];
	vecScale(3, query->tEnd, query->d, tTimesD);
	vecAdd(3, query->e, tTimesD, x);
	isoUntransformPoint(&(plane->isometry), x, xLocal);
	/* Sample texture to get diffuse surface color. */
	double texCoords[2] = {xLocal[0] / 5,xLocal[1] / 5};
	vecScale(3, 0.5, texCoords, texCoords);
	/* Do lighting calculations in global coordinates */
	double cDiff[plane->texture->texelDim];
	texSample(plane->texture, texCoords[0], texCoords[1], cDiff);
	/* loops over the lights, adding each one's diffuse and specular contributions to the ambient color. */
    lightClass **class;
    lightResponse lightResponse1;
	double cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
    for (int i = 0; i < lightNum; i +=1){
        class = (lightClass **)(lights[i]);
        lightResponse1 = (*class)->lighting(lights[i], x);
        int index = shadowTest(lightResponse1, x, bodyNum, bodies);
        if (index < 0) {
			/* Do lighting calculations in local coordinates. */
            double dNormalLocal[3] = {0.0, 0.0, -response->intersected}, dLightLocal[3], dCameraLocal[3];
            isoUnrotateVector(&(plane->isometry), lightResponse1.dLight, dLightLocal);
            vecUnit(3, dLightLocal, dLightLocal);
            double dCamera[3];
            vecScale(3, -1.0, query->d, dCamera);
			isoUnrotateVector(&(plane->isometry), dCamera, dCameraLocal);
            vecUnit(3, dCameraLocal, dCameraLocal);
            double temp[3];
            rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff,
                    lightResponse1.cLight, cSpec, shininess, temp);
            vecAdd(3, temp, rgb, rgb);
        }
	}
	/* Add mirror contribution */
	double cMirr[3];
	if (recursionNum == 0)
		vec3Set(0.0, 0.0, 0.0, cMirr);
	else if (recursionNum > 0){
		rayResponse mirrorRay =	rayColor(bodyNum, bodies, lightNum, lights, cAmbient, query, recursionNum - 1, cMirr);
	}
	/* Ambient light. */
	rgb[0] += cMirr[0] * cSpec[0] + cDiff[0] * cAmbient[0];
	rgb[1] += cMirr[1] * cSpec[1] + cDiff[1] * cAmbient[1];
	rgb[2] += cMirr[2] * cSpec[2] + cDiff[2] * cAmbient[2];
}

rayClass planeClass = {planeIntersection, planeColor};


