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
#include "FrameTimer.h"

#define cylAngle atan(radius2/height);

int obj;

unsigned int texture[3];
unsigned int sky[6];
unsigned int testSky;

#define NUM_TURBINES 9
#define MAX_TURBINES 9
int wind = 0;

int thetaWind = 0;
double speedWind = 0;
int windChange = 0;

int th=0;         //  Azimuth of view angle
int ph=15;
double zh = 0;
int axes = 0;
int moveLight = 1;
int texmode = 0;
int mode = 1;
int fov = 40;
int box = 1;
double asp = 1;
double dim = 5;    //Size of world
double zoom = .8;
double rot[10];
int fanDir = 0;

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
float ylight = 7;     //Elevation of light

int one  = 1;         //Unit value
int distance = 10;     //Light Distance
int inc = 10;         //Ball Increment
int smooth = 1;       //Smooth/Flat Shading
int local   = 0;      //Local viewer model

double spin = 0.0;
double speed = 0.0;

#define Dfloor 7.5
#define Yfloor 0.0001
float N[] = {0, -1, 0};
float E[] = {0, Yfloor, 0};

//---------------------- Fog Attributes ------------------------------

/// the colour of the fog and the clear colour.
float  fog_colour[] = {0.6f,0.58f,0.79f,0.0f};

/// the density of the fog
float  fog_density  = 0.02f;

/// The fog mode, either GL_LINEAR, GL_EXP or GL_EXP2
GLenum fog_mode     = GL_EXP2;

/// the far distance of the fog & camera
float far_distance  = 10.0f;

/// the near distance of the fog & camera
float near_distance = 0.05f;

//----------------------Camera Attrs----------------------------------
float zoom2 = 15.0f;
float rotx = 15;
float roty = 0.0;
float tx = 0;
float ty = 0;
int lastx = 0;
int lasty = 0;
unsigned char Buttons[3] = {0};
//double energyOutput;
//--------------------------------------------------------------------

struct turbines {
   //Placement in xyz space
   double x;
   double y;
   double z;

   double size;

   double fanSpeed;
   double thetaBlades;
   int yRot;
   int shutdown;
   // 1 value = brake on fan

   double energyOutput;
};

struct turbines turbine[MAX_TURBINES];

/// a structure to hold a particle
struct Particle {

   /// the current particle position
   float position[3];

   /// the direction of the particle
   float direction[3];

   float color[3];

   /// the lifespan
   float life;

   /// pointer to the next particle
   struct Particle* next;
};

typedef struct Particle Object;

Object* Particle_new();

Object* Particle_new() {
   Object* p = malloc(sizeof(Object));
   p->position[0] = (float) (rand() % 16);
   p->position[2] = 0;
   p->position[1] = 3.0f;

   p->direction[0] = (10000 - rand()%20000)/10000.0f;
   p->direction[1] = (10000 - rand()%20000)/10000.0f;
   p->direction[2] = (10000 - rand()%20000)/10000.0f; 

   p->color[0] = 0.8f;
   p->color[1] = rand() % 15000/20000.0f;
   p->color[2] = 0.1f;

   p->life         = rand()%10000/1000.0f;   
   return p;
}

void ShadowProjection(float L[4], float E[4], float N[4])
{
   float mat[16];
   float e = E[0]*N[0] + E[1]*N[1] + E[2]*N[2];
   float l = L[0]*N[0] + L[1]*N[1] + L[2]*N[2];
   float c = e - l;
   //  Create the matrix.
   mat[0] = N[0]*L[0]+c; mat[4] = N[1]*L[0];   mat[8]  = N[2]*L[0];   mat[12] = -e*L[0];
   mat[1] = N[0]*L[1];   mat[5] = N[1]*L[1]+c; mat[9]  = N[2]*L[1];   mat[13] = -e*L[1];
   mat[2] = N[0]*L[2];   mat[6] = N[1]*L[2];   mat[10] = N[2]*L[2]+c; mat[14] = -e*L[2];
   mat[3] = N[0];        mat[7] = N[1];        mat[11] = N[2];        mat[15] = -l;
   //  Multiply modelview matrix
   glMultMatrixf(mat);
}

/// the first particle in the linked list
struct Particle* pList;

void NewParticle() {

   // create new particle and add as first in list
   Object* p = Particle_new();

   p->next = pList;
   pList = p;
}

