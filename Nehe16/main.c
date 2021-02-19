//From cygwin version of Nehe tutorial lesson16

// This code was created by Jeff Molofee '99 (ported to Linux/GLUT by Richard Campbell '99)
//
// If you've found this code useful, please let me know.
//
// Visit me at www.demonews.com/hosted/nehe 
// (email Richard Campbell at ulmont@bellsouth.net)
//

// this was modified to work on PSP by Edorul (edorul@free.fr)
// Many Thanks to  jared bruni (jared@lostsidedead.com) for is
// MasterPiece3D port to PSP : it gave me a good sample and not
// the least a working makefile !

// important notes :  - all modified portion of code from cygwin version
//                      of Nehe tutorial are marked with @@@
//                    - I leaved many printf in this source code to stay
//                      as close as possible of the cygwin version of 
//                      Nehe tutorial, but they won't be displayed on screen
//               ========================================================
//               ==                                                    ==
//               ==  For the Fog :                                     ==
//               ==                                                    ==
//               == GL_FOG_END MUST BE DECLARED BEFORE GL_FOG_START    ==
//               == ELSE YOU'LL HAVE AN INVERSED RESULT !!!!!          ==
//               == except if you use the modified glFog.c             ==
//               ========================================================

// Used keys :
// START = exit 
// PAD arrows = turn the cube
// trigger left = move the cube into the distance.
// trigger right =  move the cube closer.
// circle = switch the filter.
// square = switch the light on/off.
// cross = switch diferent types of fog (but it's alway GL_LINEAR the only mode PSPGL can use)

#include <GL/glut.h>    /* Header File For The GLUT Library  */
#include <GL/gl.h>	/* Header File For The OpenGL Library  */
#include <GL/glu.h>	/* Header File For The GLu Library  */
#include <stdio.h>      /* Header file for standard file i/o.  */
#include <stdlib.h>     /* Header file for malloc/free.  */
#include <math.h>       /* Header file for trigonometric functions.  */

/* ascii codes for various special keys */
// #define ESCAPE 27 @@@

int window;				/* GLUT window ID */

GLfloat	xrot;				/* X Rotation  */
GLfloat	yrot;				/* Y Rotation  */
GLfloat xspeed;				/* X Rotation Speed  */
GLfloat yspeed;				/* Y Rotation Speed  */
GLfloat	z=-5.0f;			/* Depth Into The Screen  */

/* Used to toggle full screen/window mode */
//int fullscreen=0;	/* toggle fullscreen */ @@@
int x_position = 50;	/* position on screen */
int y_position = 50;
//int width = 640;	/ Size  // @@@
//int height = 480;	// @@@

