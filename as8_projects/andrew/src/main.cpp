#include "main.h"
#include "LoadImage.h"

using namespace std;


//****************************************************
// Some Classes
//****************************************************
class Viewport {
public:
    Viewport(): mousePos(0.0,0.0) { orientation = /*identity3D()*/ rotation3D(vec3(-3, 2, 1), 50); };
	int w, h; // width and height
	vec2 mousePos;
    mat4 orientation;
};

//****************************************************
// Global Variables
//****************************************************
Viewport viewport;
UCB::ImageSaver * imgSaver;
int frameCount = 0;
Sweep *sweep;
Mesh *mesh;
Shader *shade;
double distanceBack = 20;
GLuint skyboxNorth,
       skyboxSouth,
       skyboxEast,
       skyboxWest,
       skyboxUp,
       skyboxDown;
GLuint skyboxDLid;
GLint inverseOrientation;
bool startAnimating = false;
bool animating = false;
bool lastFrame = false;
int animationFrames = 180;
bool walking = false;
double walkX = 0.0;
double walkY = 0.0;

// A simple helper function to load a mat4 into opengl
void applyMat4(mat4 &mat) {
    double glmat[16];
    int k = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            glmat[k++] = mat[i][j];
        }
    }
    glMultMatrixd(glmat);
}

// A simple helper function to load a mat4 into opengl
void flattenMat4(mat4 &mat, float *glmat) {
    //float glmat[16];
    int k = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            glmat[k++] = mat[i][j];
        }
    }
    //return glmat;
}

void renderSkybox() {
	// North
	glBindTexture(GL_TEXTURE_2D, skyboxNorth);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-10, -10, -10);
	glTexCoord2f(1, 0);
	glVertex3f(10, -10, -10);
	glTexCoord2f(1, 1);
	glVertex3f(10, 10, -10);
	glTexCoord2f(0, 1);
	glVertex3f(-10, 10, -10);
	glEnd();
	// South
	glBindTexture(GL_TEXTURE_2D, skyboxSouth);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(10, -10, 10);
	glTexCoord2f(1, 0);
	glVertex3f(-10, -10, 10);
	glTexCoord2f(1, 1);
	glVertex3f(-10, 10, 10);
	glTexCoord2f(0, 1);
	glVertex3f(10, 10, 10);
	glEnd();
	// East
	glBindTexture(GL_TEXTURE_2D, skyboxEast);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(10, -10, -10);
	glTexCoord2f(1, 0);
	glVertex3f(10, -10, 10);
	glTexCoord2f(1, 1);
	glVertex3f(10, 10, 10);
	glTexCoord2f(0, 1);
	glVertex3f(10, 10, -10);
	glEnd();
	// West
	glBindTexture(GL_TEXTURE_2D, skyboxWest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(-10, -10, 10);
	glTexCoord2f(1, 0);
	glVertex3f(-10, -10, -10);
	glTexCoord2f(1, 1);
	glVertex3f(-10, 10, -10);
	glTexCoord2f(0, 1);
	glVertex3f(-10, 10, 10);
	glEnd();
	// Up
	glBindTexture(GL_TEXTURE_2D, skyboxUp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(10, 10, 10);
	glTexCoord2f(1, 0);
	glVertex3f(-10, 10, 10);
	glTexCoord2f(1, 1);
	glVertex3f(-10, 10, -10);
	glTexCoord2f(0, 1);
	glVertex3f(10, 10, -10);
	glEnd();
	// Down
	glBindTexture(GL_TEXTURE_2D, skyboxDown);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(10, -10, -10);
	glTexCoord2f(1, 0);
	glVertex3f(-10, -10, -10);
	glTexCoord2f(1, 1);
	glVertex3f(-10, -10, 10);
	glTexCoord2f(0, 1);
	glVertex3f(10, -10, 10);
	glEnd();

	

    // load textures
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, skyboxNorth);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, skyboxSouth);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, skyboxEast);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, skyboxWest);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, skyboxUp);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, skyboxDown);
}

