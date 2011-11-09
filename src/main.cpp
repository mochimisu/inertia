#include "gameengine.h"

#pragma GCC diagnostic ignored "-Wwrite-strings"

//Game Engine
GameEngine * game_engine;

/*
 * Main
 */
int main(int argc,char** argv) {
  //Initialize OpenGL/GLUT
  glutInit(&argc, argv);

	//initialize game engine
	game_engine = new GameEngine();
	game_engine->Init();

	//And Go!
	game_engine->Go();
}

