////////////////////////////////////////////////////////////////////////////////////        
// canvas.cpp
//
// This program allows the user to draw simple shapes on a canvas.
//
// Interaction:
// Left click on a box on the left to select a primitive.
// Then left click on the drawing area: once for point, twice for line or rectangle.
// Right click for menu options.
//
//  Sumanta Guha.
//////////////////////////////////////////////////////////////////////////////////// 

#include <cstdlib>
#include <vector>
#include <map>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h> 

#define INACTIVE 0
#define POINT 1
#define LINE 2
#define RECTANGLE 3
#define CIRCLE 4
#define HEXAGON 5 
#define BROKENLINE 6
#define NUMBERPRIMITIVES 6

// Globals.
static int width, height; // OpenGL window size.
static float pointSize = 3.0; // Size of point
static int primitive = INACTIVE; // Current drawing primitive.
static int pointCount = 0; // Number of  specified points.
static int tempX, tempY; // Co-ordinates of clicked point.
static int isGrid = 1; // Is there grid?
static int isFilled = 1;
float square_color[3] = { 0.25, 0.5, 0.75 }; // Color of the square.

// Point class.
class Point
{
public:
	Point(float xVal, float yVal, float sizeVal, float colorVal[3])
	{
		x = xVal; y = yVal; size = sizeVal;
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
	}
	Point() {};
	void drawPoint(void); // Function to draw a point.
	void getColor();  //get color
private:
	float x, y; // x and y co-ordinates of point.
	float size; // Size of point.
	float color[3];
};

//float Point::size = pointSize; // Set point size.

void Point::getColor()
{
	glColor3fv(color);
}

// Function to draw a point.
void Point::drawPoint()
{
	glPointSize(size);
	glBegin(GL_POINTS);
	glVertex3f(x, y, 0.0);
	glEnd();
}

// Vector of points.
std::vector<Point> points;

// Iterator to traverse a Point array.
std::vector<Point>::iterator pointsIterator;

// Function to draw all points in the points array.
void drawPoints(void)
{
	// Loop through the points array drawing each point.
	for (auto point : points) {
		point.getColor();
		point.drawPoint();
	}
}

// Line class.
class Line
{
public:
	Line(float x1Val, float y1Val, float x2Val, float y2Val, float colorVal[3])
	{
		x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
	}
	Line() {};
	void drawLine();
	void getColor();
private:
	float x1, y1, x2, y2; // x and y co-ordinates of endpoints.
	float color[3];
};

void Line::getColor()
{
	glColor3fv(color);
}

// Function to draw a line.
void Line::drawLine()
{
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0.0);
	glVertex3f(x2, y2, 0.0);
	glEnd();
}

// Vector of lines.
std::vector<Line> lines;

// Function to draw all lines in the lines array.
void drawLines(void)
{
	// Loop through the lines array drawing each line.
	for (auto line : lines) {
		line.getColor();
		line.drawLine();
	}
}

// Rectangle class.
class Rect
{
public:
	Rect(float x1Val, float y1Val, float x2Val, float y2Val, float colorVal[3], bool fillVal)
	{
		x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
		fill = fillVal;
	}
	Rect() {};
	void drawRectangle();
	void getColor();
private:
	float x1, y1, x2, y2; // x and y co-ordinates of diagonally opposite vertices.
	float color[3];
	bool fill;
};

void Rect::getColor()
{
	glColor3fv(color);
}

// Function to draw a rectangle.
void Rect::drawRectangle()
{
	if (fill == true) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glRectf(x1, y1, x2, y2);
		glColor3fv(square_color);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glRectf(x1, y1, x2, y2);
	}
}

// Vector of rectangles.
std::vector<Rect> rectangles;

// Function to draw all rectangles in the rectangles array.
void drawRectangles(void)
{
	// Loop through the rectangles array drawing each rectangle.
	for (auto rectangle : rectangles) {
		rectangle.getColor();
		rectangle.drawRectangle();
	}
}

