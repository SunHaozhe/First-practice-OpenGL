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

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <GL/glut.h>
#endif
#endif



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

static unsigned int width;
static unsigned int height;
static unsigned char * image;
GLuint texture; // Identifiant opengl de la texture

static float * sphereTexcoordArray;
static float * spherePositionArray;
static unsigned int * sphereIndexArray;
static float currentTime;
static float acceleration = 1;
static double mouse_init_x;
static double mouse_init_y;


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
    << "Author : Tamy Boubekeur, Haozhe Sun" << std::endl << std::endl
    << "Usage : ./main [<file.off>]" << std::endl
    << "Cammandes clavier :" << std::endl
    << "------------------" << std::endl
    << " ?: Print help" << std::endl
    << " w: Toggle wireframe mode" << std::endl
    << " <drag>+<left button>: rotate model" << std::endl
    << " q, <esc>: Quit" << std::endl
    << " 1 or x: switch of light 1" << std::endl
    << " 2 or c: switch of light 2" << std::endl
    << " UP button: zoom in " << std::endl
    << " DOWN button: zoom out " << std::endl
    << " LEFT button: move model to left" << std::endl
    << " RIGHT button: move model to right " << std::endl
    << " + button: accelerate the speed of animation " << std::endl
    << " - button: decelerate the speed of animation " << std::endl<< std::endl;
}