GLuint light;			/* Lighting toggle */
GLfloat LightAmbient[]=		{ 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]=		{ 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[]=	{ 0.0f, 0.0f, 2.0f, 1.0f };
GLuint	filter;				/* Which Filter To Use */
GLuint	texture[3];			/* Storage For 3 Textures */
// @@@ only GL_LINEAR mode in PSPGL :
GLuint	fogMode[]= { GL_LINEAR, GL_LINEAR, GL_LINEAR };	// Storage For Three Types Of Fog 
GLuint	fogfilter = 0;					/* Which Fog Mode To Use  */
GLfloat	fogColor[4] = {0.5f,0.5f,0.5f,1.0f};		/* Fog Color */

/* Image type - contains height, width, and data */
typedef struct {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
} Image;

/*
 * getint and getshort are help functions to load the bitmap byte by byte on 
 * SPARC platform (actually, just makes the thing work on platforms of either
 * endianness, not just Intel's little endian)
 */
static unsigned int getint(fp)
     FILE *fp;
{
  int c, c1, c2, c3;

  /* get 4 bytes */
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(fp)
     FILE *fp;
{
  int c, c1;
  
  /* get 2 bytes */
  c = getc(fp);  
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/* quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.   */
/* See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info. */
int ImageLoad(char *filename, Image *image) 
{
    FILE *file;
    unsigned long size;                 /* size of the image in bytes. */
    unsigned long i;                    /* standard counter. */
    unsigned short int planes;          /* number of planes in image (must be 1)  */
    unsigned short int bpp;             /* number of bits per pixel (must be 24) */
    char temp;                          /* used to convert bgr to rgb color. */

    /* make sure the file is there. */
    if ((file = fopen(filename, "rb"))==NULL) {
      printf("File Not Found : %s\n",filename);
      return 0;
    }
    
    /* seek through the bmp header, up to the width/height: */
    fseek(file, 18, SEEK_CUR);

    /* No 100% errorchecking anymore!!! */

    /* read the width */
    image->sizeX = getint (file);
    printf("Width of %s: %lu\n", filename, image->sizeX);
    
    /* read the height  */
    image->sizeY = getint (file);
    printf("Height of %s: %lu\n", filename, image->sizeY);
    
    /* calculate the size (assuming 24 bits or 3 bytes per pixel). */
    size = image->sizeX * image->sizeY * 3;

    /* read the planes */
    planes = getshort(file);
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    /* read the bpp */
    bpp = getshort(file);
    if (bpp != 24) {
      printf("Bpp from %s is not 24: %u\n", filename, bpp);
      return 0;
    }
	
    /* seek past the rest of the bitmap header. */
    fseek(file, 24, SEEK_CUR);

    /* read the data.  */
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { /* reverse all of the colors. (bgr -> rgb) */
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }

    /* we're done. */
    return 1;
}

/* Load Bitmaps And Convert To Textures */
void LoadGLTextures()	
{
    /* Load Texture */
    Image *TextureImage;
    
    /* allocate space for texture */
    TextureImage = (Image *) malloc(sizeof(Image));
    if (TextureImage == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Data/Crate.bmp", TextureImage)) {
	exit(1);
    }        
		glGenTextures(3, &texture[0]);		/* Create Three Textures */

		/* Create Nearest Filtered Texture */
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, 
							GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);

		/* Create Linear Filtered Texture */
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, 
							GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);

		/* Create MipMapped Texture */
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage->sizeX, TextureImage->sizeY, 
							GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
}

/* Resize And Initialize The GL Window */
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height==0)		/* Prevent A Divide By Zero By */
	{
		height=1;	/* Making Height Equal One */
	}

	glViewport(0,0,width,height);	/* Reset The Current Viewport */

	glMatrixMode(GL_PROJECTION);	/* Select The Projection Matrix */
	glLoadIdentity();		/* Reset The Projection Matrix */

	/* Calculate The Aspect Ratio Of The Window */
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);	/* Select The Modelview Matrix */
	glLoadIdentity();		/* Reset The Modelview Matrix */
}

void InitGL(GLsizei Width, GLsizei Height) /* All Setup For OpenGL Goes Here */
{
	LoadGLTextures();	

	glEnable(GL_TEXTURE_2D);		/* Enable Texture Mapping */
	glShadeModel(GL_SMOOTH);		/* Enable Smooth Shading */
	glClearColor(0.5f,0.5f,0.5f,1.0f);	/* We'll Clear To The Color Of The Fog */
	glClearDepth(1.0f);			/* Depth Buffer Setup */
	glEnable(GL_DEPTH_TEST);		/* Enables Depth Testing */
	glDepthFunc(GL_LEQUAL);			/* The Type Of Depth Testing To Do */
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	/* Really Nice Perspective Calculations */ @@@ glHint doesn't exist in pspgl

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		/* Setup The Ambient Light */
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		/* Setup The Diffuse Light */
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);	/* Position The Light */
	glEnable(GL_LIGHT1);					/* Enable Light One */

	glFogi(GL_FOG_MODE, fogMode[fogfilter]);	/* Fog Mode */
	glFogfv(GL_FOG_COLOR, fogColor);		/* Set Fog Color */
	glFogf(GL_FOG_DENSITY, 0.35f);			/* How Dense Will The Fog Be */