// Function to draw a circle.
void glCircf(float cx, float cy, float r) {

	int num_segments = 1e6;
	float theta = 2.0f * 3.1415926f / float(num_segments);
	float c = cosf(theta); // 用于计算顶点位置
	float s = sinf(theta);

	float x = r; // 第一个顶点的位置
	float y = 0;

	glBegin(GL_LINE_LOOP);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for (int i = 0; i < num_segments; i++) {
		glVertex2f(x + cx, y + cy); // 绘制当前顶点
		// 计算下一个顶点位置
		float t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
	// 绘制最后一个顶点，保证圆形闭合
	glVertex2f(x + cx, y + cy);
	glEnd();
}

// Circle class.
class Circ
{
public:
	Circ(float x1Val, float y1Val, float x2Val, float y2Val, float colorVal[3], bool fillVal)
	{
		x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
		r = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
		fill = fillVal;
	}
	Circ() {};
	void drawCircle();
	void getColor();
private:
	float x1, y1, x2, y2, r; // x and y co-ordinates of diagonally opposite vertices.
	float color[3];
	bool fill;
};

void Circ::getColor()
{
	glColor3fv(color);
}

// Function to draw a Circle.
void Circ::drawCircle()
{
	if (fill == true)
	{
		const int n = 10000;
		const GLfloat pi = 3.1415926f;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_POLYGON);
		for (int i = 0; i < n; i++)//循环打点
		{
			glVertex2f(x1 + r * cos(2 * pi * i / n), y1 + r * sin(2 * pi * i / n));//x1 y1是圆心  用cos sin表示圆心到边的距离 从而进行打点
		}
		glEnd();
		glColor3fv(square_color);
		glutPostRedisplay();
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCircf(x1, y1, r);
	}
}

// Vector of circles.
std::vector<Circ> circles;

// Function to draw all circles in the circles array.
void drawCircles(void)
{
	// Loop through the rectangles array drawing each rectangle.
	for (auto circle : circles) {
		circle.getColor();
		circle.drawCircle();
	}
}

// Function to draw a hexagon.
void glHexaf(float centerX, float centerY, float radius)
{
	glBegin(GL_POLYGON);
	for (int i = 0; i <= 5; i++) {
		float M_PI = 3.1415926f;
		float angle1 = i * 2.0f * M_PI / 6.0f;
		float angle2 = (i + 1) * 2.0f * M_PI / 6.0f;
		glVertex2f(centerX + radius * cos(angle1), centerY + radius * sin(angle1));
		glVertex2f(centerX + radius * cos(angle2), centerY + radius * sin(angle2));
	}
	glEnd();
}


// Hexagon class.
class Hexa
{
public:
	Hexa(float x1Val, float y1Val, float x2Val, float y2Val, float colorVal[3], bool fillVal)
	{
		x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
		r = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
		fill = fillVal;
	}
	Hexa() {};
	void drawHexagon();
	void getColor();
private:
	float x1, y1, x2, y2, r; // x and y co-ordinates of diagonally opposite vertices.
	float color[3];
	bool fill;
};

void Hexa::getColor()
{
	glColor3fv(color);
}

// Function to draw a hexagon.
void Hexa::drawHexagon()
{
	if (fill == true)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glHexaf(x1, y1, r);
		glColor3fv(square_color);
		glutPostRedisplay();
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glHexaf(x1, y1, r);
		glutPostRedisplay();
	}
}

// Vector of hexagons.
std::vector<Hexa> hexagons;

// Function to draw all hexagons in the hexagons array.
void drawHexagons(void)
{
	// Loop through the BROKENLINE array drawing each Brokenline.
	for (auto hexagon : hexagons) {
		hexagon.getColor();
		hexagon.drawHexagon();
	}
}


