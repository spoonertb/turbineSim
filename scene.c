//File: lighting.c - assignment3 - CSCI 4229
//Author: Thomas Spooner, Some code taken from class examples
// thsp2343 - thsp2343@colorado.edu
//  6/20/14

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "CSCIx229.h"

#define cylAngle atan(radius2/height);

unsigned int texture[3];
unsigned int sky[2];

int th=0;         //  Azimuth of view angle
int ph=15;
double zh = 0;
int axes = 0;
int moveLight = 1;
int texmode = 0;
int mode = 1;
int fov = 55;
int box = 1;
double asp = 1;
double dim = 5;    //Size of world
double zoom = 1;
double rot[10];

//Light Values
int move = 1;         //Move light
int light = 1;        //Lighting
int emission = 0;     //Emission Intensity
int ambient = 30;     //Ambient Intensity
int diffuse = 100;    //Diffuse Intensity
int specular = 0;     //Specular Intensity
int shininess = 0;    //Shininess (Power of two)
float shinyvec[1];    //Shininess (Value)
int zht   = 90;        //Light azimuth
float ylight = 2;     //Elevation of light

int one  = 1;         //Unit value
int distance = 7;     //Light Distance
int inc = 10;         //Ball Increment
int smooth = 1;       //Smooth/Flat Shading
int local   = 0;      //Local viewer model
/*
struct turbine {
   double fanSpeed = 0;
   double thetaBlades = 0;
   double yRot = 0;
};
*/
/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 *  Draw vertex in polar coordinates
 */
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);

   glNormal3d(x,y,z);
   glTexCoord2f(th/360, ph/180 +.5);
   glVertex3d(x,y,z);
}

static void Vertex2(double th,double ph, int dir)
{
   double x = Sin(th)*Cos(ph);
   double y = Sin(ph);
   double z = Cos(th)*Cos(ph);
   if(dir == 1)
      glNormal3d(x, y, z);
   else 
      glNormal3d(-x, -y, -z);

   glTexCoord2f(th/360, ph/180 +.5);
   glVertex3d(x, y, z);
}

static void ball(double x, double y, double z, double r)
{
   int th, ph;
   float yellow[] = {1.0, 1.0, 0.0, 1.0};
   float Emission[] = {0.0, 0.0, 0.01 * emission, 1.0};
   //Save trans
   glPushMatrix();

   glTranslated(x,y,z);
   glScaled(r, r, r);

   //White ball
   glColor3f(1, 1, 1);
   glMaterialfv(GL_FRONT, GL_SHININESS, shinyvec);
   glMaterialfv(GL_FRONT, GL_SPECULAR, yellow);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emission);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[1]);   
   // Bands of latitude
   for (ph= -90; ph < 90; ph +=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   glPopMatrix();
}

static void drawCylinderFinal(double radius1, double radius2, double height, double x, double y, double z)
{
   const int d = 1;
   int th;
   double dRad = radius2 - radius1;
   double length = sqrt(dRad * dRad + height * height);
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

   glPushMatrix(); 
   glTranslated(x, y, z);

   double yNormal = dRad / length;

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);

   glBegin(GL_QUAD_STRIP);
      for(th = 0; th < 360; th+=d) {
         glNormal3f(Cos(th), yNormal, Sin(th));
         glTexCoord2f(1 - th /360, 0); glVertex3f(radius2 * Cos(th), 0, radius2 * Sin(th));
         glTexCoord2f(th/360, 1); glVertex3f((radius2 - dRad) * Cos(th), height, (radius2 - dRad) * Sin(th));
      }
   glEnd();

   glDisable(GL_TEXTURE_2D);

   glPopMatrix();
}


// Used in angled cylinders


// Cylinders on the dish use this for the pitch yaw and roll settings
static void drawCylinder2(double radius1, double radius2, double height, double x, double y, double z, 
                           double pitch, double yaw, double roll)
{
   const int d = 1;
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

   glPushMatrix(); 
   int th;

   glRotatef(yaw  , 0,1,0);
   glRotatef(pitch, 1,0,0);
   glRotatef(roll , 0,0,1);
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);

   glBegin(GL_TRIANGLE_FAN);
   glVertex3f(0, height, 0); //Top of cylinder
   for(th = 0; th <=360; th += d)
   {  
      glVertex3f(radius1 * cos(th), height, radius1 * sin(th));
   }
   glEnd();

   glBegin(GL_TRIANGLE_FAN);
   glVertex3f(0,0,0);
   for(th = 360; th >= 0; th -= d)
   {
      glVertex3f(radius2 * cos(th), 0, radius2 * sin(th));
   }
   glEnd();

   glBegin(GL_QUAD_STRIP);
   for(th = 0; th < 360; th += d)
   {
      glNormal3f(cos(th), 0, sin(th));
      glTexCoord2f(1 - (th/360), 1); glVertex3f(radius2 * cos(th), 0, radius2 * sin(th));
      glTexCoord2f(th/360, 0); glVertex3f(radius1 * cos(th), height, radius1 * sin(th));
   }

   glVertex3f(radius2, 0, 0);
   glVertex3f(radius1, height, 0);
   glEnd();
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

