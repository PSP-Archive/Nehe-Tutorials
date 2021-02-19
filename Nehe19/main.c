// Nehe Lesson 19

/*
 *		This Code Was Created By Jeff Molofee 2000
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 *
 *		this linux port by Ken Rockot ( read README )
 */
 
// this was modified to work on PSP by Edorul (edorul@free.fr)
// Many Thanks to  jared bruni (jared@lostsidedead.com) for is
// MasterPiece3D port to PSP : it gave me a good sample and not
// the least a working makefile !

// important notes :  - all modified portion of code from cygwin version
//                      of Nehe tutorial are marked with @@@
//                    - I leaved many printf in this source code to stay
//                      as close as possible of the cygwin version of 
//                      Nehe tutorial, but they won't be displayed on screen

// Used keys :
// START = exit 
// SELECT = cause a burst
// PAD arrows = change gravity strength and direction
// trigger left = zoom out.
// trigger right =  zoom in.
// circle = slow down particles
// square = rainbow on/off
// triangle = Change The Particle Color + rainbow off
// cross = accelerate particles
// joystick = change speed

#include <stdlib.h>
#include <stdio.h>					// Header File For Standard Input/Output
#include <GL/gl.h>					// Header File For The OpenGL 
#include <GL/glu.h>					// Header File For The GLu
#include <GL/glut.h>				// Header File For The GLut

#define	MAX_PARTICLES	1000				// Number Of Particles To Create

/* The number of our GLUT window */
int window; 

GLboolean	rainbow=GL_TRUE;					// Rainbow Mode?
GLboolean	tp;						// triangle key Pressed?
GLboolean	sp;						// square Key Pressed?
GLboolean	lp, rp, up, dp, selp;			//Left, Right, Up, Down, Select key pressed?

GLfloat	slowdown=2.0f;					// Slow Down Particles
GLfloat	xspeed;						// Base X Speed (To Allow Keyboard Direction Of Tail)
GLfloat	yspeed;						// Base Y Speed (To Allow Keyboard Direction Of Tail)
GLfloat	zoom=-40.0f;					// Used To Zoom Out

GLuint	loop;						// Misc Loop Variable
GLuint	col;						// Current Color Selection
GLuint	delay;						// Rainbow Effect Delay
GLuint	texture[1];					// Storage For Our Particle Texture

typedef struct						// Create A Structure For Particle
{
	GLboolean	active;					// Active (Yes/No)
	float	life;					// Particle Life
	float	fade;					// Fade Speed
	float	r;					// Red Value
	float	g;					// Green Value
	float	b;					// Blue Value
	float	x;					// X Position
	float	y;					// Y Position
	float	z;					// Z Position
	float	xi;					// X Direction
	float	yi;					// Y Direction
	float	zi;					// Z Direction
	float	xg;					// X Gravity
	float	yg;					// Y Gravity
	float	zg;					// Z Gravity
}
particles;						// Particles Structure

particles particle[MAX_PARTICLES];			// Particle Array (Room For Particle Info)

static GLfloat colors[12][3]=				// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

/* Image type - contains height, width, and data */
struct Image {
    unsigned long sizeX;
    unsigned long sizeY;
    char *data;
};
typedef struct Image Image;

// quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  
// See http://www.dcs.ed.ac.uk/~mxr/gfx/2d/BMP.txt for more info.
// if mesa ever gets glaux, let me know.
int ImageLoad(char *filename, Image *image) {
    FILE *file;
    unsigned long size;                 // size of the image in bytes.
    unsigned long i;                    // standard counter.
    unsigned short int planes;          // number of planes in image (must be 1) 
    unsigned short int bpp;             // number of bits per pixel (must be 24)
    char temp;                          // used to convert bgr to rgb color.

    // make sure the file is there.
    if ((file = fopen(filename, "rb"))==NULL)
    {
	printf("File Not Found : %s\n",filename);
	return 0;
    }
    
    // seek through the bmp header, up to the width/height:
    fseek(file, 18, SEEK_CUR);

    // read the width
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
	printf("Error reading width from %s.\n", filename);
	return 0;
    }
    printf("Width of %s: %lu\n", filename, image->sizeX);
    
    // read the height 
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
	printf("Error reading height from %s.\n", filename);
	return 0;
    }
    printf("Height of %s: %lu\n", filename, image->sizeY);
    
    // calculate the size (assuming 24 bits or 3 bytes per pixel).
    size = image->sizeX * image->sizeY * 3;

    // read the planes
    if ((fread(&planes, 2, 1, file)) != 1) {
	printf("Error reading planes from %s.\n", filename);
	return 0;
    }
    if (planes != 1) {
	printf("Planes from %s is not 1: %u\n", filename, planes);
	return 0;
    }

    // read the bpp
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
	printf("Error reading bpp from %s.\n", filename);
	return 0;
    }
    if (bpp != 24) {
	printf("Bpp from %s is not 24: %u\n", filename, bpp);
	return 0;
    }
	
    // seek past the rest of the bitmap header.
    fseek(file, 24, SEEK_CUR);

    // read the data. 
    image->data = (char *) malloc(size);
    if (image->data == NULL) {
	printf("Error allocating memory for color-corrected image data");
	return 0;	
    }

    if ((i = fread(image->data, size, 1, file)) != 1) {
	printf("Error reading image data from %s.\n", filename);
	return 0;
    }

    for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
	temp = image->data[i];
	image->data[i] = image->data[i+2];
	image->data[i+2] = temp;
    }

    // we're done.
    return 1;
}

GLvoid LoadGLTextures()							// Load Bitmap And Convert To A Texture
{
    // Load Texture
    Image *image1;
    
    // allocate space for texture
    image1 = (Image *) malloc(sizeof(Image));
    if (image1 == NULL) {
	printf("Error allocating space for image");
	exit(0);
    }

    if (!ImageLoad("Data/Particle.bmp", image1)) {
	exit(1);
    }        

	// do stuff
	glGenTextures(1, &texture[0]);				// Create One Texture

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 32, 32,
		0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)			// Resize And Initialize The GL Window
{
	if (height==0)							// Prevent A Divide By Zero By
	{
		height=1;						// Making Height Equal One
	}

	glViewport(0,0,width,height);					// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);					// Select The Projection Matrix
	glLoadIdentity();						// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,200.0f);

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();						// Reset The Modelview Matrix
}

GLvoid InitGL(int Width, int Height)							// All Setup For OpenGL Goes Here
{
	LoadGLTextures();						// Jump To Texture Loading Routine

	glShadeModel(GL_SMOOTH);					// Enable Smooth Shading
	glClearColor(0.0f,0.0f,0.0f,0.0f);				// Black Background
	glClearDepth(1.0f);						// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);					// Disable Depth Testing
	glEnable(GL_BLEND);						// Enable Blending
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);				// Type Of Blending To Perform
//	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);		// Really Nice Perspective Calculations @@@ doesn't exist in pspgl
//	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				// Really Nice Point Smoothing @@@ doesn't exist in pspgl
	glEnable(GL_TEXTURE_2D);					// Enable Texture Mapping
	glBindTexture(GL_TEXTURE_2D,texture[0]);			// Select Our Texture

	for (loop=0;loop<MAX_PARTICLES;loop++)				// Initials All The Textures
	{
		particle[loop].active=GL_TRUE;				// Make All The Particles Active
		particle[loop].life=1.0f;				// Give All The Particles Full Life
		particle[loop].fade=(GLfloat)(rand()%100)/1000.0f+0.003f;	// Random Fade Speed
		particle[loop].r=colors[(loop+1)/(MAX_PARTICLES/12)][0];	// Select Red Rainbow Color
		particle[loop].g=colors[(loop+1)/(MAX_PARTICLES/12)][1];	// Select Green Rainbow Color
		particle[loop].b=colors[(loop+1)/(MAX_PARTICLES/12)][2];	// Select Blue Rainbow Color
		particle[loop].xi=(GLfloat)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
		particle[loop].yi=(GLfloat)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
		particle[loop].zi=(GLfloat)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
		particle[loop].xg=0.0f;					// Set Horizontal Pull To Zero
		particle[loop].yg=-0.8f;				// Set Vertical Pull Downward
		particle[loop].zg=0.0f;					// Set Pull On Z Axis To Zero
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix

	gluPerspective(45.0f,(float)Width/(float)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);
}

