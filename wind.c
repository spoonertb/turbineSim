 #include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "CSCIx229.h"

double t = 0;

void RenderSine()
{
	  double i;
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double w = glutGet(GLUT_WINDOW_WIDTH);
    double h = glutGet(GLUT_WINDOW_HEIGHT);

    double ar = w / h;

    glOrtho( -10, 10 * ar, -10, 10, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLineWidth(5);
    glColor3f(.37,.37,.37);

    glBegin(GL_LINE_STRIP);
    for( i = -10; i < 10; i += 0.01)
    {
    	float y = sin(2*i + t);
    	glVertex2f(i, y);
    }
    glEnd();

    t +=1;

    glWindowPos2i(5,25);
    Print("Time: %d",t);

    glFlush();
    glutSwapBuffers();
}
void idle() {

t+=1;
}
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Wind Test");
   glutIdleFunc(idle);

   glutDisplayFunc(RenderSine);

   glutMainLoop();
   return 0;
}