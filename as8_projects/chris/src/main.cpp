#include "main.h"
#include "GL/gl.h"

using namespace std;


//****************************************************
// Some Classes
//****************************************************
class Viewport {
public:
    Viewport(): mousePos(0.0,0.0) { orientation = identity3D(); };
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

// A simple helper function to load a mat4 into opengl
void applyMat4(mat4 &mat) {
    double glmat[16];
    int k = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            glmat[k++] = mat[j][i];
        }
    }
    glMultMatrixd(glmat);
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


    glTranslatef(0,0,-20);
    applyMat4(viewport.orientation);

    
    // feel free to remove the mesh; it's mainly there as a demo
	//glCullFace(GL_BACK);
    //mesh->draw(*shade);

	glCullFace(GL_FRONT);	
    sweep->renderWithDisplayList(*shade,40,.3,40);


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
		case 'Q':
        case 'q':
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
		case 'W':
		case 'w':
			shade->toggleShader();
			sweep->clearDisplayList();
			break;
		case 'H':
		case 'h':
			shade->toggleToonShading();
			sweep->clearDisplayList();
			break;
	}
	shade->set();
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
	viewport.w = 600;
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
	glutMotionFunc(myActiveMotionFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);
    frameTimer(0);

    // set some lights
    {
       float ambient[4] = { .1f, .1f, .1f, 1.f };
       float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
       float pos[4] = { 0, 0, 1, 0 };
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

    if (!GLEW_VERSION_2_0) {
        printf("WARNING: Technically, OpenGL 2.0 is required\n");
    }

    shade = new Shader("sweepShader.vert", "sweepShader.frag");
  
    // Create the coaster
    sweep = new Sweep(argv[1]);
    if (sweep->bad()) {
        cout << "Sweep file appears to not have a proper sweep in it" << endl;
        return -1;
    }

    mesh = new Mesh();
    mesh->loadFile("cyl_textured.obj");
    mesh->loadTextures("cylinder_tex.bmp", "cylinder_tex_bump.png");
    mesh->centerAndScale(3);
    // do two steps of subdivision by swapping through a temp mesh
    Mesh temp; mesh->subdivide(temp); temp.subdivide(*mesh);
    // recompute vertex normals
    mesh->computeVertexNormals();
	glEnable(GL_CULL_FACE);

	glutMainLoop();
}