static void drawRing(double x, double y, double z, double r1,
                     double r2,double h, double p, double r, double yaw)
{
   const int d = 1;
   int th;
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);

   glPushMatrix();
   glTranslated(x, y, z);
   glRotatef(90, 1, 0, 0);
   glColor3f(0.4, 0.63, 0.7);   

   //Outside
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=d)
   {
      glNormal3f(Cos(th), 0, Sin(th));
      glVertex3f(r1*Cos(th), 0, r1*Sin(th));
      glVertex3f((r2)*Cos(th), h, (r2)*Sin(th));
   }
   glEnd();
   //Inside
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=d)
   {
      glNormal3f(-Cos(th), 0, -Sin(th));
      glVertex3f((r1-0.5)*Cos(th), 0, (r1-0.5)*Sin(th));
      glVertex3f((r2-0.5)*Cos(th), h, (r2-0.5)*Sin(th));
   }
   glEnd();   

   //Edge
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=d)
   {
      glNormal3f(0, 1, 0);
      glVertex3f(r1*Cos(th),h,r1*Sin(th));
      glVertex3f((r2-0.5)*Cos(th), h, (r2-0.5)*Sin(th));
   }
   glEnd();   

   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=d)
   {
      glNormal3f(0, -1, 0);
      glVertex3f(r1*Cos(th),0,r1*Sin(th));
      glVertex3f((r2-0.5)*Cos(th), 0, (r2-0.5)*Sin(th));
   }
   glEnd();     

   ball(0,4.1,0, 3.1);
 
/*
   glBegin(GL_QUAD_STRIP);
   for (th=0;th<=360;th+=15)
   {
      glVertex3f(r2*Cos(th),0,r2*Sin(th));
      glVertex3f(r2*Cos(th),h,r2*Sin(th));
   }
   glEnd();
*/
   glPopMatrix();
}      

// Draw sphere taken from example code
static void sphere2(double x,double y,double z,double r, 
   double pitch, double yaw, double roll)
{
   const int d=5;
   int th,ph;

   //  Save transformation
   glPushMatrix();

   glRotatef(60.0, pitch, yaw, roll);

   //  Offset and scale
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //glEnable(GL_TEXTURE_2D);
  // glBindTexture(GL_TEXTURE_2D, texture[2]);
   glColor3f(1,1,1);
   //  Latitude bands
   for (ph=-90;ph<90;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=d)
      {
         Vertex2(th,ph, 1);
         Vertex2(th,ph+d, 1);
      }
      glEnd();
   }
  // glDisable(GL_TEXTURE_2D);
   //  Undo transformations
   glPopMatrix();
}

// Draw a portion of a sphere from above -> leaves a dish shape, orient at an angle
static void dish(double x,double y,double z,double r, 
   double pitch, double yaw, double roll, int dir)
{
   const int d=5;
   int th,ph;

   //  Save transformation
   glPushMatrix();

   glTranslated(x, y, z);
   //glTranslated(-3,12,3);
   glRotatef(60.0, pitch, yaw, roll);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   //  Offset and scale
   glScaled(r,r/1.5,r);
   //  Latitude bands
   for (ph=-90;ph<-30;ph+=d)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=d)
      {
         Vertex2(th,ph, dir);
         Vertex2(th,ph+d, dir);
      }
      glEnd();
   }
   glDisable(GL_TEXTURE_2D);
   //  Undo transformations
   glPopMatrix();
}

// Draws the cylinders that make up the receiver structure of the dish. NOT PRETTY!
static void drawReceiver(double x, double y, double z)
{
   glPushMatrix();
   glTranslated(x, y, z); 

   sphere2(0,0,0,1, 0,0,0);
   glRotatef(60, 1, 0, 1);
   glColor3f(1, 0, 0);
   drawCylinder2(.15, .15, 7, x, y, z, 130, 20, 0);
   glColor3f(0, 1, 0);
   drawCylinder2(.15, .15, 7, x, y, z, 130, 140,0);
   glColor3f(0, 0, 1);   
   drawCylinder2(.15, .15, 7, x, y, z, 130, 260,0);
   glPopMatrix();
}

