////////////////////////////////////////////////////////////////////////////////         
// litTexturedCylinderClipped.cpp
//
// This program adds a clipping plane to litTexturedCylinderShaderized.cpp (only
// per-pixel lighting).
//
// Interaction:
// Press x, X, y, Y, z, Z to turn the cylinder.
// Press space to toggle between clipping on and off.
//
// Sumanta Guha
//
// Texture Credits: See ExperimenterSource/Textures/TEXTURE_CREDITS.txt
////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/freeglut.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "prepShader.h"
#include "cylinder.h"
#include "disc.h"
#include "light.h"
#include "material.h"
#include "getBMP.h"

using namespace glm;

static enum object { CYLINDER, DISC }; // VAO ids.
static enum buffer { CYL_VERTICES, CYL_INDICES, DISC_VERTICES }; // VBO ids.

// Globals.
static float Xangle = 210.0, Yangle = 230.0, Zangle = 0.0; // Angles to rotate the cylinder.

// Light properties.
static const Light light0 =
{
	vec4(0.0, 0.0, 0.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 1.5, 3.0, 0.0)
};

// Global ambient.
static const vec4 globAmb = vec4(0.2, 0.2, 0.2, 1.0);

// Front and back material properties.
static const Material canFandB =
{
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0),
	vec4(0.0, 0.0, 0.0, 1.0),
	50.0f
};

// Cylinder data.
static Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)];
static unsigned int cylIndices[CYL_LATS][2 * (CYL_LONGS + 1)];
static int cylCounts[CYL_LATS];
static void* cylOffsets[CYL_LATS];

// Disc data.
static Vertex discVertices[DISC_SEGS];

static mat4 modelViewMat, projMat;
static mat3 normalMat;

static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
normalMatLoc,
projMatLoc,
canLabelTexLoc,
canTopTexLoc,
objectLoc,
clipPlaneLoc, // Location of the vec4 clipPlane coordinates uniform.
buffer[4],
vao[3],
texture[5],
width,
height;

static imageFile *image[2]; // Local storage for bmp image data.

static uint isClipping = 0; // Is the clipping plane enabled?
static vec4 clipPlane = vec4(0.0, 0.0, -1.0, 0.99); // Clip plane coefficients.

// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);

	// Create shader program executable.
	vertexShaderId = setShader("vertex", "Shaders/vertexShader.glsl");
	fragmentShaderId = setShader("fragment", "Shaders/fragmentShader.glsl");
	programId = glCreateProgram();
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);
	glUseProgram(programId);

	// Initialize cylinder and disc.
	fillCylinder(cylVertices, cylIndices, cylCounts, cylOffsets);
	fillDiscVertexArray(discVertices);

	// Create VAOs and VBOs... 
	glGenVertexArrays(2, vao);
	glGenBuffers(3, buffer);

	// ...and associate cylinder data with vertex shader.
	glBindVertexArray(vao[CYLINDER]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[CYL_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cylVertices), cylVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[CYL_INDICES]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylIndices), cylIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	// ...and associate disc data with vertex shader.
	glBindVertexArray(vao[DISC]);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[DISC_VERTICES]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(discVertices), discVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(5);

	// Obtain modelview matrix, projection matrix, normal matrix and object uniform locations.
	modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
	projMatLoc = glGetUniformLocation(programId, "projMat");
	normalMatLoc = glGetUniformLocation(programId, "normalMat");
	objectLoc = glGetUniformLocation(programId, "object");

	// Obtain light property uniform locations and set values.
	glUniform4fv(glGetUniformLocation(programId, "light0.ambCols"), 1, &light0.ambCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.difCols"), 1, &light0.difCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.specCols"), 1, &light0.specCols[0]);
	glUniform4fv(glGetUniformLocation(programId, "light0.coords"), 1, &light0.coords[0]);

	// Obtain global ambient uniform location and set value.
	glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);

	// Obtain material property uniform locations and set values.
	glUniform4fv(glGetUniformLocation(programId, "canFandB.ambRefl"), 1, &canFandB.ambRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "canFandB.difRefl"), 1, &canFandB.difRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "canFandB.specRefl"), 1, &canFandB.specRefl[0]);
	glUniform4fv(glGetUniformLocation(programId, "canFandB.emitCols"), 1, &canFandB.emitCols[0]);
	glUniform1f(glGetUniformLocation(programId, "canFandB.shininess"), canFandB.shininess);

	// Load the images.
	image[0] = getBMP("../../Textures/canLabel.bmp");
	image[1] = getBMP("../../Textures/canTop.bmp");

	// Create texture ids.
	glGenTextures(2, texture);

	// Bind can label image.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	canLabelTexLoc = glGetUniformLocation(programId, "canLabelTex");
	glUniform1i(canLabelTexLoc, 0);

	// Bind can top image.
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->width, image[1]->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	canTopTexLoc = glGetUniformLocation(programId, "canTopTex");
	glUniform1i(canTopTexLoc, 1);

	// Obtain clip plane uniform location and set value.
	clipPlaneLoc = glGetUniformLocation(programId, "clipPlane");
	glUniform4fv(clipPlaneLoc, 1, &clipPlane[0]);
}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable/disable clipping.
	if (isClipping) glEnable(GL_CLIP_PLANE0);
	else glDisable(GL_CLIP_PLANE0);

	// Calculate and update projection matrix.
	projMat = perspective(radians(60.0f), (float)width / (float)height, 1.0f, 50.0f);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

	// Calculate and update modelview matrix.
	modelViewMat = mat4(1.0);
	modelViewMat = lookAt(vec3(0.0, 0.0, 3.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

	// Calculate and update modelview matrix.
	modelViewMat = rotate(modelViewMat, radians(Zangle), vec3(0.0, 0.0, 1.0));
	modelViewMat = rotate(modelViewMat, radians(Yangle), vec3(0.0, 1.0, 0.0));
	modelViewMat = rotate(modelViewMat, radians(Xangle), vec3(1.0, 0.0, 0.0));
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));

	// Calculate and update normal matrix.
	normalMat = transpose(inverse(mat3(modelViewMat)));
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));

	// Draw cylinder.
	glUniform1ui(objectLoc, CYLINDER);
	glBindVertexArray(vao[CYLINDER]);
	glMultiDrawElements(GL_TRIANGLE_STRIP, cylCounts, GL_UNSIGNED_INT, (const void **)cylOffsets, CYL_LATS);

	// Draw disc.
	glUniform1ui(objectLoc, DISC);
	glBindVertexArray(vao[DISC]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, DISC_SEGS);

	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w; height = h;
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case ' ':
		isClipping = isClipping ? 0 : 1;
		glutPostRedisplay();
		break;
	case 'x':
		Xangle += 5.0;
		if (Xangle > 360.0) Xangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'X':
		Xangle -= 5.0;
		if (Xangle < 0.0) Xangle += 360.0;
		glutPostRedisplay();
		break;
	case 'y':
		Yangle += 5.0;
		if (Yangle > 360.0) Yangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Y':
		Yangle -= 5.0;
		if (Yangle < 0.0) Yangle += 360.0;
		glutPostRedisplay();
		break;
	case 'z':
		Zangle += 5.0;
		if (Zangle > 360.0) Zangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'Z':
		Zangle -= 5.0;
		if (Zangle < 0.0) Zangle += 360.0;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Press x, X, y, Y, z, Z to turn the cylinder." << std::endl
		<< "Press space to toggle between clipping on and off." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("litTexturedCylinderClipped.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}

