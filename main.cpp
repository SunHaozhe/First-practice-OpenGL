// ----------------------------------------------
// Informatique Graphique 3D & R�alit� Virtuelle.
// Travaux Pratiques
// Introduction � OpenGL
// Copyright (C) 2015 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <GLUT/glut.h>

using namespace std;

// App parameters
static const unsigned int DEFAULT_SCREENWIDTH = 1024;
static const unsigned int DEFAULT_SCREENHEIGHT = 768;
static string appTitle ("Informatique Graphique & Realite Virtuelle - Travaux Pratiques - Introduction a OpenGL");
static GLint window;
static unsigned int screenWidth;
static unsigned int screenHeight;

static int nb = 60;
static float inc1 = M_PI / nb;
static float inc2 = 2 * M_PI / nb;

static float * spherePositionArray;
static unsigned int * sphereIndexArray;
static float * sphereNormalArray;
static float currentTime;
static float acceleration = 1;
//static double mouse_init_x;
//static double mouse_init_y;
//static float angle = 0;


// Camera parameters
static float fovAngle;
static float aspectRatio;
static float nearPlane;
static float farPlane;
static float camPhi; // Expressing the camera position in polar coordinate, in the frame of the target
static float camTheta;
static float camDist2Target;
static float camTargetX;
static float camTargetY;
static float camTargetZ;

void polar2Cartesian (float phi, float theta, float r, float & x, float & y, float & z) {
    x = r * sin (theta) * cos (phi);
    y = r * sin (theta) * sin (phi);
    z = r * cos (theta);
}

void printUsage () {
    std::cerr << std::endl // send a line break to the standard error output
    << appTitle << std::endl
    << "Author : Tamy Boubekeur" << std::endl << std::endl
    << "Usage : ./main [<file.off>]" << std::endl
    << "Cammandes clavier :" << std::endl
    << "------------------" << std::endl
    << " ?: Print help" << std::endl
    << " w: Toggle wireframe mode" << std::endl
    << " <drag>+<left button>: rotate model" << std::endl
    << " <drag>+<right button>: move model" << std::endl
    << " <drag>+<middle button>: zoom" << std::endl
    << " q, <esc>: Quit" << std::endl << std::endl;
}

float * normalAt(float theta, float phi){
    float * pointNormal = new float[3];
    
    float x0 = sin (theta) * cos (phi);
    float y0 = sin (theta) * sin (phi);
    float z0 = cos (theta);
    float x1 = sin (theta) * cos (phi + inc2);
    float y1 = sin (theta) * sin (phi + inc2);
    float z1 = cos (theta);
    float x2 = sin (theta - inc1) * cos (phi);
    float y2 = sin (theta - inc1) * sin (phi);
    float z2 = cos (theta - inc1);
    float x3 = sin (theta) * cos (phi - inc2);
    float y3 = sin (theta) * sin (phi - inc2);
    float z3 = cos (theta);
    float x4 = sin (theta + inc1) * cos (phi);
    float y4 = sin (theta + inc1) * sin (phi);
    float z4 = cos (theta + inc1);
    
    //014 face normal v10 v04
    float * v10 = new float[3];
    v10[0] = x0 - x1;
    v10[1] = y0 = y1;
    v10[2] = z0 - z1;
    float * v04 = new float[3];
    v04[0] = x4 - x0;
    v04[1] = y4 = y0;
    v04[2] = z4 - z0;
    float * n014 = new float[3];
    n014[0] = v10[1] * v04[2] - v04[1] * v10[2];
    n014[1] = v10[2] * v04[0] - v04[2] * v10[0];
    n014[2] = v10[0] * v04[1] - v10[1] * v04[0];
    delete[] v10;
    
    //012 face normal V01 v02
    float * v01 = new float[3];
    v01[0] = x1 - x0;
    v01[1] = y1 = y0;
    v01[2] = z1 - z0;
    float * v02 = new float[3];
    v02[0] = x2 - x0;
    v02[1] = y2 = y0;
    v02[2] = z2 - z0;
    float * n012 = new float[3];
    n012[0] = v01[1] * v02[2] - v02[1] * v01[2];
    n012[1] = v01[2] * v02[0] - v02[2] * v01[0];
    n012[2] = v01[0] * v02[1] - v01[1] * v02[0];
    delete [] v01;
    
    //023 face normal v02 v03
    float * v03 = new float[3];
    v03[0] = x3 - x0;
    v03[1] = y3 = y0;
    v03[2] = z3 - z0;
    float * n023 = new float[3];
    n023[0] = v02[1] * v03[2] - v03[1] * v02[2];
    n023[1] = v02[2] * v03[0] - v03[2] * v02[0];
    n023[2] = v02[0] * v03[1] - v02[1] * v03[0];
    delete [] v02;
    
    //034 face normal V03 v04
    float * n034 = new float[3];
    n034[0] = v03[1] * v04[2] - v04[1] * v03[2];
    n034[1] = v03[2] * v04[0] - v04[2] * v03[0];
    n034[2] = v03[0] * v04[1] - v03[1] * v04[0];
    delete [] v03;
    delete[] v04;
    
    pointNormal[0]     = n014[0] + n012[0] + n023[0] + n034[0];
    pointNormal[1] = n014[1] + n012[1] + n023[1] + n034[1];
    pointNormal[2] = n014[2] + n012[2] + n023[2] + n034[2];
    
    delete [] n014;
    delete [] n012;
    delete [] n023;
    delete [] n034;
    
    return pointNormal;
}

