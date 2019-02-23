


#define meshglBUFFEROFFSET(bytes) ((GLubyte*) NULL + (bytes))

/* Feel free to read from this struct's members, but don't write to them except 
through the accessor functions. */
typedef struct meshglMesh meshglMesh;
struct meshglMesh {
	GLuint triNum, vertNum, attrDim, VAO;
	GLuint buffers[2];
};

/* Initializes the mesh from a non-OpenGL base mesh. After this function 
completes, the base mesh can be destroyed (because its data have been copied 
into GPU memory). When you are done using the mesh, don't forget to deallocate 
its resources using meshglDestroy. */
void meshglInitialize(meshglMesh *mesh, const meshMesh *base) {
	mesh->triNum = base->triNum;
	mesh->vertNum = base->vertNum;
	mesh->attrDim = base->attrDim;
	glGenBuffers(2, mesh->buffers);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertNum * mesh->attrDim * sizeof(GLdouble),
		(GLvoid *)base->vert, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->triNum * 3 * sizeof(GLuint),
		(GLvoid *)base->tri, GL_STATIC_DRAW);
	/* VAO stuff */
    glGenVertexArrays(1, &mesh->VAO);
    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
}

/* Immediately after meshglInitialize, the user must configure the attributes
using glEnableVertexAttribArray and glVertexAttribPointer. Immediately after
that configuration, the user must call this function to complete the
initialization of the mesh. */
void meshglFinishInitialization(meshglMesh *mesh) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->buffers[1]);
    glBindVertexArray(0);
}

/* Renders the mesh. Immediately before calling this function, you must 
'pre-render' with custom code to set up the attributes, such as
	glBindBuffer(GL_ARRAY_BUFFER, mesh->buffers[0]);
	glVertexPointer(3, GL_DOUBLE, mesh->attrDim * sizeof(GLdouble), 
		meshglBUFFEROFFSET(0));
	glNormalPointer(GL_DOUBLE, mesh->attrDim * sizeof(GLdouble), 
		meshglBUFFEROFFSET(0));
	glColorPointer(3, GL_DOUBLE, mesh->attrDim * sizeof(GLdouble), 
		meshglBUFFEROFFSET(3 * sizeof(GLdouble))); */
void meshglRender(const meshglMesh *mesh) {
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->triNum * 3, GL_UNSIGNED_INT, meshglBUFFEROFFSET(0));
    glBindVertexArray(0);
}

/* Releases the resources backing the mesh. Invoke this function when you are 
done using the mesh. */
void meshglDestroy(meshglMesh *mesh) {
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(2, mesh->buffers);

}


