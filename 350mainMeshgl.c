/* On macOS, compile with...
    clang 350mainMeshgl.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "320shading.c"
#include "310vector.c"
#include "320matrix.c"
#include "320isometry.c"
#include "320camera.c"
#include "310mesh.c"
#include "350meshgl.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

GLuint program;
GLint positionLoc, colorLoc;
GLint viewingLoc, modelingLoc;
GLint cLightLoc, pLightLoc, cAmbientLoc, pCameraLoc;

#define TRINUM 8
#define VERTNUM 6
#define ATTRDIM 6
#define UNIFNUM 6
#define ATTRNUM 2
#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFcLIGHT 2
#define UNIFpLIGHT 3
#define UNIFcAMBIENT 4
#define UNIFpCAMERA 5
#define ATTRPOSITION 0
#define ATTRCOLOR 1

shaShading sha;
const GLchar *uniformNames[6] = {"viewing", "modeling", "cLight", "pLight", "cAmbient", "pCamera"};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[2] = {"position", "color"};
const GLchar **attrNames = attributeNames;
double angle = 0.0;

meshMesh sphere;
meshglMesh sphereGL;
isoIsometry modeling;
camCamera cam;

double getTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void handleError(int error, const char *description) {
    fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    /* The documentation for camSetFrustum says that we must re-call it here. */
    camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, width, height);
}

void initializeMesh(void) {
    meshInitializeSphere(&sphere, 1.0, 32, 32);
    meshglInitialize(&sphereGL, &sphere);
    meshDestroy(&sphere);
    /* VAO stuff */
    glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
    glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
                          ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(sha.attrLocs[ATTRCOLOR]);
    glVertexAttribPointer(sha.attrLocs[ATTRCOLOR], 3, GL_DOUBLE, GL_FALSE,
                          ATTRDIM * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
    meshglFinishInitialization(&sphereGL);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
    /* The two matrices will be sent to the shaders as uniforms. */
    GLchar vertexCode[] = "\
        #version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec3 color;\
		out vec4 rgba;\
		out vec3 dNormal;\
		out vec3 pFragment;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			rgba = vec4(color, 1.0);\
			dNormal = position;\
            vec4 world = modeling * vec4(position, 1.0);\
			pFragment = vec3(world);\
		}";
    GLchar fragmentCode[] = "\
		#version 140\n\
		uniform vec3 cLight;\
		uniform vec3 pLight;\
		uniform vec3 cAmbient;\
	    uniform vec3 pCamera;\
		in vec4 rgba;\
		in vec3 dNormal;\
        in vec3 pFragment;\
        out vec4 fragColor;\
		void main() {\
            vec3 dNorm = normalize(dNormal);\
            vec3 dCamera = normalize(pCamera - pFragment);\
            vec3 dLight = normalize(pLight - pFragment);\
			float iDiff = dot(dLight, dNorm);\
			if (iDiff < 0.0)\
				iDiff = 0.0;\
			vec3 cDiff = rgba.xyz;\
			vec3 diffuse = iDiff * cDiff * cLight;\
            vec3 dRefl = 2.0 * iDiff * dNorm - dLight;\
            float iSpec = dot(dRefl, dCamera);\
            if (iSpec < 0.0 || iDiff == 0.0)\
                iSpec = 0.0;\
            float shininess = 20.0;\
            iSpec = pow(iSpec, shininess);\
            vec3 cSpec = vec3(1.0, 1.0, 1.0);\
            vec3 specular = iSpec * cSpec * cLight;\
			vec3 ambient = cDiff * cAmbient;\
			fragColor = vec4(diffuse + specular + ambient, rgba.w);\
		}";
    return shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM, attrNames);
}

/* We want to pass 4x4 matrices into uniforms in OpenGL shaders, but there are 
two obstacles. First, our matrix library uses double matrices, but OpenGL 
shaders expect GLfloat matrices. Second, C matrices are implicitly stored one-
row-after-another, while OpenGL shaders expect matrices to be stored one-column-
after-another. This function plows through both of those obstacles. */
void uniformMatrix44(GLdouble m[4][4], GLint uniformLocation) {
    GLfloat mTFloat[4][4];
    for (int i = 0; i < 4; i += 1)
        for (int j = 0; j < 4; j += 1)
            mTFloat[i][j] = m[j][i];
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}