void UpdateParticles(float dt) {

   // traverse all particles and update
   Object* p = pList;
   while(p) {
      // decrease lifespan
      p->life -= dt;

      // apply gravity
      p->direction[1] -= 9.81f*dt;

      // modify position
      p->position[0] += dt * p->direction[0];
      p->position[1] += 0.5 * dt * p->direction[1];
      p->position[2] += 0.5 * dt * p->direction[2];

      // goto next particle
      p=p->next;
   }
}

void RemoveDeadParticles() {

   // iterate over particles
   Object* curr = pList;
   Object* prev = 0;

   while (curr) {

      // if dead
      if (curr->life<0) {

         // update the previous pointer to skip over the curr 
         // particle, or just remove the particle if it's the 
         // first in the list that we need to remove.
         //
         if (prev) {
            prev->next = curr->next;
         }
         else {
            pList = curr->next;
         }

         // take temporary reference
         Object* temp = curr;

         // skip over particle in list
         curr = curr->next;

         // delete particle
         free(temp);
      }
      else {
         // move to next if not removing
         prev = curr;
         curr = curr->next;
      }
   }
}

void DrawParticles() {
   glPushMatrix();
   // iterate over all particles and draw a point
   Object* curr = pList;
   glTranslatef(-8, 0, 18);
   glRotatef(90, 1, 0, 0);
   glPointSize(2);
   glBegin(GL_POINTS);
   //glColor3f(1, 0, 0);
   while (curr) {
      glColor3fv(curr->color);
      glVertex3fv(curr->position);
      curr = curr->next;
   }
   glEnd();
   glPopMatrix();
}




/// the rotation of the teapot
double g_Rotation=0;

void OnExit() {
}

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
/*
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
*/
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
   const int d = 5;
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
      for(th = 0; th <= 360; th+=d) {
         glNormal3f(Cos(th), yNormal, Sin(th));
         glTexCoord2f(1 - th /360, 0); glVertex3f(radius2 * Cos(th), 0, radius2 * Sin(th));
         glTexCoord2f(th/360, 1); glVertex3f((radius2 - dRad) * Cos(th), height, (radius2 - dRad) * Sin(th));
      }
   glEnd();

   glDisable(GL_TEXTURE_2D);

   glPopMatrix();
}


// Used in angled cylinders

/*
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
*/

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

   ball(0,4.1,0, 2.5);
 
   glPopMatrix();
}      
/*
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
*/
// Draws triangles that make up the blades of the wind turbine
static void blade(double dx, double dy, double dz, double r, double base, double depth, double angle) {
   glPushMatrix();
   //glTranslated(dx, dy, dz);
   glRotatef(angle, 0, 0, 1);
   //glScaled(base, r, depth);
   glScaled(1.2, 1.2, 1.2);
   //glColor3f(0.12, 0.63, 0.9);
   glColor3f(1,1,1);
   glPushMatrix();
   glRotatef(-90, 0, 1, 0);
   glColor3f(1,1,1);

   glCallList(obj);
   glPopMatrix();
   glPopMatrix();  
}

// Invokes draw blade function and handles rotation of the blades
static void drawBlades(double dx, double dy, double dz, double r, double size, double speed) {
   glPushMatrix();

   glColor3f(1,1,1);
   glTranslated(dx,dy, dz);
   //Blade rotation
   glRotatef(spin * speed, 0, 0, 1);
   glScaled(size, size, size);

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
/*
static bladeModel(double x, double y, double z, double scale, double speed)
{
   glPushMatrix();
   glTranslated(x, y, z);
   //glRotatef(90, 0, 1, 0);
   glPushMatrix();
   glRotatef(speed * spin, 1, 0, 0);
   glScaled(scale, scale, scale);
   glCallList(obj);
   glPopMatrix();
   glPopMatrix();
}
*/
//Handles drawing of entire wind turbine system, from mount to fan
static void drawFan(struct turbines turbine)
{
   glPushMatrix();
   glTranslated(turbine.x, turbine.y, turbine.z);
   glScaled(0.016, 0.016, 0.016);
   glRotatef(turbine.yRot, 0, 1, 0);

   drawCylinderFinal(3, 7, 100, 0, 0, 0);
   cube(0, 100, -2, 4, 3.75, 10, 0, -1);
   drawRing(0,100,8,3,3,5, 0,0,0);
   glColor3f(1,1,1);
   drawBlades(0, 100, 10, 50, turbine.size, 1 / (turbine.size));
   //glRotatef(90, 0, 1, 0);
   glPopMatrix();
}

static void Sky(double D)
{
   glPushMatrix();
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);
   //glTranslatef(0, 0.5*D, 0);
   //  Sides
   glBindTexture(GL_TEXTURE_2D,testSky);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0.34); glVertex3f(-D,-D,-D);
   glTexCoord2f(0.25,0.34); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,0.66); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.0,0.66); glVertex3f(-D,+D,-D);
   glEnd();

   //glBindTexture(GL_TEXTURE_2D,sky[3]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.25,0.34); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.5,0.34); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.5,0.66); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,0.66); glVertex3f(+D,+D,-D);
   glEnd();

   //glBindTexture(GL_TEXTURE_2D,sky[2]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.5,0.34); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.75,0.34); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.75,0.66); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.5,0.66); glVertex3f(+D,+D,+D);
   glEnd();

   //glBindTexture(GL_TEXTURE_2D,sky[4]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.75,0.34); glVertex3f(-D,-D,+D);
   glTexCoord2f(1   ,0.34); glVertex3f(-D,-D,-D);
   glTexCoord2f(1   ,0.66); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,0.66); glVertex3f(-D,+D,+D);
   glEnd();

   //  Top and bottom
   //glBindTexture(GL_TEXTURE_2D,sky[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.25,0.67); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.50,0.67); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(-D,+D,-D);
   glEnd();
   glPopMatrix();

