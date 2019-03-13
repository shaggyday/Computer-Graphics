


/* A simple directional light. dLight should be a unit vector when initialized. */
typedef struct directLight directLight;
struct directLight {
	lightClass *class;
	double dLight[3];
	double cLight[3];
};

lightResponse directLighting(const void *light, const double world[3]) {
	const directLight *direct = (const directLight *)light;
	lightResponse result;
	vecCopy(3, direct->cLight, result.cLight);
	vecCopy(3, direct->dLight, result.dLight);
	result.distance = rayINFINITY;
	return result;
}

lightClass directClass = {directLighting};