//	glHint(GL_FOG_HINT, GL_DONT_CARE);		/* Fog Hint Value */ @@@ glHint doesn't exist in pspgl

	// @@@
	glFogf(GL_FOG_END, 5.0f);			// Fog End Depth  @@@ GL_FOG_END MUST BE DECLARED BEFORE GL_FOG_START...
	glFogf(GL_FOG_START, 1.0f);			// Fog Start Depth @@@ ...ELSE YOU'LL HAVE AN INVERSED RESULT !!!!!

	glEnable(GL_FOG);				/* Enables GL_FOG */
}

void DrawGLScene(GLvoid)	/* Here's Where We Do All The Drawing */
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	/* Clear The Screen And The Depth Buffer */
	glLoadIdentity();					/* Reset The View */
	glTranslatef(0.0f,0.0f,z);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);

	glBindTexture(GL_TEXTURE_2D, texture[filter]);

// @@@ GL_QUADS not implemented in pspgl : it's internaly replaced with GL_TRIANGLE_FAN
// @@@ so we need a glEnd() for each Quad in oder to have the right shape
	glBegin(GL_QUADS);
		/* Front Face */
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
  glEnd();					// @@@ because GL_QUADS not implemented in pspgl
		/* Back Face */
  glBegin(GL_QUADS);				// @@@ because GL_QUADS not implemented in pspgl
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
  glEnd();					// @@@ because GL_QUADS not implemented in pspgl
		/* Top Face */
  glBegin(GL_QUADS);				// @@@ because GL_QUADS not implemented in pspgl
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
  glEnd();					// @@@ because GL_QUADS not implemented in pspgl
		/* Bottom Face */
  glBegin(GL_QUADS);				// @@@ because GL_QUADS not implemented in pspgl
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
  glEnd();					// @@@ because GL_QUADS not implemented in pspgl
		/* Right face */
  glBegin(GL_QUADS);				// @@@ because GL_QUADS not implemented in pspgl
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
  glEnd();					// @@@ because GL_QUADS not implemented in pspgl
		/* Left Face */
  glBegin(GL_QUADS);				// @@@ because GL_QUADS not implemented in pspgl
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	xrot+=xspeed;
	yrot+=yspeed;

/* Swap buffers because we are in double buffer mode */
	glutSwapBuffers();
}

/* The function called whenever a normal key is pressed. */
// @@@ keys are modified to match PSP pad
void keyPressed(unsigned char key, int x, int y) 
{
   switch (key) {    
    case 'x': /* cross */ 	//  switch the fog.
		fogfilter+=1;
		if (fogfilter>2)
		{
			fogfilter=0;
		}
		glFogi (GL_FOG_MODE, fogMode[fogfilter]); /* Fog Mode */
		printf("Fog mode is %d\n",fogfilter);
		break;
	case 'o':			/* round */ // switch the filter.
		printf("F/f pressed; filter is: %d\n", filter);
		filter+=1;
		if (filter>2) {
			filter=0;	
		}	
		printf("Filter is now: %d\n", filter);
		break;
	case 'q':			/* square*/  // switch the lighting.
		printf("L/l pressed; light is: %d\n", light);
		light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
		printf("Light is now: %d\n", light);
		if (!light) {
			glDisable(GL_LIGHTING);
		} else {
			glEnable(GL_LIGHTING);
		}
		break;
	case 'a':			/* startbutton */
		/* shut down our window */
//		glutDestroyWindow(window);  @@@ don't exist in pspgl
	
		/* exit the program...normal termination. */
		exit(0);                   
	break;
    default:
	break;
    }	
}

