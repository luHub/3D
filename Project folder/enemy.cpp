#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "trackball.h"
#include "argumentParser.h"
#include <stdio.h>
#include <string.h>

//START READING HERE!!!


//////Predefined global variables
std::vector<float> MeshVertices;
std::vector<unsigned int> MeshTriangles;

//Declare your own global variables here:
float globalXIncVal = 0.0;
float handAngleRot = 0.0;
float armAngleRot = 0.0;

////////// Draw Functions 

//function to draw coordinate axes with a certain length (1 as a default)
void drawCoordSystem(float length = 1)
{
	//draw simply colored axes

	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	//deactivate the lighting state
	glDisable(GL_LIGHTING);
	//draw axes
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(length, 0, 0);

	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, length, 0);

	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, length);
	glEnd();

	//reset to previous state
	glPopAttrib();
}

/**
* Several drawing functions for you to work on
*/

void drawTriangle()
{
	//a simple example of a drawing function for a triangle
	//1) try changing its color to red
	//2) try changing its vertex positions
	//3) add a second triangle in blue
	//4) add a global variable (initialized at 0), which represents the 
	// x-coordinate of the first vertex of each triangle
	//5) go to the function animate and increment this variable 
	//by a small value - observe the animation.

	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glColor3f(1, 0, 0);
	glNormal3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(0 + globalXIncVal, 0, 0);
	glVertex3f(1 + globalXIncVal, 0, 0);
	glVertex3f(1 + globalXIncVal, 1, 0);
	glEnd();

	glColor3f(0, 0, 1);
	glNormal3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(1 + globalXIncVal, 0, 0);
	glVertex3f(2 + globalXIncVal, 0, 0);
	glVertex3f(2 + globalXIncVal, 1, 0);
	glEnd();

	//reset to previous state
	glPopAttrib();
}

void drawTriangleAdapted(int index1, int index2, int index3)
{
	//a simple example of a drawing function for a triangle
	//1) try changing its color to red
	//2) try changing its vertex positions
	//3) add a second triangle in blue
	//4) add a global variable (initialized at 0), which represents the 
	// x-coordinate of the first vertex of each triangle
	//5) go to the function animate and increment this variable 
	//by a small value - observe the animation.

	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glColor3f(0.5, 0.5, 0.5);
	float v1 = MeshVertices[index1];
	float v2 = MeshVertices[index2];
	float v3 = MeshVertices[index3];

	/*glColor3f(1, 0, 0);
	glNormal3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(0 + globalXIncVal, 0, 0);
	glVertex3f(1 + globalXIncVal, 0, 0);
	glVertex3f(1 + globalXIncVal, 1, 0);
	glEnd();

	glColor3f(0, 0, 1);
	glNormal3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(1 + globalXIncVal, 0, 0);
	glVertex3f(2 + globalXIncVal, 0, 0);
	glVertex3f(2 + globalXIncVal, 1, 0);
	glEnd();*/

	//reset to previous state
	glPopAttrib();
}

void drawUnitFace()
{
	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glColor3f(1, 1, 1);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	glVertex3f(1, 1, 0);
	glVertex3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glEnd();
	//1) draw a unit quad in the x,y plane oriented along the z axis
	//2) make sure the orientation of the vertices is positive (counterclock wise)
	//3) What happens if the order is inversed?
	// answer to 3: a unit quad is drawn with the filled-in side at the other side.

	//reset to previous state
	glPopAttrib();
}

void drawUnitCube()
{
	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Placing it correctly.
	glTranslatef(0, 1, 1);
	// Resetting rotations.
	glRotatef(90, 0, 1, 0);
	glRotatef(90, 1, 0, 0);
	// Instructions for drawing the cube.
	glTranslatef(0, 0, 1);
	drawUnitFace();
	glTranslatef(0, 1, -1);
	glRotatef(180, 1, 0, 0);
	drawUnitFace();
	glTranslatef(0, 0, -1);
	glRotatef(90, 1, 0, 0);
	drawUnitFace();
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 0, 1);
	drawUnitFace();
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 0, 1);
	drawUnitFace();
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 0, 1);
	drawUnitFace();
	//1) draw a cube using your function drawUnitFace
	//rely on glTranslate, glRotate, glPushMatrix, and glPopMatrix
	//the latter two influence the model matrix, as seen during the course.
	//glPushMatrix stores the current matrix and puts a copy on 
	//the top of a stack.
	//glPopMatrix pops the top matrix on the stack

	//reset to previous state
	glPopAttrib();
}

void drawArm()
{
	//remember all states of the GPU
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	drawUnitCube();
	glRotatef(armAngleRot, 1, 0, 0);
	//glTranslatef(0, 0, 1);
	drawUnitCube();
	glRotatef(handAngleRot, 1, 0, 0);
	//glTranslatef(0, 0, 1);
	drawUnitCube();
	//produce a three-unit arm (upperarm, forearm, hand) making use of your 
	//function drawUnitCube to define each of them
	//1) define 3 global variables that control the angles 
	//between the arm parts
	//and add cases to the keyboard function to control these values

	//2) use these variables to define your arm
	//use glScalef to achieve different arm length
	//use glRotate/glTranslate to correctly place the elements

	//3 optional) make an animated snake out of these boxes 
	//(an arm with 10 joints that moves using the animate function)

	//reset to previous state
	glPopAttrib();
}


void drawMesh()
{
	printf("Mesh triangle size: %d", MeshTriangles.size());
	//1) use the mesh data structure;
	//each triangle is defined with 3 consecutive indices in the MeshTriangles table
	//these indices correspond to vertices stored in the MeshVertices table.
	//provide a function that draws these triangles.

	//2) compute the normals of these triangles

	//3) try computing a normal per vertex as the average of the adjacent face normals
	// call glNormal3f with the corresponding values before each vertex
	// What do you observe with respect to the lighting?

	//4) try loading your own model (export it from Blender as a Wavefront obj) and replace the provided mesh file.

	/**
	Due to the David.obj not loading, I will provide pseudocode on how to do this:
	1. Look for the three individual vertices per triangle.
	2. Draw these vertices. This results in a triangle.
	3. Compute the normal per triangle. This can be done by:
	3a. Call the three vertices v1, v2 and v3.
	3b. Calculate distances d1 = v2 - v1, d2 = v3 - v1.
	3c. Calculate normal: N = d1 x d2 (x is for cross product).
	4. Repeat this for every triangle.
	**/

}