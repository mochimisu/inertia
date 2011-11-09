#include "main.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

//Sound Engine
//SoundEngine * soundengine;

//Render Engine
//RenderEngine * renderengine;

//Game Engine
GameEngine * game_engine;

//==OBJECTS
Sweep *sweep;
Vehicle *vehicle;
Mesh * vehMesh;

//==USER INTERACTION/GAMEPLAY
int lastTimeStep;
int lapStartTime;
int gameMode;
enum { MODE_RACE, MODE_TRACK_SELECT, MODE_TITLE , MODE_DEATH};
int lastStartPress = 0;

void setMode(int mode);

//===WINDOW PROPERTIES
Viewport viewport;


/*
 * Main
 */
int main(int argc,char** argv) {
	/*unsigned int seed = time(NULL);
		cout << seed << endl;
		srand(seed);*/
	srand(time(NULL));

	//Initialize OpenGL
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	viewport.w = kRenderWidth;
	viewport.h = kRenderHeight;
	glutInitWindowSize(viewport.w,viewport.h);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Inertia Alpha");

	//initialize game engine
	game_engine = new GameEngine();
	game_engine->Init();


	game_engine->Go();

	//And Go!
	glutMainLoop();
}