// Brokenline class.
class brokenline
{
public:
	brokenline(float x1Val, float y1Val, float x2Val, float y2Val, float colorVal[3])// Two Points 
	{
		x1 = x1Val; y1 = y1Val; x2 = x2Val; y2 = y2Val;
		color[0] = colorVal[0];
		color[1] = colorVal[1];
		color[2] = colorVal[2];
	}
	brokenline() {};
	void drawBrokenline();
	void getColor();
private:
	float x1, x2, y1, y2;
	float color[3];
};

void brokenline::getColor()
{
	glColor3fv(color);
}


void brokenline::drawBrokenline()
{
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

std::vector<brokenline>brokenlines;

void drawBrokenline(void)
{
	for (auto brokenline : brokenlines) {
		brokenline.getColor();
		brokenline.drawBrokenline();
	}
}

// Function to draw point selection box in left selection area.
void drawPointSelectionBox(void)
{
	if (primitive == POINT) glColor3f(1.0, 1.0, 1.0); // Highlight.
	else glColor3f(0.8, 0.8, 0.8); // No highlight.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.9 * height, 0.1 * width, height);

	// Draw black boundary.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.9 * height, 0.1 * width, height);

	// Draw point icon.
	glPointSize(pointSize);
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	glVertex3f(0.05 * width, 0.95 * height, 0.0);
	glEnd();
}

// Function to draw line selection box in left selection area.
void drawLineSelectionBox(void)
{
	if (primitive == LINE) glColor3f(1.0, 1.0, 1.0); // Highlight.
	else glColor3f(0.8, 0.8, 0.8); // No highlight.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.8 * height, 0.1 * width, 0.9 * height);

	// Draw black boundary.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.8 * height, 0.1 * width, 0.9 * height);

	// Draw line icon.
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.025 * width, 0.825 * height, 0.0);
	glVertex3f(0.075 * width, 0.875 * height, 0.0);
	glEnd();
}

// Function to draw rectangle selection box in left selection area.
void drawRectangleSelectionBox(void)
{
	if (primitive == RECTANGLE) glColor3f(1.0, 1.0, 1.0); // Highlight.
	else glColor3f(0.8, 0.8, 0.8); // No highlight.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.7 * height, 0.1 * width, 0.8 * height);

	// Draw black boundary.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.7 * height, 0.1 * width, 0.8 * height);

	// Draw rectangle icon.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.025 * width, 0.735 * height, 0.075 * width, 0.765 * height);
	glEnd();
}

// Function to draw circle selection box in left selection area.
void drawCircleSelectionBox(void)
{
	if (primitive == CIRCLE) glColor3f(1.0, 1.0, 1.0); // Highlight.
	else glColor3f(0.8, 0.8, 0.8); // No highlight.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.6 * height, 0.1 * width, 0.7 * height);

	// Draw black boundary.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.6 * height, 0.1 * width, 0.7 * height);

	// Draw circle icon.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glCircf(0.05 * width, 0.655 * height, 0.025 * width);
	glEnd();
}

// Function to draw hexagon selection box in left selection area.
void drawHexagonSelectionBox(void)
{
	if (primitive == HEXAGON) glColor3f(1.0, 1.0, 1.0); // Highlight.
	else glColor3f(0.8, 0.8, 0.8); // No highlight.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.5 * height, 0.1 * width, 0.6 * height);

	// Draw black boundary.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.5 * height, 0.1 * width, 0.6 * height);

	// Draw circle icon.
	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glHexaf(0.05 * width, 0.555 * height, 0.025 * width);
	glEnd();
}

// Function to draw brokenline selection box in left selection area.
void drawBrokenlineSelectionBox(void)
{
	if (primitive == BROKENLINE) glColor3f(1.0, 1.0, 1.0);
	else glColor3f(0.8, 0.8, 0.8);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.4 * height, 0.1 * width, 0.5 * height);


	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.4 * height, 0.1 * width, 0.5 * height);


	glBegin(GL_LINES);
	glVertex3f(0.0475 * width, 0.435 * height, 0.0);
	glVertex3f(0.035 * width, 0.455 * height, 0.0);
	glVertex3f(0.0475 * width, 0.435 * height, 0.0);
	glVertex3f(0.075 * width, 0.475 * height, 0.0);//draw a brokenline
	glEnd();
}

