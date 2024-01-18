//////////////////////////////////////////////////////////
// tessellatedCurve.cpp
//
// Forward-compatible core GL 4.3 program drawing a Bezier 
// curve with use of tessellation shaders.
//
// Interaction:
// Press the up/down arrow keys to increase/decrease
// number of Bezier curve segments.
//
// Sumanta Guha
//////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <fstream>

#include <GL/glew.h>
#include <GL/freeglut.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "prepShader.h"
#include "vertex.h"

using namespace glm;

static enum object {INPUT_SET}; // VAO ids.
static enum buffer {INPUT_SET_VERTICES}; // VBO ids.

// Globals.
static Vertex inputVertices[3] = 
{ 
   { vec4(-0.95, 0.0, 0.0, 1.0) },
   { vec4(0.0, 0.95, 0.0, 1.0) },
   { vec4(0.95, 0.0, 0.0, 1.0) } 
};

static unsigned int
   programId,
   vertexShaderId,
   tessControlShaderId,
   tessEvaluationShaderId,
   fragmentShaderId,
   tessLevelOuter1Loc,
   buffer[1], 
   vao[1],
   pipeline;

static float tessLevelOuter1 = 5.0; // Value of gl_TessLevelOuter[1].

// Initialization routine.
void setup(void) 
{
   glClearColor(1.0, 1.0, 1.0, 0.0); 

   // Create shader program executable.
   vertexShaderId = setShader("vertex", "Shaders/vertexShader.glsl");
   tessControlShaderId = setShader("tessControl", "Shaders/tessControlShader.glsl");
   tessEvaluationShaderId = setShader("tessEvaluation", "Shaders/tessEvaluationShader.glsl");
   fragmentShaderId = setShader("fragment", "Shaders/fragmentShader.glsl");
   programId = glCreateProgram(); 
   glAttachShader(programId, vertexShaderId); 
   glAttachShader(programId, fragmentShaderId);
   glLinkProgram(programId); 
   glUseProgram(programId); 

   // Create VAOs and VBOs... 
   glGenVertexArrays(1, vao);
   glGenBuffers(1, buffer); 

   // ...and associate points data with vertex shader.
   glBindVertexArray(vao[INPUT_SET]);
   glBindBuffer(GL_ARRAY_BUFFER, buffer[INPUT_SET_VERTICES]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(inputVertices), inputVertices, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
   glEnableVertexAttribArray(0);

   // Set value of gl_TessLevelOuter[1] in TCS.
   tessLevelOuter1Loc = glGetUniformLocation(programId, "tessLevelOuter1");

   // Point size.
   glPointSize(5.0); 

   // Input patch size.
   glPatchParameteri(GL_PATCH_VERTICES, 3); 
}

// Drawing routine.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);

   // Attach TCS and TES...
   glAttachShader(programId, tessControlShaderId); 
   glAttachShader(programId, tessEvaluationShaderId);  
   glLinkProgram(programId); 

   // ...and draw Bezier curve.
   tessLevelOuter1Loc = glGetUniformLocation(programId, "tessLevelOuter1");   
   glUniform1f(tessLevelOuter1Loc, tessLevelOuter1);
   glDrawArrays(GL_PATCHES, 0, 3);

   // Detach TCS and TES...
   glDetachShader(programId, tessControlShaderId); 
   glDetachShader(programId, tessEvaluationShaderId);
   glLinkProgram(programId); 

   // ...and draw points.
   glDrawArrays(GL_POINTS, 0, 3);
   
   glFlush();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, w, h); 
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch(key) 
   {
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
   if (key == GLUT_KEY_UP) ++tessLevelOuter1;
   if (key == GLUT_KEY_DOWN) if (tessLevelOuter1 > 1.0) --tessLevelOuter1;
   
   glutPostRedisplay();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   std::cout << "Interaction:" << std::endl;
   std::cout << "Press the up/down arrow keys to increase/decrease" << std::endl
	    << "number of Bezier curve segments." << std::endl;
}

// Main routine.
int main(int argc, char **argv) 
{
   printInteraction();
   glutInit(&argc, argv);

   glutInitContextVersion(4, 3);
   glutInitContextProfile(GLUT_CORE_PROFILE);
   glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA); 
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100); 
   glutCreateWindow("tessellatedCurve.cpp");
   glutDisplayFunc(drawScene); 
   glutReshapeFunc(resize);  
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);

   glewExperimental = GL_TRUE;
   glewInit();

   setup(); 
   
   glutMainLoop(); 
}