float * calculateNormal(){
    sphereNormalArray = new float[18 * (nb + 1) * (nb + 1)];
    // order : 321234
    for(int i = 0; i <= nb; i++){
        for(int j = 0; j <= nb; j++){
            float theta = inc1 * (i - 1);
            float phi = inc2 * (j - 1);
            
            float *  normal1 = normalAt(theta + inc1, phi);
            sphereNormalArray[18 * ((nb + 1) * i + j)]      = normal1[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 1]  = normal1[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 2]  = normal1[2];
            delete [] normal1;
            float * normal2 = normalAt(theta, phi + inc2);
            sphereNormalArray[18 * ((nb + 1) * i + j) + 3]  = normal2[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 4]  = normal2[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 5]  = normal2[2];
            delete [] normal2;
            float * normal3 = normalAt(theta, phi);
            sphereNormalArray[18 * ((nb + 1) * i + j) + 6]  = normal3[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 7]  = normal3[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 8]  = normal3[2];
            delete [] normal3;
            float * normal4 = normalAt(theta, phi += inc2);
            sphereNormalArray[18 * ((nb + 1) * i + j) + 9]  = normal4[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 10] = normal4[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 11] = normal4[2];
            delete [] normal4;
            float * normal5 = normalAt(theta + inc1, phi);
            sphereNormalArray[18 * ((nb + 1) * i + j) + 12] = normal5[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 13] = normal5[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 14] = normal5[2];
            delete [] normal5;
            float * normal6 = normalAt(theta + inc1, phi + inc2);
            sphereNormalArray[18 * ((nb + 1) * i + j) + 15] = normal6[0];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 16] = normal6[1];
            sphereNormalArray[18 * ((nb + 1) * i + j) + 17] = normal6[2];
            delete [] normal6;
        }
    }
    return sphereNormalArray;
}

void init () {
    // OpenGL initialization
    glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
    glEnable (GL_DEPTH_TEST); // Enable the z-buffer in the rasterization
    glLineWidth (2.0); // Set the width of edges in GL_LINE polygon mode
    glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // Background color
    
    
    // Camera initialization
    fovAngle = 45.f;
    nearPlane = 0.01;
    farPlane = 10.0;
    camPhi = M_PI/2.0;
    //camTheta = M_PI/2.0;
    camTheta = 0;
    //camDist2Target = 5.0;
    camDist2Target = 10.0;
    camTargetX = 0.0;
    camTargetY = 0.0;
    camTargetZ = 0.0;
    
    spherePositionArray = new float[18 * (nb + 1) * (nb + 1)];
    sphereIndexArray = new unsigned int[18 * (nb + 1) * (nb + 1)];
    
    for(int i = 0; i < 18 * (nb + 1) * (nb + 1); i++){
        sphereIndexArray[i] = i;
    }
    // order : 321234
    for(int i = 0; i <= nb; i++){
        for(int j = 0; j <= nb; j++){
            float theta = inc1 * (i - 1);
            float phi = inc2 * (j - 1);
            
            float x1 = sin (theta) * cos (phi);
            float y1 = sin (theta) * sin (phi);
            float z1 = cos (theta);
            float x2 = sin (theta) * cos (phi + inc2);
            float y2 = sin (theta) * sin (phi + inc2);
            float z2 = cos (theta);
            float x3 = sin (theta + inc1) * cos (phi);
            float y3 = sin (theta + inc1) * sin (phi);
            float z3 = cos (theta + inc1);
            float x4 = sin (theta + inc1) * cos (phi + inc2);
            float y4 = sin (theta + inc1) * sin (phi + inc2);
            float z4 = cos (theta + inc1);
            
            
            spherePositionArray[18 * ((nb + 1) * i + j)]      = x3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 1]  = y3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 2]  = z3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 3]  = x2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 4]  = y2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 5]  = z2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 6]  = x1;
            spherePositionArray[18 * ((nb + 1) * i + j) + 7]  = y1;
            spherePositionArray[18 * ((nb + 1) * i + j) + 8]  = z1;
            spherePositionArray[18 * ((nb + 1) * i + j) + 9]  = x2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 10] = y2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 11] = z2;
            spherePositionArray[18 * ((nb + 1) * i + j) + 12] = x3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 13] = y3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 14] = z3;
            spherePositionArray[18 * ((nb + 1) * i + j) + 15] = x4;
            spherePositionArray[18 * ((nb + 1) * i + j) + 16] = y4;
            spherePositionArray[18 * ((nb + 1) * i + j) + 17] = z4;
        }
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(float), (GLvoid*)spherePositionArray);
    
    sphereNormalArray = calculateNormal();
    
    glEnableClientState (GL_NORMAL_ARRAY);
    glNormalPointer (GL_FLOAT, 3*sizeof (float), (GLvoid*)sphereNormalArray);
    glEnable (GL_NORMALIZE); // preserve les vecteurs normaux unitaires, quelle que soit la transformation courante
    
    glEnable (GL_LIGHTING);
    GLfloat light_position[4] = {10.0f, 10.0f, 10.0f, 1.0f};
    GLfloat color[4] = {1.0f, 1.0f, 0.9f, 1.0f};
    glLightfv (GL_LIGHT0, GL_POSITION, light_position); // On place la source N° 0 en (10,10,10)
    glLightfv (GL_LIGHT0, GL_DIFFUSE, color); // On lui donne légèrement orangée
    glLightfv (GL_LIGHT0, GL_SPECULAR, color); // Une hérésie, mais OpenGL est conçu comme cela
    glEnable (GL_LIGHT0); // On active la source 0
}