GLvoid DrawGLScene(GLvoid)							// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer
	glLoadIdentity();						// Reset The ModelView Matrix

	for (loop=0;loop<MAX_PARTICLES;loop++)				// Loop Through All The Particles
	{
		if (particle[loop].active)				// If The Particle Is Active
		{
			GLfloat x=particle[loop].x;			// Grab Our Particle X Position
			GLfloat y=particle[loop].y;			// Grab Our Particle Y Position
			GLfloat z=particle[loop].z+zoom;			// Particle Z Pos + Zoom

			// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
			glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);				// Build Quad From A Triangle Strip
			    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z);	// Top Right
				glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); // Top Left
				glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); // Bottom Right
				glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); // Bottom Left
			glEnd();						// Done Building Triangle Strip

			particle[loop].x+=particle[loop].xi/(slowdown*1000);// Move On The X Axis By X Speed
			particle[loop].y+=particle[loop].yi/(slowdown*1000);// Move On The Y Axis By Y Speed
			particle[loop].z+=particle[loop].zi/(slowdown*1000);// Move On The Z Axis By Z Speed

			particle[loop].xi+=particle[loop].xg;			// Take Pull On X Axis Into Account
			particle[loop].yi+=particle[loop].yg;			// Take Pull On Y Axis Into Account
			particle[loop].zi+=particle[loop].zg;			// Take Pull On Z Axis Into Account
			particle[loop].life-=particle[loop].fade;		// Reduce Particles Life By 'Fade'

			if (particle[loop].life<0.0f)					// If Particle Is Burned Out
			{
				particle[loop].life=1.0f;				// Give It New Life
				particle[loop].fade=(GLfloat)(rand()%100)/1000.0f+0.003f;	// Random Fade Value
				particle[loop].x=0.0f;					// Center On X Axis
				particle[loop].y=0.0f;					// Center On Y Axis
				particle[loop].z=0.0f;					// Center On Z Axis
				particle[loop].xi=xspeed+(GLfloat)((rand()%60)-32.0f);	// X Axis Speed And Direction
				particle[loop].yi=yspeed+(GLfloat)((rand()%60)-30.0f);	// Y Axis Speed And Direction
				particle[loop].zi=(GLfloat)((rand()%60)-30.0f);		// Z Axis Speed And Direction
				particle[loop].r=colors[col][0];			// Select Red From Color Table
				particle[loop].g=colors[col][1];			// Select Green From Color Table
				particle[loop].b=colors[col][2];			// Select Blue From Color Table
			}

			// If up arrow And Y Gravity Is Less Than 1.5 Increase Pull Upwards
			if (up && (particle[loop].yg<1.5f)) particle[loop].yg+=0.05f;

			// If down arrow And Y Gravity Is Greater Than -1.5 Increase Pull Downwards
			if (dp && (particle[loop].yg>-1.5f)) particle[loop].yg-=0.05f;

			// If right arrow And X Gravity Is Less Than 1.5 Increase Pull Right
			if (rp && (particle[loop].xg<1.5f)) particle[loop].xg+=0.05f;

			// If left arrow And X Gravity Is Greater Than -1.5 Increase Pull Left
			if (lp && (particle[loop].xg>-1.5f)) particle[loop].xg-=0.05f;

			if (selp)						// select Key Causes A Burst
			{
				particle[loop].x=0.0f;					// Center On X Axis
				particle[loop].y=0.0f;					// Center On Y Axis
				particle[loop].z=0.0f;					// Center On Z Axis
				particle[loop].xi=(GLfloat)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
				particle[loop].yi=(GLfloat)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
				particle[loop].zi=(GLfloat)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
			}
		}
    	}

	delay++;			// Increase Rainbow Mode Color Cycling Delay Counter
	if ((rainbow)&&(delay>25)){
		delay = 0;
		col++;							// Change The Particle Color
		if (col>11)	col=0;					// If Color Is To High Reset It
	}

    // since this is double buffered, swap the buffers to display what just got drawn.
    glutSwapBuffers();
}