//Invokes the receiver function as well as handles drawing 
// of the dish mount structure
static void drawDish(double x, double y, double z, double scale, 
                     double angle)
{  
   glPushMatrix();
   glTranslated(x, y, z);
   glScaled(scale/2,scale/2, scale/2);

   glRotatef(angle, 0, 1, 0);
   //dish(0, 0, 0, 8, 1, 0, 1);
   dish(-3.1, 12,3.1, 8, 1, 0, 1, 0);
   dish(-3,12,3, 8, 1, 0, 1, 1);
   //sphere2(-3,12,3, 8, 0,0,0);
   drawCylinderFinal(0.02, 3, 2, 0, 0, 0);
   drawCylinderFinal(0.5, 0.5, 8, 0, 0, 0); 
   drawCylinderFinal(0.1, 0.5, .6, 0, 8, 0); 
   glColor3f(1, 0, 0);
   drawReceiver(-3.5, 12.5, 3.5);

   glPopMatrix();
}

// Draws triangles that make up the blades of the wind turbine
static void blade(double dx, double dy, double dz, double r, double base, double depth, double angle) {
   glPushMatrix();
   glTranslated(dx, dy, dz);
   glRotatef(angle, 0, 0, 1);
   glScaled(base, r, depth);
   glColor3f(0.12, 0.63, 0.9);

   glBegin(GL_TRIANGLES);
      //Base
      glNormal3f(0, -1, 0);
      glVertex3f( 0.5,-0.5, 0.5);
      glVertex3f(-0.5,-0.5, 0.5);
      glVertex3f( 0.0,-0.5, -0.5);
   glEnd();
      //Front
   glBegin(GL_TRIANGLES);  
      glNormal3f(0, 1, 1); 
      glVertex3f( 0.5,-0.5, 0.5);
      glVertex3f(-0.5,-0.5, 0.5);
      glVertex3f( 0.0, 0.5, 0.0);
   glEnd();
   glBegin(GL_TRIANGLES);
      //left
      glNormal3f(-1, 1, -1);
      glVertex3f(-0.5, -0.5, 0.5);
      glVertex3f( 0.0, -0.5, -0.5);
      glVertex3f( 0.0,  0.5, 0.0);
   glEnd();
   glBegin(GL_TRIANGLES);
      //right
      glNormal3f(1, 1, -1);
      glVertex3f( 0.5, -0.5, 0.5);
      glVertex3f( 0.0, -0.5, -0.5);
      glVertex3f( 0.0,  0.5, 0.0);           
   glEnd();    

   glPopMatrix();  
}

