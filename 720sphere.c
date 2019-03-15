


/* The cylinder is infinitely long. It is centered on its local z-axis and has 
the prescribed radius. Its class should be set to cylClass. */
typedef struct sphereSphere sphereSphere;
struct sphereSphere {
	rayClass *class;
	isoIsometry isometry;
	double radius;
	texTexture *texture;
};

rayResponse sphIntersection(const void *body, const rayQuery *query) {
	const sphereSphere *sph = (const sphereSphere *)body;
	rayResponse result;
	double center[3], eMinusCenter[3], a, b, c, disc, t;
	vecCopy(3, sph->isometry.translation, center);
	a = pow(vecLength(3, query->d), 2);
	vecSubtract(3, query->e, center, eMinusCenter);
	b = 2 * vecDot(3, query->d, eMinusCenter);
	c = vecDot(3, eMinusCenter, eMinusCenter) - pow(sph->radius, 2.0);
	disc = b * b - 4 * a * c;
	if (disc <= 0) {
		result.intersected = 0;
		return result;
	}
	double sqrtDisc = sqrt(disc);
	t = (-b - sqrtDisc) / (2 * a);
	if (query->tStart <= t && t <= query->tEnd) {
		result.intersected = -1;
		result.t = t;
		return result;
	}
	t = (-b + sqrtDisc) / (2 * a);
	if (query->tStart <= t && t <= query->tEnd) {
		result.intersected = 1;
		result.t = t;
		return result;
	}
	result.intersected = 0;
	return result;
}

void sphTexCoords(const double xLocal[3], double st[2]) {
	/* Simply use spherical coordinates as texture coordinates. */
	double rho, phi, theta;
	vec3Rectangular(xLocal, &rho, &phi, &theta);
	st[0] = phi / M_PI;
	st[1] = theta / (2 * M_PI);
}

void sphColor(const void *body,rayQuery *query,
              const rayResponse *response, int bodyNum, const void *bodies[],
              int lightNum, const void *lights[], const double cAmbient[3],
              int recursionNum, double rgb[3]) {
    const sphereSphere *sph = (const sphereSphere *) body;
    /* x = e + t d. */
    double tTimesD[3], x[3], xLocal[3];
    vecScale(3, query->tEnd, query->d, tTimesD);
    vecAdd(3, query->e, tTimesD, x);
    isoUntransformPoint(&(sph->isometry), x, xLocal);
    /* Sample texture to get diffuse surface color. */
    double texCoords[2];
    sphTexCoords(xLocal, texCoords);
    /* Do lighting calculations in global coordinates */
    double cDiff[sph->texture->texelDim];
    texSample(sph->texture, texCoords[0], texCoords[1], cDiff);
    /* loops over the lights, adding each one's diffuse and specular contributions to the ambient color. */
    lightClass **class;
    lightResponse lightResponse1;
    double diffuseNSpecular[3], cSpec[3] = {0.5, 0.5, 0.5}, shininess = 16.0;
    double center[3], xMinusCenter[3], dNormal[3], dNormalLocal[3];
    vecCopy(3, sph->isometry.translation, center);
    vecSubtract(3, x, center, xMinusCenter);
    vecUnit(3, xMinusCenter, dNormal);
    isoUnrotateVector(&sph->isometry, dNormal, dNormalLocal);
    for (int i = 0; i < lightNum; i +=1){
        class = (lightClass **)(lights[i]);
        lightResponse1 = (*class)->lighting(lights[i], x);
        int index = shadowTest(lightResponse1,  x, bodyNum, bodies);
        if (index < 0) {
            /* Do lighting calculations in local coordinates. */
            double dCamera[3], dCameraLocal[3], dLightLocal[3];
            isoUnrotateVector(&sph->isometry, lightResponse1.dLight, dLightLocal);
            vecUnit(3, dLightLocal, dLightLocal);
            vecScale(3, -1.0, query->d, dCamera);
            isoUnrotateVector(&sph->isometry, dCamera, dCameraLocal);
            vecUnit(3, dCameraLocal, dCameraLocal);
            rayDiffuseAndSpecular(dNormalLocal, dLightLocal, dCameraLocal, cDiff,
                                  lightResponse1.cLight, cSpec, shininess, diffuseNSpecular);
            vecAdd(3, diffuseNSpecular, rgb, rgb);
        }
    }
	/* Mirror contribution */
	if (recursionNum > 0){
        double cMirr[3], Mirror[3];
        rayQuery mirrorQuery;
        vecCopy(3, x, mirrorQuery.e);
        reflection(query->d, dNormal, mirrorQuery.d);
        vecScale(3, -1, mirrorQuery.d, mirrorQuery.d);
        mirrorQuery.tStart = rayEPSILON;
        mirrorQuery.tEnd = rayINFINITY;
		rayResponse mirrorRay =	rayColor(bodyNum, bodies, lightNum, lights, cAmbient, &mirrorQuery, recursionNum - 1, cMirr);
        vecMultiply(3, cMirr, cSpec, Mirror);
        vecAdd(3, Mirror, rgb, rgb);
	/* Transmission contributions
	   I can't debug this part :( The returned transmission colors are way over {1.0, 1.0, 1.0} so everything is white. */
        double dInc[3], cTransed[3], cTran[3] = {0.5, 0.5, 0.5}, tranMission[3];
        rayQuery tranQuery;
        vecCopy(3, x, tranQuery.e);
        vecScale(3, -1, query->d, dInc);
        vecUnit(3, dInc, dInc);
        if (vecUnit(3, dInc, dNormal) >= 0)
            rayRefraction(dNormal, AIRINDEX, dInc, SPHEREINDEX, tranQuery.d);
        else
            rayRefraction(dNormal, SPHEREINDEX, dInc, AIRINDEX, tranQuery.d);
        vecUnit(3, tranQuery.d, tranQuery.d);
        tranQuery.tStart = rayEPSILON;
        tranQuery.tEnd = rayINFINITY;
        rayResponse tranRay = rayColor(bodyNum, bodies, lightNum, lights, cAmbient, &tranQuery, recursionNum - 1, cTransed);
        vecMultiply(3, cTransed, cTran, tranMission);
//        vecPrint(3, cTransed);
        vecAdd(3, tranMission, rgb, rgb);
    }
	/* Ambient light. */
    double ambient[3];
    vecMultiply(3, cDiff, cAmbient, ambient);
    vecAdd(3, ambient, rgb, rgb);
}

rayClass sphClass = {sphIntersection, sphColor};