void genCheckerboard (unsigned int width, unsigned int height, unsigned char * image){
    unsigned int unitw = width / 10;
    unsigned int unith = height / 10;
    for(unsigned int i = 0; i < height; i++){
        for(unsigned int j = 0; j < width; j++){
            unsigned int row = (i - i % unith) / unith;
            unsigned int column = (j - j % unitw) / unitw;
            if((row % 2) == (column % 2)){
                image[3 * (i * width + j)]     = 255;
                image[3 * (i * width + j) + 1] = 0;
                image[3 * (i * width + j) + 2] = 0;
            }else{
                image[3 * (i * width + j)]     = 0;
                image[3 * (i * width + j) + 1] = 0;
                image[3 * (i * width + j) + 2] = 255;
            }
        }
    }
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
    farPlane = 50.0;
    camPhi = M_PI/2.0;
    camTheta = M_PI/7.0;
    camDist2Target = 20.0;
    camTargetX = 0.0;
    camTargetY = 0.0;
    camTargetZ = 0.0;
    
    spherePositionArray = new float[18 * nb * nb];
    sphereIndexArray = new unsigned int[18 * nb * nb];
    sphereTexcoordArray = new float[12 * nb * nb];
    
    for(int i = 0; i < nb; i++){
        float theta = inc1 * i;
        for(int j = 0; j < nb; j++){
            float phi = inc2 * j;
            
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
            
            spherePositionArray[18 * (nb * i + j)]      = x3;
            spherePositionArray[18 * (nb * i + j) + 1]  = y3;
            spherePositionArray[18 * (nb * i + j) + 2]  = z3;
            spherePositionArray[18 * (nb * i + j) + 3]  = x2;
            spherePositionArray[18 * (nb * i + j) + 4]  = y2;
            spherePositionArray[18 * (nb * i + j) + 5]  = z2;
            spherePositionArray[18 * (nb * i + j) + 6]  = x1;
            spherePositionArray[18 * (nb * i + j) + 7]  = y1;
            spherePositionArray[18 * (nb * i + j) + 8]  = z1;
            spherePositionArray[18 * (nb * i + j) + 9]  = x2;
            spherePositionArray[18 * (nb * i + j) + 10] = y2;
            spherePositionArray[18 * (nb * i + j) + 11] = z2;
            spherePositionArray[18 * (nb * i + j) + 12] = x3;
            spherePositionArray[18 * (nb * i + j) + 13] = y3;
            spherePositionArray[18 * (nb * i + j) + 14] = z3;
            spherePositionArray[18 * (nb * i + j) + 15] = x4;
            spherePositionArray[18 * (nb * i + j) + 16] = y4;
            spherePositionArray[18 * (nb * i + j) + 17] = z4;
            
            sphereIndexArray[18 * (nb * i + j)]          = 18 * (nb * i + j);
            sphereIndexArray[18 * (nb * i + j) + 1]      = 18 * (nb * i + j) + 1;
            sphereIndexArray[18 * (nb * i + j) + 2]      = 18 * (nb * i + j) + 2;
            sphereIndexArray[18 * (nb * i + j) + 3]      = 18 * (nb * i + j) + 3;
            sphereIndexArray[18 * (nb * i + j) + 4]      = 18 * (nb * i + j) + 4;
            sphereIndexArray[18 * (nb * i + j) + 5]      = 18 * (nb * i + j) + 5;
            sphereIndexArray[18 * (nb * i + j) + 6]      = 18 * (nb * i + j) + 6;
            sphereIndexArray[18 * (nb * i + j) + 7]      = 18 * (nb * i + j) + 7;
            sphereIndexArray[18 * (nb * i + j) + 8]      = 18 * (nb * i + j) + 8;
            sphereIndexArray[18 * (nb * i + j) + 9]      = 18 * (nb * i + j) + 9;
            sphereIndexArray[18 * (nb * i + j) + 10]     = 18 * (nb * i + j) + 10;
            sphereIndexArray[18 * (nb * i + j) + 11]     = 18 * (nb * i + j) + 11;
            sphereIndexArray[18 * (nb * i + j) + 12]     = 18 * (nb * i + j) + 12;
            sphereIndexArray[18 * (nb * i + j) + 13]     = 18 * (nb * i + j) + 13;
            sphereIndexArray[18 * (nb * i + j) + 14]     = 18 * (nb * i + j) + 14;
            sphereIndexArray[18 * (nb * i + j) + 15]     = 18 * (nb * i + j) + 15;
            sphereIndexArray[18 * (nb * i + j) + 16]     = 18 * (nb * i + j) + 16;
            sphereIndexArray[18 * (nb * i + j) + 17]     = 18 * (nb * i + j) + 17;
            
            sphereTexcoordArray[12 * (nb * i + j)]      = j * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 1]  = 1 - (i + 1) * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 2]  = (j + 1) * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 3]  = 1 - i* 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 4]  = j * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 5]  = 1 - i * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 6]  = (j + 1) * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 7]  = 1 - i * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 8]  = j * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 9]  = 1 - (i + 1) * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 10] = (j + 1) * 1.0 / nb;
            sphereTexcoordArray[12 * (nb * i + j) + 11] = 1 - (i + 1) * 1.0 / nb;
        }
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(float), (GLvoid*)spherePositionArray);
    
    glEnableClientState (GL_NORMAL_ARRAY);
    glNormalPointer (GL_FLOAT, 3*sizeof (float), (GLvoid*)spherePositionArray);
    glEnable (GL_NORMALIZE); // preserve les vecteurs normaux unitaires, quelle que soit la transformation courante
    
    //2 sources of light
    glEnable (GL_LIGHTING);
    GLfloat light_position[4] = {10.0f, 10.0f, 10.0f, 1.0f};
    GLfloat color[4] = {1.0f, 1.0f, 0.9f, 1.0f};
    glLightfv (GL_LIGHT0, GL_POSITION, light_position); // On place la source N° 0 en (10,10,10)
    glLightfv (GL_LIGHT0, GL_DIFFUSE, color); // On lui donne légèrement orangée
    glLightfv (GL_LIGHT0, GL_SPECULAR, color); // Une hérésie, mais OpenGL est conçu comme cela
    glEnable (GL_LIGHT0); // On active la source 0
    
    GLfloat light_position2[4] = {-10.0f, 0.0f, -1.0f, 1.0f};
    GLfloat color2[4] = {0.0f, 0.1f, 0.3f, 1.0f};
    glLightfv (GL_LIGHT1, GL_POSITION, light_position2); // On place la source N° 0 en (10,10,10)
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color2); // On lui donne légèrement orangée
    glLightfv (GL_LIGHT1, GL_SPECULAR, color2); // Une hérésie, mais OpenGL est conçu comme cela
    glEnable (GL_LIGHT1); // On active la source 0
    
    //Texture
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    
    width = nb;
    height = nb;
    image = new unsigned char[3*width*height];
    
    genCheckerboard(width, height, image);
    glTexCoordPointer (2, GL_FLOAT, 2*sizeof (float), (GLvoid*)sphereTexcoordArray);
    
    glEnable (GL_TEXTURE_2D); // Activation de la texturation 2D
    glGenTextures (1, &texture); // Génération d’une texture OpenGL
    glBindTexture (GL_TEXTURE_2D, texture); // Activation de la texture comme texture courante
    // les 4 lignes suivantes paramètre le filtrage de texture ainsi que sa répétition au-delà du carré unitaire
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // La commande suivante remplit la texture (sur GPU) avec les données de l’image
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
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