// Invokes draw blade function and handles rotation of the blades
static void drawBlades(double dx, double dy, double dz, double r, double speed) {
   glPushMatrix();

   glTranslated(dx,dy, dz);
   //Blade rotation
   glRotatef(zh * speed, 0, 0, 1);

   //Draw blades at angles of 120 degrees
   blade(0,25,0,    r, 2, 1, 0);
   blade(-21,-12,0, r, 2, 1, 120);
   blade(21,-12,0,  r, 2, 1, -120);

   glPopMatrix();

}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimentions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th, unsigned int texnum)
{
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   //  Front
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0, 1);
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   glEnd();
   //  Back
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Top
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Bottom
   glBindTexture(GL_TEXTURE_2D,texture[2]);
   glBegin(GL_QUADS);
   glNormal3f( 0,-1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glEnd();
   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

// Draw lengthy box that will serve as ground for objects
static void drawGround(double x, double y, double z, unsigned int texnum)
{
   glPushMatrix();

   glTranslated(0, -0.1, 0);   
   glScaled(x, y, z);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texmode?GL_REPLACE:GL_MODULATE);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D, texture[texnum]);

   glBegin(GL_QUADS);
   glColor3f(.36, .36, .36);

   //Front
   glNormal3f(0,0,+1);   
   glVertex3f(-1,-1, 1);
   glVertex3f(+1,-1, 1);
   glVertex3f(+1,+1, 1);
   glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f(0,0,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,+1,-1);
   glVertex3f(+1,+1,-1);
   //  Right
   glNormal3f(+1, 0, 0);
   glVertex3f(+1,-1,+1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,+1,-1);
   glVertex3f(+1,+1,+1);
   //  Left
   glNormal3f(-1, 0, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(-1,-1,+1);
   glVertex3f(-1,+1,+1);
   glVertex3f(-1,+1,-1);
   //  Top
   glNormal3f(0,+1,0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+0,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+0,+1,0);
   glTexCoord2f(0,1); glVertex3f(-1,+1,0);

   glNormal3f(0,+1,0);
   glTexCoord2f(0,0); glVertex3f(0,+1,0);
   glTexCoord2f(1,0); glVertex3f(+0,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,0);

   glNormal3f(0,+1,0);
   glTexCoord2f(0,0); glVertex3f(0,+1,0);
   glTexCoord2f(1,0); glVertex3f(+1,+1,0);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(0,+1,-1);

   glNormal3f(0,+1,0);
   glTexCoord2f(0,0); glVertex3f(0,1,0);
   glTexCoord2f(1,0); glVertex3f(0,+1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,0);         
   //  Bottom
   glNormal3f(0, -1, 0);
   glVertex3f(-1,-1,-1);
   glVertex3f(+1,-1,-1);
   glVertex3f(+1,-1,+1);
   glVertex3f(-1,-1,+1); 
   glEnd();  

   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Handles drawing of entire wind turbine system, from mount to fan
static void drawFan(double x, double y, double z, double sizeratio,
                     double radiusrat, double speedrat, double anglerot)
{
   glPushMatrix();
   glTranslated(x, y, z);
   glScaled(sizeratio, sizeratio, sizeratio);
   glRotatef(anglerot, 0, 1, 0);

   drawCylinderFinal(3, 7, 100, 0, 0, 0);
   cube(0, 100, -2, 4, 3.75, 10, 0, -1);
   drawRing(0,100,8,3.5,3.5,5, 0,0,0);
   drawBlades(0, 100, 12, 50, speedrat);

   glPopMatrix();
}

static void Sky(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,sky[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(1.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glEnd();

   //  Top and bottom
   glBindTexture(GL_TEXTURE_2D,sky[1]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(1.0,1); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.5,1); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.5,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(1.0,0); glVertex3f(-D,-D,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=4;  //  Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();

   if (mode)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez, 0,0,0, 0,Cos(ph),0);
   }
   else 
   {
      //  Set view angle
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);

   }

   if(box)
      Sky(3*dim);

   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   // Light Switch
   if(light) {
      float Ambient[]   = {0.01 * ambient, 0.01 * ambient, 0.01 * ambient, 1.0};
      float Diffuse[]   = {0.01 * diffuse, 0.01 * diffuse, 0.01 * diffuse, 1.0};
      float Specular[]  = {0.01* specular, 0.01* specular, 0.01* specular, 1.0};

      //Light Position
      float Position[]  = {distance * Cos(zht), ylight, distance * Sin(zht), 1.0};

      glColor3f(1,1,1);
      ball(Position[0], Position[1] + 1, Position[2], 0.1);

      glEnable(GL_NORMALIZE);

      glEnable(GL_LIGHTING);

      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);

      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

      glEnable(GL_COLOR_MATERIAL);

      glEnable(GL_LIGHT0);

      glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
      glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
      glLightfv(GL_LIGHT0, GL_POSITION, Position);
   }
   else
      glDisable(GL_LIGHTING);

   //blade(0,0,0, 2,1,0.5, 0);

   //drawGround(250, 10, 250, 0);
   drawGround(5, 0.1, 5, 0);
   //cube(0,0,0, 250,10,250, 0, 0);
   //cube(0,1,0, 1,1,1, 0, 1);

   //Draw several instances of wind turbines
   drawFan( 0,   0, -2.4, 1.0*.016, 0,   1.0, 0);
   drawFan(-.16,  0,  1.6, 0.5*.016, 0,  3.0, 0);
   drawFan( 1.6, 0, -.16,  0.7*.016, 0,   1.0, 0);
   drawFan( 1.12,  0,  2.4, 1.5*.016, 0,   1.0, rot[2]);
   drawFan(-1.6, 0, -1.6, 0.6*.016, 0,   2.0, rot[3]);
   drawFan(-1.92, 0,  1.92, 1.0*.016, 0,   1.0, rot[4]);
   drawFan( 0,   0,  0,   1.0*.016, 0,   1.0, rot[5]);
   drawFan(-0.8,  0, -0.8,  1.0*.016, 0,   1.0, rot[6]);  

   //Draw 2 instances of satellite dishes
   drawDish(2.4,0,-2.4 ,10*0.016, 0);
   drawDish(-3.2, 0, -3.2, 15*0.016, 90);
   //drawDish(0,0,0, 15*.016, 0);

   //drawCylinderFinal(.5, 1, 2, 0, 0, 0, 1);
  // drawCylinder(.5, 1, 2, 0, 0, 0);
  // drawDish(0,0,0, 10*0.016, 0);
   glDisable(GL_LIGHTING);

   //  White
   glColor3f(1,1,1);
   //  Draw axes
   if (axes) {
   glBegin(GL_LINES);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(len,0.0,0.0);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(0.0,len,0.0);
   glVertex3d(0.0,0.0,0.0);
   glVertex3d(0.0,0.0,len);
   glEnd();
   //  Label axes
   glRasterPos3d(len,0.0,0.0);
   Print("X");
   glRasterPos3d(0.0,len,0.0);
   Print("Y");
   glRasterPos3d(0.0,0.0,len);
   Print("Z");
   }


   //  Five pixels from the lower left corner of the window
/*   glWindowPos2i(5,5);
   //  Print the text string
   Print("Angle=%d,%d",th,ph);

   glWindowPos2i(160, 5);
   if (mode == 1) {
      Print("Mode= Perspective");
      glWindowPos2i(350, 5);
      Print("Fov=%d", fov); }
   else if (mode == 0)
      Print("Mode= Orthogonal");
*/
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s",
     th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off");
   if (light)
   {
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shinyvec[0]);
   }
   ErrCheck("display");

   //  Render the scene
   //glPopMatrix();
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360);

   if (moveLight)
      zht = fmod(90*t,360);

   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;

   else if (key == GLUT_KEY_PAGE_UP)
      zoom += 0.1;
   else if (key == GLUT_KEY_PAGE_DOWN)
      zoom -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   Project(mode?fov:0,asp,dim);
   glutPostRedisplay();
}

