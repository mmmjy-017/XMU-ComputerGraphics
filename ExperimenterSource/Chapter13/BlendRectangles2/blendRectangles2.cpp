////////////////////////////////////////////////////////////////////////////////        
// blendRectangles2.cpp
//
// This program builds on blendRectangles1.cpp adding an opaque green rectangle. 
//
// Sumanta Guha.
//////////////////////////////////////////////////////////////////////////////// 

#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h> 

// Function to draw a blue rectangle at z = 0.1 with alpha = 0.5 (translucent).
void drawBlueRectangle(void)
{
	glColor4f(0.0, 0.0, 1.0, 0.5);
	glBegin(GL_POLYGON);
	glVertex3f(-40.0, -35.0, 0.1);
	glVertex3f(40.0, -35.0, 0.1);
	glVertex3f(40.0, -20.0, 0.1);
	glVertex3f(-40.0, -20.0, 0.1);
	glEnd();
}

// Function to draw a green rectangle at z = 0.3 with alpha = 1.0 (opaque).
void drawGreenRectangle(void)
{
	glColor4f(0.0, 1.0, 0.0, 1.0);
	glBegin(GL_POLYGON);
	glVertex3f(30.0, 20.0, 0.3);
	glVertex3f(15.0, 35.0, 0.3);
	glVertex3f(-40.0, -20.0, 0.3);
	glVertex3f(-25.0, -35.0, 0.3);
	glEnd();
}

// Function to draw a red rectangle at z = 0.5 with alpha = 0.5 (translucent).
void drawRedRectangle(void)
{
	glColor4f(1.0, 0.0, 0.0, 0.5);
	glBegin(GL_POLYGON);
	glVertex3f(30.0, 35.0, 0.5);
	glVertex3f(15.0, 35.0, 0.5);
	glVertex3f(15.0, -45.0, 0.5);
	glVertex3f(30.0, -45.0, 0.5);
	glEnd();
}

// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST); // Enable depth testing.

	glEnable(GL_BLEND); // Enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Specify blending parameters.
}

// Drawing routine.
void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);

	drawRedRectangle(); // Red rectangle closest to viewer, translucent.
	drawGreenRectangle(); // Green rectangle second closest, opaque.
	drawBlueRectangle(); // Blue rectangle farthest, translucent.

	glFlush();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-50.0, 50.0, -50.0, 50.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

// Main routine.
int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("blendRectangles2.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}
