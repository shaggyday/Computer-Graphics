/* On macOS, compile with...
    clang 390mainSpot.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
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
#include "360texture.c"
#include "310mesh.c"
#include "350meshgl.c"
#include "140landscape.c"
#include "370body.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

#define mainSCREENWIDTH 512
#define mainSCREENHEIGHT 768
#define UNIFNUM 10
#define ATTRNUM 3
#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFcLIGHT1 2
#define UNIFdLIGHT 3
#define UNIFcAMBIENT 4
#define UNIFpCAMERA 5
#define UNIFTEXTURE0 6
#define UNIFcLIGHT2 7
#define UNIFpLIGHT 8
#define UNIFdSPOT 9
#define UNIFdCOSSPOTANGLE 10
#define ATTRPOSITION 0
#define ATTRTEXURECOORDS 1
#define ATTRNORMAL 2

double angle = 0.0;
camCamera cam;
double cameraTarget[3] = {0.0, 0.0, 0.0};
double cameraRho = 200.0;
double cameraPhi = M_PI / 3.0;
double cameraTheta = -M_PI / 4.0;
double spotAngle = M_PI / 12;

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

shaShading sha;
const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "cLight1", "dLight", "cAmbient",
                                       "pCamera", "texture0", "cLight2", "pLight", "dSpot"};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position", "textureCoords", "normal"};
const GLchar **attrNames = attributeNames;
/* Returns 0 on success, non-zero on failure. */
int initializeShading(void) {
    /* The two matrices will be sent to the shaders as uniforms. */
    GLchar vertexCode[] = "\
        #version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec2 textureCoords;\
        in vec3 normal;\
		out vec4 rgba;\
        out vec2 textureST;\
		out vec3 dNormal;\
		out vec3 pFragment;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
			dNormal = vec3(modeling * vec4(normal, 0.0));\
            vec4 world = modeling * vec4(position, 1.0);\
			pFragment = vec3(world);\
            textureST = textureCoords;\
		}";
    GLchar fragmentCode[] = "\
		#version 140\n\
        uniform sampler2D texture0;\
		uniform vec3 cLight1;\
		uniform vec3 dLight;\
		uniform vec3 cAmbient;\
	    uniform vec3 pCamera;\
		uniform vec3 cLight2;\
        uniform vec3 pLight;\
        uniform vec3 dSpot;\
		in vec3 dNormal;\
        in vec3 pFragment;\
        in vec2 textureST;\
        out vec4 fragColor;\
		void main() {\
		    vec3 dLightNormal = normalize(dLight);\
            vec3 dNorm = normalize(dNormal);\
            vec3 dCamera = normalize(pCamera - pFragment);\
			float iDiff1 = dot(dLightNormal, dNorm);\
			if (iDiff1 < 0.0)\
				iDiff1 = 0.0;\
            vec3 rgbFromTex = vec3(texture(texture0, textureST));\
			vec3 cDiff = rgbFromTex;\
			vec3 diffuse1 = iDiff1 * cDiff * cLight1;\
            vec3 dRefl1 = 2.0 * iDiff1 * dNorm - dLightNormal;\
            float iSpec1 = dot(dRefl1, dCamera);\
            if (iSpec1 < 0.0 || iDiff1 == 0.0)\
                iSpec1 = 0.0;\
            float shininess1 = 20.0;\
            iSpec1 = pow(iSpec1, shininess1);\
            vec3 cSpec1 = vec3(1.0, 1.0, 1.0);\
            vec3 specular1 = iSpec1 * cSpec1 * cLight1;\
            vec3 diffuse2 = vec3(0.0, 0.0, 0.0);\
            vec3 specular2 = vec3(0.0, 0.0, 0.0);\
            vec3 dSpotLight = normalize(pLight - pFragment);\
            vec3 dSpotNormal = normalize(dSpot);\
            if (dot(dSpotLight, dSpotNormal) >= 0.989){\
                float iDiff2 = dot(dSpotLight, dNorm);\
                if (iDiff2 < 0.0)\
                    iDiff2 = 0.0;\
                diffuse2 = iDiff2 * cDiff * cLight2;\
                vec3 dRefl2 = 2.0 * iDiff2 * dNorm - dSpotLight;\
                float iSpec2 = dot(dRefl2, dCamera);\
                if (iSpec2 < 0.0 || iDiff2 == 0.0)\
                    iSpec2 = 0.0;\
                float shininess2 = 10.0;\
                iSpec2 = pow(iSpec2, shininess1);\
                vec3 cSpec2 = vec3(1.0, 1.0, 1.0);\
                specular2 = iSpec2 * cSpec2 * cLight2;\
            }\
			vec3 ambient = cDiff * cAmbient;\
			fragColor = vec4(diffuse1 + specular1 + diffuse2 + specular2 + ambient, 1.0);\
		}";
    return shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM, attrNames);
}