bool loadSkybox() {
	bool success = loadTexture("skybox/north.bmp", skyboxNorth) &&
				   loadTexture("skybox/south.bmp", skyboxSouth) &&
				   loadTexture("skybox/east.bmp", skyboxEast) &&
				   loadTexture("skybox/west.bmp", skyboxWest) &&
				   loadTexture("skybox/up.bmp", skyboxUp) &&
				   loadTexture("skybox/down.bmp", skyboxDown);
	if (!success) {
		return false;
	}
	
    skyboxDLid = glGenLists(1);
    glNewList(skyboxDLid, GL_COMPILE);
	renderSkybox();
    glEndList();

	return true;
}


//-------------------------------------------------------------------------------
/// You will be calling all of your drawing-related code from this function.
/// Nowhere else in your code should you use glBegin(...) and glEnd() except code
/// called from this method.
///
/// To force a redraw of the screen (eg. after mouse events or the like) simply call
/// glutPostRedisplay();
void display() {
	//Clear Buffers
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
	glLoadIdentity();							// make sure transformation is "zero'd"

	


	mat4 viewDirMat = identity3D();
	vec3 offset(0, 0, -distanceBack);

	if (walking) {
		sweep->sampleWalk(walkX, walkY, offset, viewDirMat);
		//offset[1] += 2;
	}



	
	// Disable the depth buffer for the skybox
	glDisable(GL_DEPTH_TEST);
    //glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	shade->set(true);
	glPushMatrix();
	
    //glTranslatef(0,0,-2*distanceBack);
    applyMat4(viewport.orientation.transpose());
	applyMat4(viewDirMat);
	glCallList(skyboxDLid);

	glPopMatrix();


	// Re-enable the depth buffer
	glEnable(GL_DEPTH_TEST);
	shade->set();
    //glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);


	
	
	//glTranslatef(offset[0], offset[1], offset[2]);
	applyMat4(translation3D(offset));
	//glTranslatef(-offset[0], -offset[1], -offset[2]);


	applyMat4(viewport.orientation.transpose());
	applyMat4(viewDirMat);
	//glTranslatef(0, -0.5, 0);
	//mat3 blah = identity2D() * inverseOrientation;
	float inv[16];
	flattenMat4(viewDirMat.inverse(), inv);
	glUniformMatrix4fv(inverseOrientation, 16, GL_FALSE, inv);
	// Render the sweep
    sweep->renderWithDisplayList(*shade,20,.3,10);
    
    // feel free to remove the mesh; it's mainly there as a demo
    //mesh->draw(*shade);

	//Now that we've drawn on the buffer, swap the drawing buffer and the displaying buffer.
	glutSwapBuffers();

}


//-------------------------------------------------------------------------------
/// \brief	Called when the screen gets resized.
/// This gives you the opportunity to set up all the relevant transforms.
///
void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, ((double)w / MAX(h, 1)), 1.0, 100.0);
	//glOrtho(-10,10,-10,10,1,100);

    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//-------------------------------------------------------------------------------
/// Called to handle keyboard events.
void myKeyboardFunc (unsigned char key, int x, int y) {
	switch (key) {
		case 27:			// Escape key
			exit(0);
			break;
        case 'S':
        case 's':
    	    imgSaver->saveFrame();
            break;
        case 'T':
        case 't':
            shade->toggleTextureMap();
            sweep->clearDisplayList();
            break;
        case 'B':
        case 'b':
            shade->toggleBumpMap();
            sweep->clearDisplayList();
            break;
        case 'P':
        case 'p':
            shade->togglePhong();
            sweep->clearDisplayList();
            break;
        case 'D':
        case 'd':
            shade->toggleDisplacement();
            sweep->clearDisplayList();
            break;
        case 'E':
        case 'e':
            shade->toggleEnvironment();
            sweep->clearDisplayList();
            break;
        case 'A':
        case 'a':
            startAnimating = !startAnimating;
            break;
        case 'W':
        case 'w':
            walking = !walking;
            break;
	}
}
///////////////////////////////////////////
// special func
void mySpecialFunc(int key, int x, int y) {
	switch (key) {
	// Modify local azimuth
	case GLUT_KEY_UP:
		if (walking) {
			walkX += 0.0005;
		}
		break;
	case GLUT_KEY_DOWN:
		if (walking) {
			walkX -= 0.0005;
		}
		break;
	case GLUT_KEY_LEFT:
		if (walking) {
			walkY += 0.05;
		}
		break;
	case GLUT_KEY_RIGHT:
		if (walking) {
			walkY -= 0.05;
		}
		break;
	}
}