//   glBindTexture(GL_TEXTURE_2D,sky[5]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.25,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.5,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.5,0.34); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,0.34); glVertex3f(-D,-D,-D);
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
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

   // Set background color
   glClearColor(fog_colour[0], fog_colour[1], fog_colour[2], fog_colour[3]);

   // Set the fog attribs
   glFogf(GL_FOG_START, near_distance);
   glFogf(GL_FOG_END, far_distance);
   glFogfv(GL_FOG_COLOR, fog_colour);
   glFogi(GL_FOG_MODE, fog_mode);
   glFogf(GL_FOG_DENSITY, fog_density);
   glEnable(GL_FOG);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();

   if (mode)
   {
      //double Ex = -2*dim*Sin(th)*Cos(ph);
      //double Ey = +2*dim *Sin(ph);
      //double Ez = +2*dim*Cos(th)*Cos(ph);
      //gluLookAt(Ex,Ey,Ez, 0,0,0, 0,Cos(ph),0);
      double Ex = -2*dim*Sin(roty)*Cos(rotx);
      double Ey = +2*dim *Sin(rotx);
      double Ez = +2*dim*Cos(roty)*Cos(rotx);
      gluLookAt(Ex,Ey,Ez, 0,0,0, 0,Cos(rotx),0);      
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
//   if(light) {
      float Ambient[]   = {0.01 * ambient, 0.01 * ambient, 0.01 * ambient, 1.0};
      float Diffuse[]   = {0.01 * diffuse, 0.01 * diffuse, 0.01 * diffuse, 1.0};
      float Specular[]  = {0.01* specular, 0.01* specular, 0.01* specular, 1.0};

      //Light Position
      float Position[]  = {distance * Cos(30), ylight, distance * Sin(30), 1.0};

      glColor3f(1,1,1);
      ball(Position[0], Position[1] + 3, Position[2], 0.1);

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
 //  }
 //  else
 //     glDisable(GL_LIGHTING);

 //  glBegin(GL_QUADS);
//   glColor3f(0,0,0);
 //  glVertex

   drawGround(7.5, 0.1, 7.5, 0);
   int i;

   //Draw several instances of wind turbines
   for (i = 0; i < NUM_TURBINES; i++) {
      drawFan(turbine[i]);
   }   

   int temp = 0;
   if(wind) {
      for (i = 0; i < NUM_TURBINES; i++)
      {
         if (thetaWind > turbine[i].yRot)
            turbine[i].yRot += 1;
         else if (thetaWind < turbine[i].yRot)
            turbine[i].yRot -= 1;

         turbine[i].yRot = turbine[i].yRot % 360;

      }
   }
/*
   if(speedWind > 56) {
      for(i = 0; i < NUM_TURBINES; i++)
      {
         if(turbine[i].thetaFeather < 90)
      }
   }
*/   
   if(wind) {
      glPushMatrix();
      glRotatef(thetaWind, 0, 1, 0);
      DrawParticles();
      glPopMatrix();
   }

   if(windChange) {
      for (i = 0; i < NUM_TURBINES; i++) {
         if (turbine[i].yRot == thetaWind)
            temp += 1;
      }

      if (temp == 0)
         windChange = 0;


   }

   if(wind) {
      for(i = 0; i < NUM_TURBINES; i++) {
         if (speedWind > 9 && speedWind < 56)
         {
            double wSpeed;
            if (speedWind < 34)
               wSpeed = speedWind * 0.44704;
            else
               wSpeed = 34 * 0.44704;

            double radius = 44 * turbine[i].size;
            double powerCoeff = 0.3;
            double area = 3.14159 * (radius * radius);
            double airDensity = 1.0;
            //Covert to m/s
            turbine[i].energyOutput = 0.5 * airDensity * area * (wSpeed * wSpeed * wSpeed) * powerCoeff;
            turbine[i].energyOutput /= 1000000;
         }
         else
            turbine[i].energyOutput = 0;
      }
   }

   glPushAttrib(GL_ENABLE_BIT);

   glDisable(GL_LIGHTING);

   glEnable(GL_STENCIL_TEST);

   glStencilFunc(GL_ALWAYS,1,0xFFFFFFFF);

   glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);

   glDepthMask(0);
   glColorMask(0,0,0,0);

   glPushMatrix();
   ShadowProjection(Position, E, N);
   for (i = 0; i < NUM_TURBINES; i++) {
      drawFan(turbine[i]);
   }   

   glPopMatrix();

   glDepthMask(1);
   glColorMask(1,1,1,1);

   glStencilFunc(GL_LESS,0,0xFFFFFFFF);

   glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glColor4f(0,0,0,0.5);

   glBegin(GL_QUADS);
      glVertex3f(-Dfloor,Yfloor,-Dfloor);
      glVertex3f(+Dfloor,Yfloor,-Dfloor);
      glVertex3f(+Dfloor,Yfloor,+Dfloor);
      glVertex3f(-Dfloor,Yfloor,+Dfloor);
   glEnd();

   glPopAttrib();

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

   glWindowPos2i(5,5);
   Print("Angle=%0.f,%.0f  Dim=%.1f FOV=%d Projection=%s Light=%s",
     rotx,roty,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off");

   if (light)
   {
      glWindowPos2i(5,45);
      //Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      int temp2 = thetaWind % 360;
      glColor3f(1,1,1);
      Print("Wind Angle =%d Wind Speed = %0.f Spin: %0.f", thetaWind, speedWind, spin);
      glWindowPos2i(5,25);
      Print("Energy Turbine: T0=%.1fMW\tT1=%.1fMW\t T2=%.1fMW\tT3=%.1fMW\tT4=%.1fMW\t T5=%.1fMW\tT6=%.1fMW\tT7=%.1fMW\tT8=%.1fMW", 
                                                      turbine[0].energyOutput, turbine[1].energyOutput,
                                                      turbine[2].energyOutput,turbine[3].energyOutput,
                                                      turbine[4].energyOutput,turbine[5].energyOutput,
                                                      turbine[6].energyOutput,turbine[7].energyOutput,
                                                      turbine[8].energyOutput);
      //Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shinyvec[0]);
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
   double t = glutGet(GLUT_ELAPSED_TIME)/5000.0;
   zh = fmod(180*t,360);

  // if (moveLight)
   //   zht = fmod(90*t,360);

   int i;
/*
   //if (wind > 55)
      // Feather == true

   if (speed > 360)
      speed = 360;
   if (speed < 0)
      speed = 0;

   if(speed >= 400 && speedWind == 0)
      spin -= 1;
   else if(speed >= 400)
      spin = spin + 4;
   else
      spin = spin + speed / 100;
*/
      if (speedWind >= 8 && speedWind < 56)
         speed += 0.9;

      if(speedWind >= 56 && speedWind < 65)
         speed -= 1.5;

      if(speedWind >=65)
         speed -= 2.5;

      if(speedWind < 8 && speed > 0)
         speed -= 0.5;

  // if(! wind && speed > 0)
   //   speed -= 0.5;

      if( speed < 0)
         speed = 0;

      if(speed >= 400) {
         speed = 400;
         spin = spin + 4;
      }
      else 
         spin = spin + speed / 100;   
 
   // update the frame time
   SortFrameTimer();
   int val = 10;
   // create a new particle every frame
   if(speedWind > 0)
      val = rand()%(5 * (int) speedWind);

   for(i = 0; i < val; i++)
      NewParticle();

   // update the particle simulation
   UpdateParticles(FrameTime());

   // remove any dead particles
   RemoveDeadParticles();   

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
   else if (key == GLUT_KEY_DOWN && ph > 10)
      ph -= 5;

   else if (key == GLUT_KEY_PAGE_UP) {
      thetaWind += 5;
      thetaWind = thetaWind % 360;
      if(wind)
         windChange = 1;
   }
   else if (key == GLUT_KEY_PAGE_DOWN) {
      thetaWind -= 5;
      thetaWind = thetaWind % 360;
      if(wind)
         windChange = 1;      
   }
   else if (key == GLUT_KEY_HOME && speedWind < 80) {
      speedWind += 2;
      fanDir = 1;
      wind = 1;
      windChange = 1;
      fog_density += 0.001;
   }
   else if (key == GLUT_KEY_END && speedWind > 0) {
      speedWind -= 2;
      fanDir = -1;
      windChange = 1;
      if(speedWind == 0)
         wind = 0;

      fog_density -= 0.001;

   }
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //thetaWind %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   Project(mode?fov:0,asp,dim);
   glutPostRedisplay();
}

void key(unsigned char ch, int x, int y)
{
   if (ch == 27)
      exit(0);
   //else if (ch == 'w' || ch == 'W')
    //  wind = 1 - wind;
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

void Motion(int x,int y)
{
   int diffx=x-lastx;
   int diffy=y-lasty;
   lastx=x;
   lasty=y;

   if( Buttons[0] && Buttons[1] )
   {
      zoom2 -= (float) 0.05f * diffx;
   }
   else
      if( Buttons[0])
      {

         if (rotx > 5 && rotx < 175)
            rotx += (float) 0.5f * diffy;
         else if ((rotx <= 5 && diffy > 0) || (rotx >= 175 && diffy < 0))
            rotx += (float) 0.5f * diffy;

         roty += (float) 0.5f * diffx;    
      }
      else
         if( Buttons[1] )
         {
            tx += (float) 0.05f * diffx;
            ty -= (float) 0.05f * diffy;
         }
         glutPostRedisplay();
}

void Mouse(int b, int s, int x, int y)
{
   lastx = x;
   lasty = y;

   switch(b)
   {
      case GLUT_LEFT_BUTTON:
         Buttons[0] = ((GLUT_DOWN==s)?1:0);
         break;
      case GLUT_RIGHT_BUTTON:
         Buttons[1] = ((GLUT_DOWN==s)?1:0);
      default:
         break;
   }
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

   turbine[0].x = -4; turbine[0].y = 0; turbine[0].z = -4;
   turbine[0].size = 1.0;
   turbine[0].yRot = 0.0;
   turbine[1].x = -3; turbine[1].y = 0; turbine[1].z = -2;
   turbine[1].size = .9;
   turbine[1].yRot = 30.0;
   turbine[2].x = -2; turbine[2].y = 0; turbine[2].z = 0;
   turbine[2].size = .8;
   turbine[2].yRot = 60.0;
   turbine[3].x = -1; turbine[3].y = 0; turbine[3].z = 2;
   turbine[3].size = .7;
   turbine[3].yRot = 90.0;
   turbine[4].x = 0; turbine[4].y = 0; turbine[4].z = 5;
   turbine[4].size = .6;
   turbine[4].yRot = 120;
   turbine[5].x = 1; turbine[5].y = 0; turbine[5].z = 2;
   turbine[5].size = .7;
   turbine[5].yRot = 150.0;
   turbine[6].x = 2; turbine[6].y = 0; turbine[6].z = 0;
   turbine[6].size = .8;
   turbine[6].yRot = 180.0;
   turbine[7].x = 3; turbine[7].y = 0; turbine[7].z = -2;
   turbine[7].size = .9;
   turbine[7].yRot = 210;
   turbine[8].x = 4; turbine[8].y = 0; turbine[8].z = -4;
   turbine[8].size = 1.0;
   turbine[8].yRot = 240;


   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(1024,600);
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
   glutMouseFunc(Mouse);
   glutMotionFunc(Motion);
   glutKeyboardFunc(key);
   //  Tell GLUT to call "key" when a key is pressed
   //  Pass control to GLUT so it can interact with the user

   texture[0] = LoadTexBMP("Textures/grass.bmp");
   texture[1] = LoadTexBMP("Textures/sphere.bmp");
   texture[2] = LoadTexBMP("Textures/metal.bmp");
   testSky     = LoadTexBMP("Textures/skybox_texture.bmp");


   obj        = LoadOBJ("wing.obj");
   glutMainLoop();
   return 0;
}