void setupCamera () {
    glMatrixMode (GL_PROJECTION); // Set the projection matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    gluPerspective (fovAngle, aspectRatio, nearPlane, farPlane); // Set the current projection matrix with the camera intrinsics
    glMatrixMode (GL_MODELVIEW); // Set the modelview matrix as current. All upcoming matrix manipulations will affect it.
    glLoadIdentity ();
    float camPosX, camPosY, camPosZ;
    polar2Cartesian (camPhi, camTheta, camDist2Target, camPosX, camPosY, camPosZ);
    camPosX += camTargetX;
    camPosY += camTargetY;
    camPosZ += camTargetZ;
    gluLookAt (camPosX, camPosY, camPosZ, camTargetX, camTargetY, camTargetZ, 0.0, 1.0, 0.0); // Set up the current modelview matrix with camera transform
}

void reshape (int w, int h) {
    screenWidth = w;
    screenHeight = h;
    aspectRatio = static_cast<float>(w)/static_cast<float>(h);
    glViewport (0, 0, (GLint)w, (GLint)h); // Dimension of the drawing region in the window
    setupCamera ();
}
/*
 void glSphere(float x, float y, float z, float r){
 
 int nb = 30;
 float inc1 = M_PI / nb;
 float inc2 = 2 * M_PI / nb;
 
 glMatrixMode (GL_MODELVIEW);// Indique que l’on va désormais altérer la matrice modèle-vue
 glPushMatrix (); // pousse la matrice courante sur un pile
 glTranslatef (x, y, z); // applique une translation à la matrice [...] // dessin des polygones (glVertex3f, etc), dans le repère définit par la matrice model-vue
 
 //glBegin (GL_TRIANGLES);
 for(int i = 0; i < nb; i++){
 for(int j = 0; j < nb; j++){
 float theta = inc1 * (i - 1);
 float phi = inc2 * (j - 1);
 float x1 = r * sin (theta) * cos (phi);
 float y1b = r * sin (theta) * sin (phi);
 float z1 = r * cos (theta);
 float x2 = r * sin (theta) * cos (phi + inc2);
 float y2 = r * sin (theta) * sin (phi + inc2);
 float z2 = r * cos (theta);
 float x3 = r * sin (theta + inc1) * cos (phi);
 float y3 = r * sin (theta + inc1) * sin (phi);
 float z3 = r * cos (theta + inc1);
 float x4 = r * sin (theta + inc1) * cos (phi + inc2);
 float y4 = r * sin (theta + inc1) * sin (phi + inc2);
 float z4 = r * cos (theta + inc1);
 
 glColor3f (x3, y3, z3);
 glVertex3f (x3, y3, z3);
 glColor3f (x2, y2, z2);
 glVertex3f (x2, y2, z2);
 glColor3f (x1, y1b, z1);
 glVertex3f (x1, y1b, z1);
 
 glColor3f (x2, y2, z2);
 glVertex3f (x2, y2, z2);
 glColor3f (x3, y3, z3);
 glVertex3f (x3, y3, z3);
 glColor3f (x4, y4, z4);
 glVertex3f (x4, y4, z4);
 
 }
 }
 //glEnd ();
 glDrawElements(GL_TRIANGLES, 3*sizeof(float), GL_UNSIGNED_INT, sphereIndexArray);
 
 glPopMatrix (); // replace la matrice modèle vue courante original
 }
 */