void key(unsigned char ch, int x, int y)
{
   if (ch == 27)
      exit(0);
   else if (ch == '0')
      th = ph = 0;
   else if (ch == 'm' || ch == 'M')
      mode = 1 - mode;
   else if (ch == 'x' || ch == 'X')
      axes = 1 - axes;
   else if (ch == 'l' || ch == 'L')
      light = 1 - light;
   else if (ch == '-' || ch == '_')
      fov ++;
   else if (ch == '+' || ch == '=')
      fov --;
   else if (ch == '[' && ylight > 0)
      ylight -= 0.1;
   else if (ch == ']')
      ylight += 0.1;

   //Ambient Light
   else if (ch == 'a' && ambient > 0)
      ambient -= 5;
   else if (ch == 'A' && ambient < 100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
      diffuse -= 5;
   else if (ch=='D' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
      specular -= 5;
   else if (ch=='S' && specular<100)
      specular += 5;
   //  Emission level
   else if (ch=='e' && emission>0)
      emission -= 5;
   else if (ch=='E' && emission<100)
      emission += 5;
   //  Shininess level
   else if (ch=='n' && shininess>-1)
      shininess -= 1;
   else if (ch=='N' && shininess<7)
      shininess += 1;   
   else if (ch == 'q' || 'Q')
      moveLight = 1 - moveLight;
   else if (ch == 'x' || 'X')
      axes = 1 - axes;

   // Translate shinieness power to value (-1 => 0)
   shinyvec[0] = shininess<0 ? 0 : pow(2.0, shininess);
   // Reproject
   Project(mode?fov:0,asp,dim);
   //Tell GLUT it is necessary to redisplay the scene
   glutIdleFunc(move?idle:NULL);

   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   asp = (height > 0) ? (double)width/height : 1;

   glViewport(0, 0, width, height);

   Project(mode?fov:0,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   int i;

   //Give random positions for turbine orientations
   //Stored in global array
   for(i = 0; i < 10; i++)
      rot[i] = rand() % 90;

   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Assignment 3 - Thomas Spooner");
   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);

   glutKeyboardFunc(key);
   //  Tell GLUT to call "key" when a key is pressed
   //  Pass control to GLUT so it can interact with the user

   texture[0] = LoadTexBMP("Textures/grass.bmp");
   texture[1] = LoadTexBMP("Textures/sphere.bmp");
   texture[2] = LoadTexBMP("Textures/metal.bmp");
   sky[0]     = LoadTexBMP("Textures/sky0.bmp");
   sky[1]     = LoadTexBMP("Textures/sky1.bmp");
   glutMainLoop();
   return 0;
}