void glSphere(float x, float y, float z, float r){
    glMatrixMode (GL_MODELVIEW);// Indique que l’on va désormais altérer la matrice modèle-vue
    glPushMatrix (); // pousse la matrice courante sur un pile
    glTranslatef (x, y, z); // applique une translation à la matrice [...] // dessin des polygones (glVertex3f, etc), dans le repère définit par la matrice model-vue
    glScalef (r, r, r);
    glRotatef(currentTime * acceleration / 25, 0, 1, 0);
    glDrawElements(GL_TRIANGLES, 6 * nb * nb, GL_UNSIGNED_INT, sphereIndexArray);
    glPopMatrix (); // replace la matrice modèle vue courante original
}

void glSphereWithMat (float x, float y, float z, float r, float difR, float difG, float difB,
                      float specR, float specG, float specB,
                      float shininess){
    GLfloat material_color[4] = {difR, difG, difB, 1.0f};
    GLfloat material_specular[4] = {specR, specG, specB,1.0};
    glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
    glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, material_color);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glBindTexture (GL_TEXTURE_2D, texture);
    glSphere(x, y, z, r);
}


void display () {
    setupCamera ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
    
    // Put your drawing code (glBegin, glVertex, glCallList, glDrawArray, etc) here
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glRotatef(currentTime * acceleration / 30, 0, 1, 0);
    glSphereWithMat(0, 0, 0, 1.2,
             1.0, 0.3,0.6,
             0.8,1.0,0.5,
             80);
    glPopMatrix ();
    
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glRotatef(currentTime * acceleration / 20, 0, 1, 0);
    glSphereWithMat(-7, 0, 0, 0.6,
             0.9, 1,0.6,
             0.1,0.4,0.7,
             80);
    glPopMatrix ();
    
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glRotatef(currentTime * acceleration / 8, 0, 1, 0);
    glSphereWithMat(4, 0, 0, 0.3,
                    0.9, 1,0.6,
                    0.1,0.4,0.7,
                    80);
    glPopMatrix ();
    
    glFlush (); // Ensures any previous OpenGL call has been executed
    glutSwapBuffers ();  // swap the render buffer and the displayed (screen) one
}


void keyboard (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
        case '1':
        case 'x':
            if(glIsEnabled(GL_LIGHT0)) glDisable(GL_LIGHT0);
            else glEnable(GL_LIGHT0);
            break;
        case '2':
        case 'c':
            if(glIsEnabled(GL_LIGHT1)) glDisable(GL_LIGHT1);
            else glEnable(GL_LIGHT1);
            break;
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
     if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)  {
     mouse_init_x = (double)x;
     mouse_init_y = (double)y;
     }
}

void motion (int x, int y) {
     if (((double)x) != mouse_init_x) {
         camPhi += ((double)x - mouse_init_x) / 8000;
     }
     if (((double)y) != mouse_init_y) {
         camTheta += ((double)y - mouse_init_y) / 8000;
     }
}

void specialKey(GLint key, GLint x, GLint y){
    switch (key) {
            break;
   	    case GLUT_KEY_UP:
            camDist2Target -= 0.5;
            break;
        case GLUT_KEY_DOWN:
            camDist2Target += 0.5;
            break;
        case GLUT_KEY_LEFT:
            camTargetX += 0.25;
            break;
        case GLUT_KEY_RIGHT:
            camTargetX -= 0.25;
            break;
        default:
            break;
    }
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
    glutSpecialFunc(specialKey);
    glutMouseFunc (mouse); // Callback function executed when a mouse button is clicked
    glutMotionFunc (motion); // Callback function executed when the mouse move
    glutIdleFunc (idle); // Callback function executed continuously when no other event happens (good for background procesing or animation for instance).
    printUsage (); // By default, display the usage help of the program
    glutMainLoop ();
    return 0;
}