void initializeMesh(meshglMesh *meshgl, meshMesh *mesh, shaShading *sha){
    meshglInitialize(meshgl, mesh);
    glEnableVertexAttribArray(sha->attrLocs[ATTRPOSITION]);
    glVertexAttribPointer(sha->attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
                          meshgl->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(sha->attrLocs[ATTRTEXURECOORDS]);
    glVertexAttribPointer(sha->attrLocs[ATTRTEXURECOORDS], 3, GL_DOUBLE, GL_FALSE,
                          meshgl->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
    glEnableVertexAttribArray(sha->attrLocs[ATTRNORMAL]);
    glVertexAttribPointer(sha->attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
                          meshgl->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
    meshglFinishInitialization(meshgl);
}

meshMesh trunk;
meshglMesh trunkGL;
meshMesh tree;
meshglMesh treeGL;
meshMesh landscape;
meshglMesh landscapeGL;
double landNum = 100;
double landData[100][100];
void initializeMeshes(void) {
    /* The capsule */
    meshInitializeBox(&trunk, -2.0, 2.0, -2.0, 2.0, 0.0, 20.0);
    initializeMesh(&trunkGL, &trunk, &sha);
    meshDestroy(&trunk);
    /* A really cool sphere */
    meshInitializeSphere(&tree, 5, 32, 32);
    initializeMesh(&treeGL, &tree, &sha);
    meshglFinishInitialization(&treeGL);
    /* The landscape */
    GLuint error = meshInitializeLandscape(&landscape, landNum, landNum, 1.0, (GLdouble *) landData);
    if (error == 0) {
        meshglInitialize(&landscapeGL,&landscape);
        initializeMesh(&landscapeGL,&landscape, &sha);
        meshDestroy(&landscape);
    }
}

texTexture texture1;
texTexture *textures1[1] = {&texture1};
texTexture **tex1 = textures1;
texTexture texture2;
texTexture *textures2[1] = {&texture2};
texTexture **tex2 = textures2;
texTexture texture3;
texTexture *textures3[1] = {&texture3};
texTexture **tex3 = textures3;
char *path1 = "grass-background-28.jpg";
char *path2 = "water.jpg";
char *path3 = "trunk.jpeg";
int initializeTextures(texTexture *texture, char *path){
    return texInitializeFile(texture, path, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
}

bodyBody landscapeBody;
bodyBody trunkBody1;
bodyBody trunkBody2;
bodyBody treeBody1;
bodyBody treeBody2;
int initializeScene() {
    int oh = 0;
    /* General stuff */
    oh += initializeShading();
    initializeMeshes();
    for (int i = 0; i < 1; i += 1)
        oh += initializeTextures(tex1[i], path1);
    for (int i = 0; i < 1; i += 1)
        oh += initializeTextures(tex2[i], path2);
    for (int i = 0; i < 1; i += 1)
        oh += initializeTextures(tex3[i], path3);
    /* bodies and their textures */
    oh += bodyInitialize(&landscapeBody, 0, 1);
    bodySetMesh(&landscapeBody, &landscapeGL);
    for (int i = 0; i < 1; i += 1)
        bodySetTexture(&landscapeBody, i, tex1[i]);
    oh += bodyInitialize(&trunkBody1, 0, 1);
    bodySetMesh(&trunkBody1, &trunkGL);
    for (int i = 0; i < 1; i += 1)
        bodySetTexture(&trunkBody1, i, tex3[i]);
    oh += bodyInitialize(&trunkBody2, 0, 1);
    bodySetMesh(&trunkBody2, &trunkGL);
    for (int i = 0; i < 1; i += 1)
        bodySetTexture(&trunkBody2, i, tex3[i]);
    oh += bodyInitialize(&treeBody1, 0, 1);
    bodySetMesh(&treeBody1, &treeGL);
    for (int i = 0; i < 1; i += 1)
        bodySetTexture(&treeBody1, i, tex2[i]);
    oh += bodyInitialize(&treeBody2, 0, 1);
    bodySetMesh(&treeBody2, &treeGL);
    for (int i = 0; i < 1; i += 1)
        bodySetTexture(&treeBody2, i, tex2[i]);
    return oh;
}

void destroyScene(){
    bodyDestroy(&trunkBody1);
    bodyDestroy(&trunkBody2);
    bodyDestroy(&treeBody1);
    bodyDestroy(&treeBody2);
    shaDestroy(&sha);
    texDestroy(tex1[0]);
    texDestroy(tex2[0]);
    meshglDestroy(&trunkGL);
    meshglDestroy(&treeGL);
    meshglDestroy(&landscapeGL);
}

void uniformMatrix44(GLdouble m[4][4], GLint uniformLocation) {
    GLfloat mTFloat[4][4];
    for (int i = 0; i < 4; i += 1)
        for (int j = 0; j < 4; j += 1)
            mTFloat[i][j] = m[j][i];
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (GLfloat *)mTFloat);
}
void uniformVector3(GLdouble v[3], GLint uniformLocation) {
    GLfloat vFloat[3];
    for (int i = 0; i < 3; i += 1)
        vFloat[i] = v[i];
    glUniform3fv(uniformLocation, 1, vFloat);
}

void uniformVector1(GLdouble v[1], GLint uniformLocation){
    GLfloat vFloat[1] = {v[0]};
    glUniform1fv(uniformLocation, 1, vFloat);

}

void renderBody(bodyBody *body){
    GLdouble model[4][4];
    isoGetHomogeneous(&body->isometry, model);
    uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
    texRender(body->tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEXTURE0]);
    meshglRender(body->mesh);
    texUnrender(body->tex[0], GL_TEXTURE0);
}

void render(double oldTime, double newTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(sha.program);
    /* Send our own viewing transformation P C^-1 to the shaders. */
    GLdouble viewing[4][4];
    camGetProjectionInverseIsometry(&cam, viewing);
    uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);
    /* Send light color and light direction to the shaders. */
    GLdouble cLIGHT1[3] = {0.1, 0.1, 0.1};
    GLdouble dLIGHT[3] = {1.0, 1.0, 1.0};
    GLdouble cAMBIENT[3] = {0.7, 0.7, 0.7};
    GLdouble pCAMERA[3] = {0.0, 0.0, 1.0};
    GLdouble cLIGHT2[3] = {1.0, 1.0, 1.0};
    GLdouble pLIGHT[3] = {25, 75, 20.0};
    GLdouble dSPOT[3] = {25, 75, 20.0};
    uniformVector3(cLIGHT1, sha.unifLocs[UNIFcLIGHT1]);
    uniformVector3(dLIGHT, sha.unifLocs[UNIFdLIGHT]);
    uniformVector3(cAMBIENT, sha.unifLocs[UNIFcAMBIENT]);
    uniformVector3(pCAMERA, sha.unifLocs[UNIFpCAMERA]);
    uniformVector3(pCAMERA, sha.unifLocs[UNIFpCAMERA]);
    uniformVector3(cLIGHT2, sha.unifLocs[UNIFcLIGHT2]);
    uniformVector3(pLIGHT, sha.unifLocs[UNIFpLIGHT]);
    uniformVector3(dSPOT, sha.unifLocs[UNIFdSPOT]);
    /* Same rotation for all bodies in scene */
    GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
    GLdouble rot[3][3];
    mat33AngleAxisRotation(angle, axis, rot);
    isoSetRotation(&landscapeBody.isometry, rot);
    isoSetRotation(&trunkBody1.isometry, rot);
    isoSetRotation(&trunkBody2.isometry, rot);
    isoSetRotation(&treeBody1.isometry, rot);
    isoSetRotation(&treeBody2.isometry, rot);
    /* but different translations obviously */
    GLdouble transLandscape[3] = {-50.0, -50.0, 0.0};
    GLdouble transTrunk1[3] = {0.0, 0.0, 5.0};
    GLdouble transTrunk2[3] = {20.0, 10.0, 5.0};
    GLdouble transTree1[3] = {0.0, 0.0, 25.0};
    GLdouble transTree2[3] = {20.0, 10.0, 25.0};
    isoSetTranslation(&landscapeBody.isometry, transLandscape);
    isoSetTranslation(&trunkBody1.isometry, transTrunk1);
    isoSetTranslation(&trunkBody2.isometry, transTrunk2);
    isoSetTranslation(&treeBody1.isometry, transTree1);
    isoSetTranslation(&treeBody2.isometry, transTree2);
    renderBody(&landscapeBody);
    renderBody(&trunkBody1);
    renderBody(&trunkBody2);
    renderBody(&treeBody1);
    renderBody(&treeBody2);
}

//void handleKeyAny(int key, int shiftIsDown, int controlIsDown,
//                  int altOptionIsDown, int superCommandIsDown) {
//    if (key == GLFW_KEY_A)
//        cameraTheta -= M_PI / 50;
//    else if (key == GLFW_KEY_D)
//        cameraTheta += M_PI / 50;
//    else if (key == GLFW_KEY_W)
//        cameraPhi -= M_PI / 50;
//    else if (key == GLFW_KEY_S)
//        cameraPhi += M_PI / 50;
//    else if (key == GLFW_KEY_Q)
//        cameraRho *= 0.9;
//    else if (key == GLFW_KEY_E)
//        cameraRho *= 1.1;
//    else if (key == GLFW_KEY_Z)
//        cameraTarget[0] -= 5;
//    else if (key == GLFW_KEY_C)
//        cameraTarget[0] += 5;
//    else if (key == GLFW_KEY_RIGHT)
//        cameraTarget[1] -= 1;
//    else if (key == GLFW_KEY_LEFT)
//        cameraTarget[1] += 1;
//    else if (key == GLFW_KEY_UP)
//        cameraTarget[2] -= 1;
//    else if (key == GLFW_KEY_DOWN)
//        cameraTarget[2] += 1;
//    else if (key == GLFW_KEY_O)
//        unifWater[mainUNIFMODELING] -= 0.1;
//    else if (key == GLFW_KEY_P)
//        unifWater[mainUNIFMODELING] += 0.1;
//    camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENSIZE,
//                  mainSCREENSIZE);
//    camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
//}

int main(void) {
    /* landscape preparation */
    double landMin, landMean, landMax;
    time_t t;
    srand((unsigned)time(&t));
    landFlat(100, 100, (double *)landData, 0.0);
    for (int i = 0; i < 32; i += 1)
        landFault(100, 100, (double *)landData, 1.5 - i * 0.04);
    for (int i = 0; i < 4; i += 1)
        landBlur(100, 100, (double *)landData);
    landStatistics(100, 100, (double *)landData, &landMin, &landMean,
                   &landMax);
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
    camLookAt(&cam, cameraTarget, cameraRho, cameraPhi, cameraTheta);
    camSetProjectionType(&cam, camPERSPECTIVE);
    camSetFrustum(&cam, M_PI / 6.0, cameraRho, 10.0, mainSCREENHEIGHT, mainSCREENWIDTH);
    /* Initialize the shader program before the mesh, so that the shader
    locations are already set up by the time the vertex array object is
    initialized. */
    if (initializeScene() != 0)
        return 4;
    while (glfwWindowShouldClose(window) == 0) {
        oldTime = newTime;
        newTime = getTime();
        if (floor(newTime) - floor(oldTime) >= 1.0)
            printf("main: %f frames/sec\n", 1.0 / (newTime - oldTime));
//        pixSetKeyDownHandler(handleKeyAny);
//        pixSetKeyRepeatHandler(handleKeyAny);
//        pixSetKeyUpHandler(handleKeyAny);
        render(oldTime, newTime);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    destroyScene();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}