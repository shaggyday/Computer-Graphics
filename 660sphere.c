


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

void sphColor(const void *body, const rayQuery *query,
			  const rayResponse *response, double rgb[3]) {
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
	double cDiffuse[sph->texture->texelDim];
	texSample(sph->texture, texCoords[0], texCoords[1], cDiffuse);
	{
		/* Calculating the normals */
		double center[3], xMinusCenter[3], dNormal[3];
		vecCopy(3, sph->isometry.translation, center);
		vecSubtract(3, x, center, xMinusCenter);
		vecUnit(3, xMinusCenter, dNormal);
		/* Specular reflection that reflects camera instead of light */
		double cSpecular[3] = {0.5, 0.5, 0.5}, shininess = 16.0, dCamera[3];
		vecScale(3, -1.0, query->d, dCamera);
		vecUnit(3, dCamera, dCamera);
		diffuseAndSpecular(dNormal, dLight, dCamera, cDiffuse,
						   cSpecular, shininess, cLight, rgb);
	}
	/* Ambient light. */
	rgb[0] += cDiffuse[0] * cAmbient[0];
	rgb[1] += cDiffuse[1] * cAmbient[1];
	rgb[2] += cDiffuse[2] * cAmbient[2];
}

rayClass sphClass = {sphIntersection, sphColor};