/* Here is a similar function for vectors. */
void uniformVector3(GLdouble v[3], GLint uniformLocation) {
    GLfloat vFloat[3];
    for (int i = 0; i < 3; i += 1)
        vFloat[i] = v[i];
    glUniform3fv(uniformLocation, 1, vFloat);
}

void render(double oldTime, double newTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(sha.program);
    /* Send our own modeling transformation M to the shaders. */
    GLdouble trans[3] = {0.0, 0.0, 0.0};
    isoSetTranslation(&modeling, trans);
    angle += 0.1 * (newTime - oldTime);
    GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
    GLdouble rot[3][3];
    mat33AngleAxisRotation(angle, axis, rot);
    isoSetRotation(&modeling, rot);
    GLdouble model[4][4];
    isoGetHomogeneous(&modeling, model);
    uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
    /* Send our own viewing transformation P C^-1 to the shaders. */
    GLdouble viewing[4][4];
    camGetProjectionInverseIsometry(&cam, viewing);
    uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);
    /* Send light color and light direction to the shaders. */
    GLdouble cLIGHT[3] = {1.0, 1.0, 1.0};
    GLdouble pLIGHT[3] = {1.0, 1.0, 1.0};
    GLdouble cAMBIENT[3] = {1.0, 1.0, 1.0};
    GLdouble pCAMERA[3] = {0.0, 0.0, 1.0};
    uniformVector3(cLIGHT, sha.unifLocs[UNIFcLIGHT]);
    uniformVector3(pLIGHT, sha.unifLocs[UNIFpLIGHT]);
    uniformVector3(cAMBIENT, sha.unifLocs[UNIFcAMBIENT]);
    uniformVector3(pCAMERA, sha.unifLocs[UNIFpCAMERA]);

    /* Binding and rendering using meshGL */
//    glBindBuffer(GL_ARRAY_BUFFER, boxGL.buffers[0]);
//    glVertexPointer(3, GL_DOUBLE, boxGL.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
//    glNormalPointer(GL_DOUBLE, boxGL.attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
//    glColorPointer(3, GL_DOUBLE, boxGL.attrDim * sizeof(GLdouble),
//                   BUFFER_OFFSET(3 * sizeof(GLdouble)));
    meshglRender(&sphereGL);
}

int main(void) {
    double oldTime;
    double newTime = getTime();
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0) {
        fprintf(stderr, "main: glfwInit failed.\n");
        return 1;
    }
    /* Ask GLFW to supply an OpenGL 3.2 context. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window;
    window = glfwCreateWindow(768, 512, "Learning OpenGL 3.2", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "main: glfwCreateWindow failed.\n");
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwMakeContextCurrent(window);
    /* You might think that getting an OpenGL 3.2 context would make OpenGL 3.2
    available to us. But you'd be wrong. The following call 'loads' a bunch of
    OpenGL 3.2 functions, so that we can use them. This is why we use GL3W. */
    if (gl3wInit() != 0) {
        fprintf(stderr, "main: gl3wInit failed.\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 3;
    }
    /* We rarely invoke any GL3W functions other than gl3wInit. But just for an
    educational example, let's ask GL3W about OpenGL support. */
    if (gl3wIsSupported(3, 2) == 0)
        fprintf(stderr, "main: OpenGL 3.2 is not supported.\n");
    else
        fprintf(stderr, "main: OpenGL 3.2 is supported.\n");
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n",
            glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    double target[3] = {0.0, 0.0, 0.0};
    camLookAt(&cam, target, 5.0, M_PI / 3.0, -M_PI / 4.0);
    camSetProjectionType(&cam, camPERSPECTIVE);
    camSetFrustum(&cam, M_PI / 6.0, 5.0, 10.0, 768, 512);
    /* Initialize the shader program before the mesh, so that the shader
    locations are already set up by the time the vertex array object is
    initialized. */
    if (initializeShaderProgram() != 0)
        return 4;
    initializeMesh();
    while (glfwWindowShouldClose(window) == 0) {
        oldTime = newTime;
        newTime = getTime();
        if (floor(newTime) - floor(oldTime) >= 1.0)
            printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    shaDestroy(&sha);
    meshglDestroy(&sphereGL);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}