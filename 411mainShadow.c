//clang 411mainShadow.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation -Wno-deprecated
#include <time.h>
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
#include "410meshgl.c"
#include "360texture.c"
#include "370body.c"
#include "140landscape.c"
#include "400shadow.c"

#define BUFFER_OFFSET(bytes) ((GLubyte*) NULL + (bytes))

#define UNIFNUM 11
#define ATTRNUM 3

#define UNIFVIEWING 0
#define UNIFMODELING 1
#define UNIFCLIGHT 2
#define UNIFDLIGHT 3
#define UNIFCAMB 4
#define UNIFPCAM 5
#define UNIFTEX 6
#define UNIFPCAMERA0 7
#define UNIFCLIGHT0 8
#define UNIFDSPOT 9
#define UNIFANGLE 10

#define UNIFNUM0 3
#define ATTRNUM0 1
#define UNIF0VIEWING 0
#define UNIF0MODELING 1
#define UNIF0COLOR 2
#define ATTR0POSITION 0

#define ATTRPOSITION 0
#define ATTRNORMAL 1
#define ATTRST 2

shaShading sha;
shaShading sha0;
const GLchar *uniformNames[UNIFNUM] = {"viewing", "modeling", "cLight", "dLight","cAmbient",
                                       "pCamera", "tex0", "pCamera0", "cLight0","dSpot","angle",
};
const GLchar **unifNames = uniformNames;
const GLchar *attributeNames[ATTRNUM] = {"position","normal","coor"};
const GLchar **attrNames = attributeNames;
const GLchar *uniformNames0[UNIFNUM0] = {"viewing", "modeling", "color"};
const GLchar **unifNames0 = uniformNames0;
const GLchar *attributeNames0[ATTRNUM0] = {"position"};
const GLchar **attrNames0 = attributeNames0;
GLdouble angle = 0.0;
//isoIsometry modeling;
camCamera cam;
camCamera cam0;
meshglMesh landgl;
meshglMesh rockgl;
meshglMesh watergl;
meshglMesh grassgl;
meshglMesh tree0gl;
meshMesh land;
meshMesh rock;
meshMesh water;
meshMesh grass;
meshMesh tree0;
texTexture tex0;
texTexture tex1;
texTexture tex2;
texTexture tex3;
bodyBody landb;
bodyBody rockb;
bodyBody waterb;
bodyBody grassb;
bodyBody tree0b;
bodyBody tree1b;
shadowMap shadow;
const texTexture *textures[4] = {&tex0,&tex1,&tex2,&tex3};
//const texTexture **tex = textures;
int landNum = 100;
double landData[100][100];
double waterData[4];

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

void meshini(meshglMesh *meshgl, bodyBody *body) {
    bodySetMesh(body, meshgl);
    glEnableVertexAttribArray(sha.attrLocs[ATTRPOSITION]);
    glVertexAttribPointer(sha.attrLocs[ATTRPOSITION], 3, GL_DOUBLE, GL_FALSE,
                          body->mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0));
    glEnableVertexAttribArray(sha.attrLocs[ATTRST]);
    glVertexAttribPointer(sha.attrLocs[ATTRST], 2, GL_DOUBLE, GL_FALSE,
                          body->mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(3 * sizeof(GLdouble)));
    glEnableVertexAttribArray(sha.attrLocs[ATTRNORMAL]);
    glVertexAttribPointer(sha.attrLocs[ATTRNORMAL], 3, GL_DOUBLE, GL_FALSE,
                          body->mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(5 * sizeof(GLdouble)));
    meshglContinueInitialization(body->mesh);
    glEnableVertexAttribArray(sha0.attrLocs[ATTR0POSITION]);
    glVertexAttribPointer(sha0.attrLocs[ATTR0POSITION], 3, GL_DOUBLE, GL_FALSE,
                          body->mesh->attrDim * sizeof(GLdouble), BUFFER_OFFSET(0 * sizeof(GLdouble)));
    //meshglContinueInitialization(body->mesh);
    meshglFinishInitialization(body->mesh);
}