//-------------------------------------------------------------------------------
/// Called whenever the mouse moves while a button is pressed
void myActiveMotionFunc(int x, int y) {

    // Rotate viewport orientation proportional to mouse motion
    vec2 newMouse = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
    vec2 diff = (newMouse - viewport.mousePos);
    double len = diff.length();
    if (len > .001) {
        vec3 axis = vec3(diff[1]/len, diff[0]/len, 0);
        viewport.orientation = viewport.orientation * rotation3D(axis, -180 * len);
    }

	//cout << viewport.orientation << endl;

    //Record the mouse location for drawing crosshairs
    viewport.mousePos = newMouse;

    //Force a redraw of the window.
    glutPostRedisplay();
}


//-------------------------------------------------------------------------------
/// Called whenever the mouse moves without any buttons pressed.
void myPassiveMotionFunc(int x, int y) {
    //Record the mouse location for drawing crosshairs
    viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

    //Force a redraw of the window.
    glutPostRedisplay();
}

//-------------------------------------------------------------------------------
/// Called to update the screen at 30 fps.
void frameTimer(int value){

    frameCount++;
    glutPostRedisplay();
	if (startAnimating) {
		startAnimating = frameCount % (2 * animationFrames) != 0;
		animating = !startAnimating;
	}
	if (lastFrame) {
    	imgSaver->saveFrame();
		lastFrame = false;
	}
	if (animating) {
		int f = frameCount % (2 * animationFrames);
		if (f < animationFrames) {
			viewport.orientation = viewport.orientation * rotation3D(vec3(-1, 1, 0), 360.0 / animationFrames);
		}
		else {
			viewport.orientation = viewport.orientation * rotation3D(vec3(1, 1, 0), 360.0 / animationFrames);
		}
		if (f != 0) {
	    	imgSaver->saveFrame();
		}
		animating = f != 2 * animationFrames - 1;
		lastFrame = !animating;
	}
    glutTimerFunc(1000/30, frameTimer, 1);
}

void exitFreeimage(void) {
    FreeImage_DeInitialise();
}


//-------------------------------------------------------------------------------
/// Initialize the environment
int main(int argc,char** argv) {
	//Initialize OpenGL
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);

    FreeImage_Initialise();
    atexit(exitFreeimage);

	//Set up global variables
	viewport.w = 800;
	viewport.h = 600;

	if (argc < 2) {
	    cout << "USAGE: sweep sweep.trk" << endl;
	    return -1;
    }

	//Initialize the screen capture class to save BMP captures
	//in the current directory, with the prefix "sweep"
	imgSaver = new UCB::ImageSaver("./", "sweep");

	//Create OpenGL Window
	glutInitWindowSize(viewport.w,viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow("CS184 Framework");

	//Register event handlers with OpenGL.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(myKeyboardFunc);
	glutSpecialFunc(mySpecialFunc);
	glutMotionFunc(myActiveMotionFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);
    frameTimer(0);

    // set some lights
    {
       float ambient[4] = { .1f, .1f, .1f, 1.f };
       float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
       float pos[4] = { 0, 1, 1, 0 };
       glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
       glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
       glLightfv(GL_LIGHT0, GL_POSITION, pos);
       glEnable(GL_LIGHT0);
    }
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glewInit();
	if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
		printf("Ready for GLSL\n");
	else {
		printf("No GLSL support\n");
		exit(1);
	}

    shade = new Shader("sweepShader.vert", "sweepShader.frag");
  
    // Create the coaster
    sweep = new Sweep(argv[1]);
    if (sweep->bad()) {
        cout << "Sweep file appears to not have a proper sweep in it" << endl;
        return -1;
    }

	// Load the skybox textures
	if (!loadSkybox()) {
        cout << "Skybox did not load correctly" << endl;
		return -1;
	}

	/*
    mesh = new Mesh();
    mesh->loadFile("cyl_textured.obj");
    mesh->loadTextures("cylinder_tex.bmp", "cylinder_tex_bump.png");
    mesh->centerAndScale(3);
    // do two steps of subdivision by swapping through a temp mesh
    Mesh temp;// mesh->subdivide(temp); temp.subdivide(*mesh);
    // recompute vertex normals
    mesh->computeVertexNormals();
	*/


	//And Go!
	glutMainLoop();
}