// Function to draw unused part of left selection area.
void drawInactiveArea(void)
{
	glColor3f(0.6, 0.6, 0.6);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glRectf(0.0, 0.0, 0.1 * width, (1 - NUMBERPRIMITIVES * 0.1) * height);

	glColor3f(0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf(0.0, 0.0, 0.1 * width, (1 - NUMBERPRIMITIVES * 0.1) * height);
}

// Function to draw temporary point.
void drawTempPoint(void)
{
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
	glVertex3f(tempX, tempY, 0.0);
	glEnd();
}

// Function to draw a grid.
void drawGrid(void)
{
	int i;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x5555);
	glColor3f(0.75, 0.75, 0.75);

	glBegin(GL_LINES);
	for (i = 2; i <= 9; i++)
	{
		glVertex3f(i * 0.1 * width, 0.0, 0.0);
		glVertex3f(i * 0.1 * width, height, 0.0);
	}
	for (i = 1; i <= 9; i++)
	{
		glVertex3f(0.1 * width, i * 0.1 * height, 0.0);
		glVertex3f(width, i * 0.1 * height, 0.0);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);

	drawPointSelectionBox();
	drawLineSelectionBox();
	drawRectangleSelectionBox();
	drawCircleSelectionBox();
	drawHexagonSelectionBox();
	drawBrokenlineSelectionBox();
	drawInactiveArea();

	drawPoints();
	drawLines();
	drawRectangles();
	drawCircles();
	drawHexagons();
	drawBrokenline();
	if (((primitive == LINE) || (primitive == RECTANGLE) || primitive == CIRCLE || primitive == HEXAGON || primitive == BROKENLINE) &&
		(pointCount == 1)) drawTempPoint();
	if (isGrid) drawGrid();

	glutSwapBuffers();
}

// Function to pick primitive if click is in left selection area.
void pickPrimitive(int y)
{
	if (y < (1 - NUMBERPRIMITIVES * 0.1) * height) primitive = INACTIVE;
	else if (y < (1 - 5 * 0.1) * height) primitive = BROKENLINE;
	else if (y < (1 - 4 * 0.1) * height) primitive = HEXAGON;
	else if (y < (1 - 3 * 0.1) * height) primitive = CIRCLE;
	else if (y < (1 - 2 * 0.1) * height) primitive = RECTANGLE;
	else if (y < (1 - 1 * 0.1) * height) primitive = LINE;
	else primitive = POINT;
}

// The mouse callback routine.
void mouseControl(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		y = height - y; // Correct from mouse to OpenGL co-ordinates.

		if (x < 0 || x > width || y < 0 || y > height); // Click outside canvas - do nothing.

		// Click in left selection area.
		else if (x < 0.1 * width)
		{
			pickPrimitive(y);
			pointCount = 0;
		}

		// Click in canvas.
		else
		{
			if (primitive == POINT) points.push_back(Point(x, y, pointSize, square_color));
			else if (primitive == LINE)
			{
				if (pointCount == 0)
				{
					tempX = x; tempY = y;
					pointCount++;
				}
				else
				{
					lines.push_back(Line(tempX, tempY, x, y, square_color));
					pointCount = 0;
				}
			}
			else if (primitive == RECTANGLE)
			{
				if (pointCount == 0)
				{
					tempX = x; tempY = y;
					pointCount++;
				}
				else
				{
					rectangles.push_back(Rect(tempX, tempY, x, y, square_color, isFilled));
					pointCount = 0;
				}
			}
			else if (primitive == CIRCLE)
			{
				if (pointCount == 0)
				{
					tempX = x; tempY = y;
					pointCount++;
				}
				else
				{
					circles.push_back(Circ(tempX, tempY, x, y, square_color, isFilled));
					pointCount = 0;
				}
			}
			else if (primitive == HEXAGON)
			{
				if (pointCount == 0)
				{
					tempX = x; tempY = y;
					pointCount++;
				}
				else
				{
					hexagons.push_back(Hexa(tempX, tempY, x, y, square_color, isFilled));
					pointCount = 0;
				}
			}
			else if (primitive == BROKENLINE)
			{
				if (pointCount == 0)
				{
					tempX = x; tempY = y;
					pointCount++;
				}
				else
				{
					brokenlines.push_back(brokenline(tempX, tempY, x, y, square_color));
					tempX = x;
					tempY = y;   //将上一次传递的值x2y2保存给下一次的折线x1y1  达成连续折线
				}
			}
		}
	}
	glutPostRedisplay();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set viewing box dimensions equal to window dimensions.
	glOrtho(0.0, (float)w, 0.0, (float)h, -1.0, 1.0);

	// Pass the size of the OpenGL window to globals.
	width = w;
	height = h;

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
	default:
		break;
	}
}