/* The function called whenever the triggers are pressed. */
// @@@ this function is added to use PSP triggers
void triggerHandle (int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {  // left trigger...
		if (state == GLUT_DOWN) {  // ...is pressed
			z-=0.02f; // move into the distance.
		}
		if (state == GLUT_UP) {  // ...is released
		}
	}

	if (button == GLUT_RIGHT_BUTTON) {  // right trigger...
		if (state == GLUT_DOWN) {  // ...is pressed
			z+=0.02f; // move closer
		}
		if (state == GLUT_DOWN) {  // ...is released
		}
	}
}

/* The function called whenever a normal key is pressed. */
// @@@ parts are modified to match PSP pad
void specialKeyPressed(int key, int x, int y) 
{
    switch (key) {   
// @@@ no need of fullscreen/windowed mode on PSP : it's allways fullscreen	
/*	case GLUT_KEY_F1:
	fullscreen = !fullscreen;
	if (fullscreen) {
	    x_position = glutGet((GLenum)GLUT_WINDOW_X);	// Save parameters 
	    y_position = glutGet((GLenum)GLUT_WINDOW_Y);
	    width = glutGet((GLenum)GLUT_WINDOW_WIDTH);
	    height = glutGet((GLenum)GLUT_WINDOW_HEIGHT);
	    glutFullScreen();				// Go to full screen 
	} else {
	    glutReshapeWindow(width, height);		// Restore us 
	    glutPositionWindow(x_position, y_position);
	}
		break;
*/     
// @@@ moved to triggerPressed function
/*	case GLUT_KEY_PAGE_UP: //  move the cube into the distance. 
	z-=0.02f;
	break;
    
    case GLUT_KEY_PAGE_DOWN: //  move the cube closer. 
	z+=0.02f;
	break;
*/
    case GLUT_KEY_UP: /*  decrease x rotation speed; */
	xspeed-=0.01f;
	break;

    case GLUT_KEY_DOWN: /*  increase x rotation speed; */
	xspeed+=0.01f;
	break;

    case GLUT_KEY_LEFT: /*  decrease y rotation speed; */
	yspeed-=0.01f;
	break;
    
    case GLUT_KEY_RIGHT: /*  increase y rotation speed; */
	yspeed+=0.01f;
	break;

    default:
	break;
    }	
}

/* Routine to save our CPU */
// @@@ not used on PSP
/*void Icon( int state )
{
	switch (state)
	{
	case GLUT_VISIBLE:
		glutIdleFunc(DrawGLScene); 
		break;
	case GLUT_NOT_VISIBLE:
		glutIdleFunc(NULL); 
		break;
	default:
		break;
	}
	
}
*/

int main(int argc, char **argv) 
{  

    /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
       X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
    glutInit(&argc, argv);  

    /* Select type of Display mode: 
     Double buffer 
     RGBA color
     Depth buffer 
     Alpha blending */  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  

    /* get a 640 x 480 window */
    glutInitWindowSize(480, 272);  // @@@

    /* the window starts at the upper left corner of the screen */
    glutInitWindowPosition(0, 0);  

    /* Open a window */  
    window = glutCreateWindow("Jeff Molofee's GL Code Tutorial ... NeHe '99");  

    /* Register the function to do all our OpenGL drawing. */
    glutDisplayFunc(DrawGLScene);  

    /* Even if there are no events, redraw our gl scene. */
    glutIdleFunc(DrawGLScene); 

    /* Register the function called when our window is resized. */
    glutReshapeFunc(ReSizeGLScene);

    /* Register the function called when the keyboard is pressed. */
    glutKeyboardFunc(keyPressed);

	/* Register the function called when Trigger_left or Trigger_right is pressed */
	glutMouseFunc(&triggerHandle); // @@@ added to use PSP triggers

    /* Register the function called when special keys (arrows, page down, etc) are pressed. */
    glutSpecialFunc(specialKeyPressed);

    /* Use this to save CPU when the scene is obscured/iconised */
//    glutVisibilityFunc(Icon); @@@ not used on PSP

    /* Initialize our window. */
    InitGL(480, 272);  // @@@
  
    /* Start Event Processing Engine */  
    glutMainLoop();  

    return 1;
}