void glSphere(float x, float y, float z, float r){
    glMatrixMode (GL_MODELVIEW);// Indique que l’on va désormais altérer la matrice modèle-vue
    glPushMatrix (); // pousse la matrice courante sur un pile
    glTranslatef (x, y, z); // applique une translation à la matrice [...] // dessin des polygones (glVertex3f, etc), dans le repère définit par la matrice model-vue
    glScalef (r, r, r);
    //glRotatef(currentTime / 30, 5, 1, 1);
    glDrawElements(GL_TRIANGLES, 6 * (nb + 1) * (nb + 1), GL_UNSIGNED_INT, sphereIndexArray);
    glPopMatrix (); // replace la matrice modèle vue courante original
    
}


void display () {
    setupCamera ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
    
    glMatrixMode (GL_MODELVIEW);// Indique que l’on va désormais altérer la matrice modèle-vue
    glPushMatrix (); // pousse la matrice courante sur un pile
    
    glRotatef((currentTime / 30) * acceleration, 1, 0, 1);
    
    // Put your drawing code (glBegin, glVertex, glCallList, glDrawArray, etc) here
    glSphere(0, 0, 0, 1);
    glSphere(-2, 0, 0, 1);
    glSphere(2, 0, 0, 1);
    glSphere(-1, sqrt(3), 0, 1);
    glSphere(1, sqrt(3), 0, 1);
    glSphere(0, 2 * sqrt(3), 0, 1);
    //glSphere(-1, 2 / sqrt(3), 1 + sqrt(3), 1);
    //glSphere(1, 2 / sqrt(3), 1 + sqrt(3), 1);
    //glSphere(0, (2 / sqrt(3)) + sqrt(3), 1 + sqrt(3), 1);
    //glSphere(0, 4 / sqrt(3), 1 + 2 * sqrt(3), 1);
    
    glPopMatrix ();
    
    glFlush (); // Ensures any previous OpenGL call has been executed
    glutSwapBuffers ();  // swap the render buffer and the displayed (screen) one
}


void keyboard (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
        case '+':
            acceleration = acceleration * 1.2;
            break;
        case '-':
            acceleration = acceleration / 1.2;
            break;
        case 'w':
            GLint mode[2];
            glGetIntegerv (GL_POLYGON_MODE, mode);
            glPolygonMode (GL_FRONT_AND_BACK, mode[1] ==  GL_FILL ? GL_LINE : GL_FILL);
            break;
        case 'q':
        case 27:
            exit (0);
            break;
        default:
            printUsage ();
            break;
    }
    glutPostRedisplay ();
}


void mouse (int button, int state, int x, int y) {
    /*
     if (state == GLUT_DOWN)  {
     mouse_init_x = (double)x;
     mouse_init_y = (double)y;
     }
     */
}

void motion (int x, int y) {
    /*
     if (((double)x) != mouse_init_x) {
     angle += (abs(x - (int)mouse_init_x));
     glutPostRedisplay();
     }
     if (((double)y) != mouse_init_y) {
     angle += (abs(y - (int)mouse_init_y));
     glutPostRedisplay();
     }
     */
}

// This function is executed in an infinite loop. It updated the window title
// (frame-per-second, model size) and ask for rendering
void idle () {
    glutPostRedisplay();
    currentTime = glutGet((GLenum)GLUT_ELAPSED_TIME);
}

int main (int argc, char ** argv) {
    glutInit (&argc, argv); // Initialize a glut app
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // Setup a RGBA framebuffer to display, with a depth buffer (z-buffer), in double buffer mode (fill a buffer then update the screen)
    glutInitWindowSize (DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT); // Set the window app size on screen
    window = glutCreateWindow (appTitle.c_str ()); // create the window
    init (); // Your initialization code (OpenGL states, geometry, material, lights, etc)
    glutReshapeFunc (reshape); // Callback function executed whenever glut need to setup the projection matrix
    glutDisplayFunc (display); // Callback function executed when the window app need to be redrawn
    glutKeyboardFunc (keyboard); // Callback function executed when the keyboard is used
    glutMouseFunc (mouse); // Callback function executed when a mouse button is clicked
    glutMotionFunc (motion); // Callback function executed when the mouse move
    glutIdleFunc (idle); // Callback function executed continuously when no other event happens (good for background procesing or animation for instance).
    printUsage (); // By default, display the usage help of the program
    glutMainLoop ();
    return 0;
}