// Clear the canvas and reset for fresh drawing.
void clearAll(void)
{
	points.clear();
	lines.clear();
	rectangles.clear();
	circles.clear();
	hexagons.clear();
	brokenlines.clear();
	primitive = INACTIVE;
	pointCount = 0;
}

// The right button menu callback function.
void rightMenu(int id)
{
	if (id == 1)
	{
		clearAll();
		glutPostRedisplay();
	}
	if (id == 2) exit(0);
}

// The sub-menu callback function.
void grid_menu(int id)
{
	if (id == 3) isGrid = 1;
	if (id == 4) isGrid = 0;
	glutPostRedisplay();
}

//
void color_menu(int id)
{
	if (id == 5)
	{
		square_color[0] = 1.0; square_color[1] = 0.0; square_color[2] = 0.0;
	}
	if (id == 6)
	{
		square_color[0] = 0.0; square_color[1] = 1.0; square_color[2] = 0.0;
	}
	if (id == 7)
	{
		square_color[0] = 0.0; square_color[1] = 0.0; square_color[2] = 1.0;
	}
	glutPostRedisplay();
}


void fill_color(int id)
{
	if (id == 8)
	{
		isFilled = 1;
	}
	if (id == 9)
	{
		isFilled = 0;
	}
	glutPostRedisplay();
}

// Function to create menu.
void makeMenu(void)
{
		int sub_menu;
		sub_menu = glutCreateMenu(grid_menu);
		glutAddMenuEntry("On", 3);
		glutAddMenuEntry("Off", 4);
		int sub_2menu;
		sub_2menu = glutCreateMenu(color_menu);
		glutAddMenuEntry("red", 5);
		glutAddMenuEntry("green", 6);
		glutAddMenuEntry("blue", 7);
		int sub_3menu;
		sub_3menu = glutCreateMenu(fill_color);
		glutAddMenuEntry("on", 8);
		glutAddMenuEntry("off", 9);
		glutCreateMenu(rightMenu);
		glutAddSubMenu("Grid", sub_menu);
		glutAddMenuEntry("Clear", 1);
		glutAddMenuEntry("Quit", 2);
		glutAddSubMenu("Color", sub_2menu);
		glutAddSubMenu("Fill", sub_3menu);
		glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);

	makeMenu(); // Create menu.
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	std::cout << "Interaction:" << std::endl;
	std::cout << "Left click on a box on the left to select a primitive." << std::endl
		<< "Then left click on the drawing area: once for point, twice for line , rectangle , circle , hexagon or brokenline." << std::endl
		<< "Right click for menu options." << std::endl;
}

// Main routine.
int main(int argc, char** argv)
{
	printInteraction();
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("canvas.cpp");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutMouseFunc(mouseControl);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}