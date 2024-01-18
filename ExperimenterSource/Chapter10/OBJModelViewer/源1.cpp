//
// OBJmodelViewer.cpp
//
// An object defined in an external Wavefront OBJ file is loaded and displayed. 
// Only vertex and face lines are read. All other lines are ignored. Vertex lines 
// are assumed to have only x, y and z coordinate values. The (optional) w value 
// is ignored if present. Within a face line only vertex indices are read. Texture 
// and normal indices are allowed but ignored. Face lines can have more than three 
// vertices. If a face line has more than three vertices the output is a fan 
// triangulation. Therefore, the mesh generated consists of only triangles.
//
// Interaction:
// Press x, X, y, Y, z, Z to turn the object.
//
// Sumanta Guha.
//

#include <iostream>
#include <sstream>    
#include <string> 
#include <fstream> 
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h> 
using namespace std;
// Globals.
static std::vector<float> verticesVector; // Vector to read in vertex x, y and z values fromt the OBJ file.
static std::vector<int> facesVector; // Vector to read in face vertex indices from the OBJ file.
static float* vertices = NULL;  // Vertex array of the object x, y, z values.
static int* faces = NULL; // Face (triangle) vertex indices.
static int numIndices; // Number of face vertex indices.
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate the object.

// Routine to read a Wavefront OBJ file. 
// Only vertex and face lines are processed. All other lines,including texture, 
// normal, material, etc., are ignored.
// Vertex lines are assumed to have only x, y, z coordinate values and no w value.
// If a w value is present it is ignored.
// Within a face line only vertex indices are read. Texture and normal indices are 
// allowed but ignored. Face lines can have more than three vertices.
//
// OUTPUT: The vertex coordinate values are written to a vector "verticesVector" 
// (this name is hardcoded in the routine so the calling program should take note). 
// The face vertex indices are written to a vector "facesVector" (hardcoded name).
// Faces with more than three vertices are fan triangulated about the first vertex
// and the triangle indices written. In other words, output faces are all triangles.
// All vertex indices are decremented by 1 to make the index range start from 0.
void loadOBJ(std::string fileName)
{
    std::string line;
    int count, vertexIndex1, vertexIndex2, vertexIndex3;
    float coordinateValue;
    char currentCharacter, previousCharacter;

    // Open the OBJ file.
    std::ifstream inFile(fileName.c_str(), std::ifstream::in);

    // Read successive lines.
    while (getline(inFile, line))
    {
        // Line has vertex data.
        if (line.substr(0, 2) == "v ")
        {
            // Initialize a string from the character after "v " to the end.
            std::istringstream currentString(line.substr(2));

            // Read x, y and z values. The (optional) w value is not read. 
            for (count = 1; count <= 3; count++)
            {
                currentString >> coordinateValue;
                verticesVector.push_back(coordinateValue);
            }
        }

        // Line has face data.
        else if (line.substr(0, 2) == "f ")
        {
            // Initialize a string from the character after "f " to the end.
            std::istringstream currentString(line.substr(2));

            // Strategy in the following to detect a vertex index within a face line is based on the
            // fact that vertex indices are exactly those that follow a white space. Texture and
            // normal indices are ignored.
            // Moreover, from the third vertex of a face on output one triangle per vertex, that
            // being the next triangle in a fan triangulation of the face about the first vertex.
            previousCharacter = ' ';
            count = 0;
            while (currentString.get(currentCharacter))
            {
                // Stop processing line at comment.
                if ((previousCharacter == '#') || (currentCharacter == '#')) break;

                // Current character is the start of a vertex index.
                if ((previousCharacter == ' ') && (currentCharacter != ' '))
                {
                    // Move the string cursor back to just before the vertex index.
                    currentString.unget();

                    // Read the first vertex index, decrement it so that the index range is from 0, increment vertex counter.
                    if (count == 0)
                    {
                        currentString >> vertexIndex1;
                        vertexIndex1--;
                        count++;
                    }

                    // Read the second vertex index, decrement it, increment vertex counter.
                    else if (count == 1)
                    {
                        currentString >> vertexIndex2;
                        vertexIndex2--;
                        count++;
                    }

                    // Read the third vertex index, decrement it, increment vertex counter AND output the first triangle.
                    else if (count == 2)
                    {
                        currentString >> vertexIndex3;
                        vertexIndex3--;
                        count++;
                        facesVector.push_back(vertexIndex1);
                        facesVector.push_back(vertexIndex2);
                        facesVector.push_back(vertexIndex3);
                    }

                    // From the fourth vertex and on output the next triangle of the fan.
                    else
                    {
                        vertexIndex2 = vertexIndex3;
                        currentString >> vertexIndex3;
                        vertexIndex3--;
                        facesVector.push_back(vertexIndex1);
                        facesVector.push_back(vertexIndex2);
                        facesVector.push_back(vertexIndex3);
                    }

                    // Begin the process of detecting the next vertex index just after the vertex index just read.
                    currentString.get(previousCharacter);
                }

                // Current character is not the start of a vertex index. Move ahead one character.
                else previousCharacter = currentCharacter;
            }
        }

        // Nothing other than vertex and face data is processed.
        else
        {
        }
    }

    // Close the OBJ file.
    inFile.close();
}
vector<double> alpha;//计算法向量和光线投射方向点积的值
void getalpha(void)//计算出点积的函数
{
    for (int i = 0; i < facesVector.size(); i = i + 3)
    {
        double a1 = double(vertices[(faces[i + 1]) * 3]) - double(vertices[(faces[i]) * 3]);//第一个向量的 x y z为a1 b1 c1
        double b1 = double(vertices[(faces[i + 1]) * 3 + 1]) - double(vertices[(faces[i]) * 3 + 1]);
        double c1 = double(vertices[(faces[i + 1]) * 3 + 2]) - double(vertices[(faces[i]) * 3 + 2]);
        double a2 = double(vertices[(faces[i + 2]) * 3]) - double(vertices[(faces[i]) * 3]);//第二个向量的x y z 为a2 b2 c2
        double b2 = double(vertices[(faces[i + 2]) * 3 + 1]) - double(vertices[(faces[i]) * 3 + 1]);
        double c2 = double(vertices[(faces[i + 2]) * 3 + 2]) - double(vertices[(faces[i]) * 3 + 2]);
        double face[3] = { b1 * c2 - c1 * b2,c1 * a2 - c2 * a1,a1 * b2 - a2 * b1 };//叉乘得到这个面的法向量
        face[0] = face[0] / (sqrt(face[0] * face[0] + face[1] * face[1] + face[2] * face[2]));//进行单位化
        face[1] = face[1] / (sqrt(face[0] * face[0] + face[1] * face[1] + face[2] * face[2]));
        face[2] = face[2] / (sqrt(face[0] * face[0] + face[1] * face[1] + face[2] * face[2]));
        double k = (face[0] * 0 + face[1] * 0 + face[2] * 1);//点乘得到点积
        alpha.push_back(k);//加入数组

    }
}
// Initialization routine.
void setup(void)
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 0.0);

    // Read the external OBJ file into the internal vertex and face vectors.
    loadOBJ("gourd.obj");

    // Size the vertex array and copy into it x, y, z values from the vertex vector.
    vertices = new float[verticesVector.size()];
    for (int i = 0; i < verticesVector.size(); i++) vertices[i] = verticesVector[i];

    // Size the faces array and copy into it face index values from the face vector.
    faces = new int[facesVector.size()];
    for (int i = 0; i < facesVector.size(); i++) faces[i] = facesVector[i];
    numIndices = facesVector.size();
    getalpha();

}