int initializeMesh(void) {
    //meshInitializeCapsule(&mesh, 0.6, 1.5, 50, 50);
    if (meshInitializeLandscape(&land, landNum, landNum, 1.0, (double *)landData) != 0
        || meshInitializeDissectedLandscape(&grass, &land, M_PI / 4.0, 1) != 0
        || meshInitializeDissectedLandscape(&rock, &land, M_PI / 4.0, 0) != 0
        || meshInitializeLandscape(&water, 2, 2, landNum - 1.0, (double *)waterData) != 0
        || meshInitializeSphere(&tree0, 6, 20, 20))
        return 1;
    meshglInitialize(&landgl, &land);
    meshDestroy(&land);
    meshini(&landgl, &landb);
    meshglInitialize(&watergl, &water);
    meshDestroy(&water);
    meshini(&watergl, &waterb);
    meshglInitialize(&rockgl, &rock);
    meshDestroy(&rock);
    meshini(&rockgl, &rockb);
    meshglInitialize(&tree0gl, &tree0);
    meshDestroy(&tree0);
    meshini(&tree0gl, &tree0b);
    meshini(&tree0gl, &tree1b);
    meshglInitialize(&grassgl, &grass);
    meshDestroy(&grass);
    meshini(&grassgl, &grassb);
    return 0;
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
    /* The two matrices will be sent to the shaders as uniforms. */
    GLchar vertexCode[] = "\
        #version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		in vec2 coor;\
        in vec3 normal;\
		out vec2 st;\
        out vec3 dNormal;\
        out vec3 pFragment;\
		void main() {\
            st = coor;\
			vec4 world = modeling * vec4(position, 1.0);\
            pFragment = vec3(world);\
            gl_Position = viewing * modeling * vec4(position, 1.0);\
            dNormal = vec3(modeling * vec4(normal, 0.0));\
		}";
    GLchar fragmentCode[] = "\
        #version 140\n\
		in vec2 st;\
        uniform vec3 dLight;\
        uniform vec3 cLight;\
        uniform vec3 cAmbient;\
        uniform vec3 dSpot;\
        uniform vec3 angle;\
        uniform vec3 pCamera;\
        uniform vec3 pCamera0;\
        uniform vec3 cLight0;\
        uniform sampler2D tex0;\
        in vec3 dNormal;\
        in vec3 pFragment;\
        out vec4 fragColor;\
		void main() {\
        vec3 rgbFromTex = vec3(texture(tex0, st));\
        vec3 dCamera = normalize(pCamera - pFragment);\
        vec3 dNorm = normalize(dNormal);\
        float iDiff = dot(dLight, dNorm);\
        if (iDiff < 0.0)\
            iDiff = 0.0;\
        vec3 cDiff = rgbFromTex;\
        vec3 diffuse = iDiff * cDiff * cLight;\
        vec3 dReft = 2.0 * iDiff * dNorm - dLight;\
        vec3 cSpec = vec3(1.0, 1.0, 1.0);\
        float iSpec = dot(dCamera ,dReft);\
        float shininess = 15.0;\
        if (iSpec < 0.0 || iDiff == 0.0)\
            iSpec = 0.0;\
        iSpec = pow(iSpec, shininess);\
        vec3 Spec =  iSpec * cSpec * cLight;\
        vec3 ambient =  cDiff * cAmbient;\
        vec3 dLight0 = normalize(pCamera0 - pFragment);\
        vec3 Spec0 = vec3(0.0,0.0,0.0);\
        if(dot(dLight0, dSpot) >= angle.x) {\
        float iDiff0 = dot(dLight0, dNorm);\
        if (iDiff0 < 0.0)\
            iDiff0 = 0.0;\
        vec3 cDiff0 = rgbFromTex;\
        vec3 diffuse0 = iDiff0 * cDiff0 * cLight0;\
        vec3 dReft0 = 2.0 * iDiff0 * dNorm - dLight0;\
        vec3 cSpec0 = vec3(1.0, 1.0, 1.0);\
        float iSpec0 = dot(dCamera ,dReft0);\
        if (iSpec0 < 0.0 || iDiff0 == 0.0)\
            iSpec0 = 0.0;\
        iSpec0 = pow(iSpec0, shininess);\
        Spec0 =  iSpec0 * cSpec0 * cLight0 + diffuse0; }\
        fragColor = vec4(ambient+diffuse+Spec+Spec0, 1.0);\
		}";
    GLchar vertexCode0[] = "\
		#version 140\n\
		uniform mat4 viewing;\
		uniform mat4 modeling;\
		in vec3 position;\
		void main() {\
			gl_Position = viewing * modeling * vec4(position, 1.0);\
		}";
    GLchar fragmentCode0[] = "\
		#version 140\n\
		uniform vec3 color;\
		out vec4 fragColor;\
		void main() {\
fragColor = vec4(color, 1.0);\
		}";
    return shaInitialize(&sha, vertexCode, fragmentCode, UNIFNUM, unifNames, ATTRNUM, attrNames)
    + shaInitialize(&sha0, vertexCode0, fragmentCode0, UNIFNUM0, unifNames0, ATTRNUM0, attrNames0);
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

void renderbody(GLuint v) {
    texRender(waterb.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    meshglRender(waterb.mesh,v);
    texUnrender(waterb.tex[0], GL_TEXTURE0);
    texRender(rockb.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    meshglRender(rockb.mesh,v);
    texUnrender(rockb.tex[0], GL_TEXTURE0);
    texRender(landb.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    meshglRender(landb.mesh,v);
    texUnrender(landb.tex[0], GL_TEXTURE0);
    texRender(grassb.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    meshglRender(grassb.mesh,v);
    texUnrender(grassb.tex[0], GL_TEXTURE0);
    texRender(tree0b.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    GLdouble modeltree[4][4];
    isoGetHomogeneous(&tree0b.isometry, modeltree);
    uniformMatrix44(modeltree, sha.unifLocs[UNIFMODELING]);
    meshglRender(tree0b.mesh,v);
    texUnrender(tree0b.tex[0], GL_TEXTURE0);
    texRender(tree1b.tex[0], GL_TEXTURE0, 0, sha.unifLocs[UNIFTEX]);
    isoGetHomogeneous(&tree1b.isometry, modeltree);
    uniformMatrix44(modeltree, sha.unifLocs[UNIFMODELING]);
    meshglRender(tree1b.mesh,v);
    texUnrender(tree1b.tex[0], GL_TEXTURE0);
}

void renderIso(double oldTime, double newTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(sha.program);
    glUseProgram(sha0.program);
    /* Send our own modeling transformation M to the shaders. */
    GLdouble trans[3] = {-15.0, 50.0, -25.0};
    GLdouble transtree0[3] = {70.0,40.0,10.0};
    GLdouble transtree1[3] = {45.0,60.0, -1.0};
    isoSetTranslation(&grassb.isometry, trans);
    isoSetTranslation(&landb.isometry, trans);
    isoSetTranslation(&rockb.isometry, trans);
    isoSetTranslation(&waterb.isometry, trans);
    isoSetTranslation(&tree0b.isometry, transtree0);
    isoSetTranslation(&tree1b.isometry, transtree1);
    //angle += 0.3 * (newTime - oldTime);
    GLdouble axis[3] = {1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0)};
    GLdouble rot[3][3];
    mat33AngleAxisRotation(angle, axis, rot);
    isoSetRotation(&grassb.isometry, rot);
    isoSetRotation(&landb.isometry, rot);
    isoSetRotation(&rockb.isometry, rot);
    isoSetRotation(&waterb.isometry, rot);
    isoSetRotation(&tree0b.isometry, rot);
    isoSetRotation(&tree1b.isometry, rot);
}

void renderRegularly(double oldTime, double newTime) {
    renderIso(oldTime, newTime);
    GLdouble model[4][4];
    isoGetHomogeneous(&grassb.isometry, model);
    uniformMatrix44(model, sha.unifLocs[UNIFMODELING]);
    GLdouble viewing[4][4];
    GLdouble cLight[3] = {0.2,0.2,0.2};
    GLdouble Light[3] = {100.0,150.0,200.0}, dLight[3];
    vecUnit(3, Light, dLight);
    GLdouble cLight0[3] = {1.0,1.0,1.0};
    GLdouble pCamera0[3] = {70.0,40.0,10.0};
    GLdouble cAmbient[3] = {0.0,0.1,0.1};
    GLdouble angle[3] = {cos(M_PI/10),0.0,0.0};
    GLdouble dSpot[3];
    cam0.isometry.translation[0] = pCamera0[0];
    cam0.isometry.translation[1] = pCamera0[1];
    cam0.isometry.translation[2] = pCamera0[2];
    //apply the camera's rotation
    GLdouble temp[3] = {0,0,1};
    isoRotateVector(&cam0.isometry, temp, dSpot);
    uniformVector3(cLight, sha.unifLocs[UNIFCLIGHT]);
    uniformVector3(dLight, sha.unifLocs[UNIFDLIGHT]);
    uniformVector3(cLight0, sha.unifLocs[UNIFCLIGHT0]);
    uniformVector3(cam0.isometry.translation, sha.unifLocs[UNIFPCAMERA0]);
    uniformVector3(cAmbient, sha.unifLocs[UNIFCAMB]);
    uniformVector3(dSpot, sha.unifLocs[UNIFDSPOT]);
    uniformVector3(angle, sha.unifLocs[UNIFANGLE]);
    uniformVector3(cam.isometry.translation, sha.unifLocs[UNIFPCAM]);
    camGetProjectionInverseIsometry(&cam, viewing);
    uniformMatrix44(viewing, sha.unifLocs[UNIFVIEWING]);
    renderbody(0);
}

void renderShadowly(double oldTime, double newTime) {
    renderIso(oldTime, newTime);
    GLdouble model[4][4];
    isoGetHomogeneous(&grassb.isometry, model);
    uniformMatrix44(model, sha0.unifLocs[UNIF0MODELING]);
    GLdouble viewing[4][4];
    GLdouble pCamera0[3] = {70.0,40.0,10.0};
    //Set the position of cam0
    cam0.isometry.translation[0] = pCamera0[0];
    cam0.isometry.translation[1] = pCamera0[1];
    cam0.isometry.translation[2] = pCamera0[2];
    camGetProjectionInverseIsometry(&cam0, viewing);
    uniformMatrix44(viewing, sha0.unifLocs[UNIF0VIEWING]);
    GLdouble color[3] = {0.8,0.8,0.8};
    uniformVector3(color, sha0.unifLocs[UNIF0COLOR]);
    meshglRender(waterb.mesh,1);
    meshglRender(rockb.mesh,1);
    GLdouble color0[3] = {0.3,1.0,0.9};
    uniformVector3(color0, sha0.unifLocs[UNIF0COLOR]);
    meshglRender(landb.mesh,1);
    meshglRender(grassb.mesh,1);
    GLdouble color1[3] = {0.6,0.2,0.6};
    uniformVector3(color1, sha0.unifLocs[UNIF0COLOR]);
    GLdouble modeltree[4][4];
    isoGetHomogeneous(&tree0b.isometry, modeltree);
    uniformMatrix44(modeltree, sha0.unifLocs[UNIFMODELING]);
    meshglRender(tree0b.mesh,1);
    isoGetHomogeneous(&tree1b.isometry, modeltree);
    uniformMatrix44(modeltree, sha0.unifLocs[UNIFMODELING]);
    meshglRender(tree1b.mesh,1);
}

void render(double oldTime, double newTime) {
    //renderShadowly(oldTime, newTime);
    renderRegularly(oldTime, newTime);
}

int initializeTexture() {
    if(texInitializeFile(&tex0, "rock.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0
       || texInitializeFile(&tex1, "water.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0
       || texInitializeFile(&tex2, "grass.jpeg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0
       || texInitializeFile(&tex3, "leaves.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0)
        return 1;
    return 0;
}

int initializeBody() {
    if(bodyInitialize(&rockb, 0, 1)!= 0
       || bodyInitialize(&waterb, 0, 1)!= 0
       || bodyInitialize(&grassb, 0, 1)!= 0
       || bodyInitialize(&landb, 0, 1)!= 0
       || bodyInitialize(&tree0b, 0, 1)!= 0
       || bodyInitialize(&tree1b, 0, 1)!= 0){
        return 1;
    }
    bodySetTexture(&rockb, 0, &tex0);
    bodySetTexture(&waterb, 0, &tex1);
    bodySetTexture(&grassb, 0, &tex2);
    bodySetTexture(&landb, 0, &tex2);
    bodySetTexture(&tree0b, 0, &tex3);
    bodySetTexture(&tree1b, 0, &tex3);
    return 0;
}


void iniUniform(void) {
    GLdouble cLight[3] = {0.2,0.2,0.2};
    GLdouble Light[3] = {100.0,150.0,200.0}, dLight[3];
    vecUnit(3, Light, dLight);
    GLdouble cLight0[3] = {1.0,1.0,1.0};
    GLdouble pCamera0[3] = {70.0,40.0,10.0};
    GLdouble cAmbient[3] = {0.0,0.1,0.1};
    GLdouble angle[3] = {cos(M_PI/10),0.0,0.0};
    GLdouble dSpot[3];
    cam0.isometry.translation[0] = pCamera0[0];
    cam0.isometry.translation[1] = pCamera0[1];
    cam0.isometry.translation[2] = pCamera0[2];
}

int main(void) {
    //int landNum = 100;
    //double landData[landNum][landNum];
    double landMin, landMean, landMax;
    time_t t;
    int i;
    srand((unsigned)time(&t));
    landFlat(landNum, landNum, (double *)landData, 0.0);
    for (i = 0; i < 32; i += 1)
        landFault(landNum, landNum, (double *)landData, 1.5 - i * 0.04);
    for (i = 0; i < 4; i += 1)
        landBlur(landNum, landNum, (double *)landData);
    landStatistics(landNum, landNum, (double *)landData, &landMin, &landMean,
                   &landMax);
    //double waterData[4] = {landMin, landMin, landMin, landMin};
    waterData[0] = landMin;
    waterData[1] = landMin;
    waterData[2] = landMin;
    waterData[3] = landMin;
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
    window = glfwCreateWindow(768, 512, "Learning OpenGL 2.0", NULL, NULL);
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
    /* Configure the camera once and for all. */
    double target[3] = {0.0, 0.0, 0.0};
    camLookAt(&cam, target, 255.0, M_PI / 3.0, -M_PI / 4.0);
    camLookAt(&cam0, target, 255.0, M_PI / 3.0, -M_PI / 4.0);
    camSetProjectionType(&cam, camPERSPECTIVE);
    camSetProjectionType(&cam0, camPERSPECTIVE);
    camSetFrustum(&cam, M_PI / 6.0, 255.0, 10.0, 768, 512);
    camSetFrustum(&cam0, M_PI / 4.0, 255.0, 10.0, 256, 256);
    /* The rest of the program is identical to the preceding tutorial. */
    if (initializeShaderProgram() != 0)
        return 4;
    if(initializeTexture()!= 0) {
        return 6;
    }
    if(initializeBody()!=0) {
        return 7;
    }
    if(initializeMesh()!= 0) {
        return 5;
    }
    if(shadowInitialize(&shadow, 256,256)!= 0) {
        return 6;
    }
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
    shaDestroy(&sha0);
    bodyDestroy(&waterb);
    bodyDestroy(&landb);
    bodyDestroy(&grassb);
    bodyDestroy(&rockb);
    shadowDestroy(&shadow);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}





