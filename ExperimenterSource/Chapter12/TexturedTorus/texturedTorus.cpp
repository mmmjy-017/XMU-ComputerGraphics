////////////////////////////////////////////////////////////////////////
// texturedTorus.cpp
//
// This program, based on torus.cpp, maps a sugary texture onto a torus.
//
// Interaction:
// Press x, X, y, Y, z, Z to turn the torus.
//
// Sumanta Guha
//
// Texture Credits: See ExperimenterSource/Textures/TEXTURE_CREDITS.txt
/////////////////////////////////////////////////////////////////////////

#include <cmath>        
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/freeglut.h> 

#include "getBMP.h"

#define PI 3.14159265358979324
#define R 12.0 // Outer radius of torus
#define r 4.0 // Inner radius of torus.

// Globals.
static int p = 20; // Number of grid columns.
static int q = 20; // Number of grid rows
static float *vertices = NULL; // Vertex array of the mapped sample on the torus.
static float *textureCoordinates = NULL; // Texture co-ordinates array of the mapped sample on the torus.
static unsigned int texture[1]; // Array of texture indices.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate torus.

// Load images as textures. 
void loadTextures()
{
	imageFile *image[1];
	image[0] = getBMP("../../Textures/sugary.bmp");

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
		         GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

// Fuctions to map the grid vertex (u_i,v_j) to the mesh vertex (f(u_i,v_j), g(u_i,v_j), h(u_i,v_j)) on the torus.
float f(int i, int j)
{
	return ((R + r * cos((-1 + 2 * (float)j / q) * PI)) * cos((-1 + 2 * (float)i / p) * PI));
}

float g(int i, int j)
{
	return ((R + r * cos((-1 + 2 * (float)j / q) * PI)) * sin((-1 + 2 * (float)i / p) * PI));
}

float h(int i, int j)
{
	return (r * sin((-1 + 2 * (float)j / q) * PI));
}

// Routine to fill the vertex array with co-ordinates of the mapped sample points.
void fillVertexArray(void)
{
	int i, j, k;

	k = 0;
	for (j = 0; j <= q; j++)
		for (i = 0; i <= p; i++)
		{
			vertices[k++] = f(i, j);
			vertices[k++] = g(i, j);
			vertices[k++] = h(i, j);
		}
}

// Routine to fill the texture co-ordinates array with the texture co-ordinate values at the mapped sample points.
void fillTextureCoordArray(void)
{
	int i, j, k;

	k = 0;
	for (j = 0; j <= q; j++)
		for (i = 0; i <= p; i++)
		{
			textureCoordinates[k++] = (float)i / p;
			textureCoordinates[k++] = (float)j / q;
		}
}

// Initialization routine.
void setup(void)
{
	// Enable depth testing and the vertex and texture coordinate arrays.
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClearColor(1.0, 1.0, 1.0, 0.0); 

	// Create texture ids.
	glGenTextures(1, texture);

	// Load texture.
	loadTextures();

	// Specify how texture values combine with current surface color values.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Turn on OpenGL texturing.
	glEnable(GL_TEXTURE_2D);

	// Allocate space for vertex and texture coordinates array.
	vertices = new float[3 * (p + 1)*(q + 1)];
	textureCoordinates = new float[2 * (p + 1)*(q + 1)];

	// Set the array pointers.
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, textureCoordinates);

	// Fill the vertex and texture co-ordinates arrays.
	fillVertexArray();
	fillTextureCoordArray();

	// Cull the back faces of the torus.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

// Drawing routine.
void drawScene(void)
{
	int  i, j;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0.0, 0.0, 25.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Commands to turn the torus.
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	// Map the texture onto the torus.
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	for (j = 0; j < q; j++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (i = 0; i <= p; i++)
		{
			glArrayElement((j + 1)*(p + 1) + i);
			glArrayElement(j*(p + 1) + i);
		}
		glEnd();
	}

	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
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
	std::cout << "Press x, X, y, Y, z, Z to turn the torus." << std::endl;
}

// Main routine.
int main(int argc, char **argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("texturedTorus.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