// Drawing routine.
void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0.0, 0.0, 0.0);

    // Rotate scene.
    glRotatef(Zangle, 0.0, 0.0, 1.0);
    glRotatef(Yangle, 0.0, 1.0, 0.0);
    glRotatef(Xangle, 1.0, 0.0, 0.0);

    // Draw the object mesh.
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, faces);
    double m[3] = { 2.0f,2.0f,0.0f };//颜色数组
    for (int i = 0; i < facesVector.size(); i = i + 3)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_TRIANGLES);//画三角形
        double k = alpha[i / 3];
        glColor3f(m[0] * k, m[1] * k, m[2] * k);//上色
        glVertex3f(vertices[(faces[i]) * 3], vertices[(faces[i]) * 3 + 1], vertices[(faces[i]) * 3 + 2]);
        glVertex3f(vertices[(faces[i + 1]) * 3], vertices[(faces[i + 1]) * 3 + 1], vertices[(faces[i + 1]) * 3 + 2]);
        glVertex3f(vertices[(faces[i + 2]) * 3], vertices[(faces[i + 2]) * 3 + 1], vertices[(faces[i + 2]) * 3 + 2]);
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
    gluPerspective(60.0, (float)w / (float)h, 1.0, 50.0);
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
    std::cout << "Press x, X, y, Y, z, Z to turn the object." << std::endl;
}

// Main routine.
int main(int argc, char** argv)
{
    printInteraction();
    glutInit(&argc, argv);

    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OBJmodelViewer.cpp");
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);

    glewExperimental = GL_TRUE;
    glewInit();

    setup();

    glutMainLoop();
}