/* The function called whenever a normal key is pressed. */
// @@@ keys are modified
void keyPressed(unsigned char key, int x, int y) 
{
    switch (key) {    
	case 'o':			/* round */ // slow down.
		if (slowdown<4.0f) slowdown+=0.05f;	// Slow Down Particles
	break;
	case 'x':			/* cross */  // accelerate.
		if (slowdown>0.0f) slowdown-=0.05f;
	break;
	case 'd':			/* delta, triangle */
		if (!tp) // new triangle press?
		{
			rainbow = GL_FALSE;
			tp=GL_TRUE;						// Set Flag Telling Us Triangle Is Pressed
			col++;							// Change The Particle Color
			if (col>11)	col=0;					// If Color Is To High Reset It
		}
	break;
	case 'q':			/* square*/ // rainbow on/off
		if (!sp)					// new square press?
		{
			sp=GL_TRUE;						// Set Flag Telling Us It's Pressed
			rainbow=!rainbow;					// Toggle Rainbow Mode On / Off
		}
	break;
	case 's':			/* select */ // cause a burst in DrawGLScene function
		selp = GL_TRUE;
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

/* The function called whenever a key is released. */
void keyReleased(unsigned char key, int x, int y) 
{
	switch (key) {
	case 'd':			/* delta, triangle */
		tp=GL_FALSE;				// If triangle Is Released Clear Flag
	break;
	case 'q':			/* square*/
		sp=GL_FALSE;				// If Return Is Released Clear Flag
	break;
	case 's':			/* select */
		selp = GL_FALSE;
	break;
	default:
	;
	}
 }

/* The function called whenever the triggers are pressed. */
// @@@ this function is added to use PSP triggers
void triggerHandle (int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {  // left trigger...
		if (state == GLUT_DOWN) {  // ...is pressed
			zoom-=0.5f;				// Zoom out
		}
		if (state == GLUT_UP) {  // ...is released
		}
	}

	if (button == GLUT_RIGHT_BUTTON) {  // right trigger...
		if (state == GLUT_DOWN) {  // ...is pressed
			zoom+=0.5f;				// Zoom In
		}
		if (state == GLUT_DOWN) {  // ...is released
		}
	}
}

/* The function called whenever a normal key is pressed. */
void specialKeyPressed(int key, int x, int y) 
{
    switch (key) {    
    case GLUT_KEY_UP: // change gravity in DrawGLScene function
		up = GL_TRUE;
	break;

    case GLUT_KEY_DOWN: // change gravity in DrawGLScene function
		dp = GL_TRUE;
	break;

    case GLUT_KEY_LEFT: // change gravity in DrawGLScene function
		lp = GL_TRUE;
	break;
    
    case GLUT_KEY_RIGHT: // change gravity in DrawGLScene function
		rp = GL_TRUE;
	break;

    default:
	break;
    }	
}

/* The function called whenever a special key is released. */
void specialKeyReleased(int key, int x, int y) 
{
    switch (key) {    
    case GLUT_KEY_UP: // pad arrow up
		up = GL_FALSE;
	break;

    case GLUT_KEY_DOWN: //  pad arrow down
		dp = GL_FALSE;
	break;

    case GLUT_KEY_LEFT: //  pad arrow left
		lp = GL_FALSE;
	break;
    
    case GLUT_KEY_RIGHT: //  pad arrow right
		rp = GL_FALSE;
	break;

    default:
	break;
    }	
}

/* The function called whenever the joystick is moved. */
void joystickMoved (unsigned int buttonMask, int x, int y, int z)
{
	// If Right Arrow And X Speed Is Less Than 200 Increase Speed To The Right
	if (x > 150) // dead zone
	{	
		if (xspeed<200)
			xspeed+=3.0f;
	}
	// If Left Arrow And X Speed Is Greater Than -200 Increase Speed To The Left
	if (x < 150) // dead zone
	{	
		if (xspeed>-200)
			xspeed-=3.0f;
	}

	// If Up Arrow And Y Speed Is Less Than 200 Increase Upward Speed
	if (y > 150) // dead zone
	{	
		if (yspeed<200) 
			yspeed+=3.0f;
	}
	// If Down Arrow And Y Speed Is Greater Than -200 Increase Downward Speed
	if (y < 150) // dead zone
	{	
		if (yspeed>-200) 
			yspeed-=3.0f;
	}
}

/*
void check_keys ()
{
	if (keys[SDLK_KP_PLUS] && (slowdown>0.0f)) slowdown-=0.01f;
	if (keys[SDLK_KP_MINUS] && (slowdown<4.0f)) slowdown+=0.01f;	// Slow Down Particles

	if (keys[SDLK_PAGEUP])	zoom+=0.1f;				// Zoom In
	if (keys[SDLK_PAGEDOWN])	zoom-=0.1f;			// Zoom Out

	if (keys[SDLK_RETURN] && !sp)					// Return Key Pressed
	{
		sp=GL_TRUE;						// Set Flag Telling Us It's Pressed
		rainbow=!rainbow;					// Toggle Rainbow Mode On / Off
	}
	if (!keys[SDLK_RETURN]) sp=GL_FALSE;				// If Return Is Released Clear Flag
				
	if ((keys[SDLK_SPACE] && !tp) || (rainbow && (delay>25)))	// Space Or Rainbow Mode
	{
		if (keys[SDLK_SPACE])	rainbow=GL_FALSE;			// If Spacebar Is Pressed Disable Rainbow Mode
		tp=GL_TRUE;						// Set Flag Telling Us Space Is Pressed
		delay=0;						// Reset The Rainbow Color Cycling Delay
		col++;							// Change The Particle Color
		if (col>11)	col=0;					// If Color Is To High Reset It
	}
	if (!keys[SDLK_SPACE])	tp=GL_FALSE;				// If Spacebar Is Released Clear Flag

	// If Up Arrow And Y Speed Is Less Than 200 Increase Upward Speed
	if (keys[SDLK_UP] && (yspeed<200)) yspeed+=1.0f;

	// If Down Arrow And Y Speed Is Greater Than -200 Increase Downward Speed
	if (keys[SDLK_DOWN] && (yspeed>-200)) yspeed-=1.0f;

	// If Right Arrow And X Speed Is Less Than 200 Increase Speed To The Right
	if (keys[SDLK_RIGHT] && (xspeed<200)) xspeed+=1.0f;

	// If Left Arrow And X Speed Is Greater Than -200 Increase Speed To The Left
	if (keys[SDLK_LEFT] && (xspeed>-200)) xspeed-=1.0f;
}
*/

int main ( int argc, char **argv )
{
    /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
       X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
    glutInit(&argc, argv);  

    /* Select type of Display mode:   
     Double buffer 
     RGBA color
     Depth buffer */  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  

    /* get a 640 x 480 window */
    glutInitWindowSize(480, 272);  // @@@  

    /* the window starts at the upper left corner of the screen */
    glutInitWindowPosition(0, 0);  

    /* Open a window */  
    window = glutCreateWindow("Jeff Molofee's GL Code Tutorial ... NeHe '99");  

    /* Register the function to do all our OpenGL drawing. */
    glutDisplayFunc(&DrawGLScene);  

    /* Go fullscreen.  This is as soon as possible. */
//    glutFullScreen(); @@@

    /* Even if there are no events, redraw our gl scene. */
    glutIdleFunc(&DrawGLScene);

    /* Register the function called when our window is resized. */
    glutReshapeFunc(&ReSizeGLScene);

	/* @@@ repeat for all keys */
	glutKeyRepeat(GLUT_KEYREPEAT_ALL); // @@@ special in Edorul's glut modification, doesn't exist in real glut!!!

    /* Register the function called when the keyboard is pressed or released. */
    glutKeyboardFunc(&keyPressed);
	glutKeyboardUpFunc(&keyReleased);

    /* Register the function called when Trigger_left or Trigger_right is pressed */
    glutMouseFunc(&triggerHandle); // @@@ added to use PSP triggers
  
    /* Register the function called when special keys (arrows, page down, etc) are pressed or released. */
    glutSpecialFunc(&specialKeyPressed);
	glutSpecialUpFunc(&specialKeyReleased);

	/* Register the function called when joystick is moved. */
	glutJoystickFunc(&joystickMoved, 0); // 0 = Joystick polling interval in milliseconds

	/* Initialize our window. */
    InitGL(480, 272);  // @@@ 
  
    /* Start Event Processing Engine */  
    glutMainLoop();  

    return 1;
}
