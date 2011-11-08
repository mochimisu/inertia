#include "main.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

//Sound Engine
SoundEngine * soundengine;

//Render Engine
RenderEngine * renderengine;

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


FTGLTextureFont * evolutionFont;
FTGLTextureFont * digitalNinjaFont;
FTGLTextureFont * accidentalPresidencyFont;
FTBufferFont * evolutionBufferFont;
FTBufferFont * accidentalPresidencyBufferFont;


//===WINDOW PROPERTIES
Viewport viewport;


/*
 * Race Render Scene
 */

namespace raceScene {
		void drawHud() {
				//fudging this...
				const float maxVelocityWidth = renderWidth * 2.5/8 /20;

				glEnable (GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glUseProgramObjectARB(0);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//==Actual HUD stuff
				std::ostringstream buff;
				glTranslated(0,0,-5);

				//backdrops 

				//header
				glColor4f(.250980392,.458823529,.631372549,0.95);
				glBegin(GL_QUADS);
				glVertex3f(-renderWidth/2,renderHeight * 3.8/8,0);
				glVertex3f(renderWidth/2,renderHeight * 3.8/8,0);
				glVertex3f(renderWidth/2,renderHeight/2,0);
				glVertex3f(-renderWidth/2,renderHeight/2,0);

				glColor4f(.250980392,.458823529,.631372549,0.1);

				//Lap
				glVertex3f(-renderWidth/2,renderHeight * 3.0/8,0);
				glVertex3f(-renderWidth * 3.4/8,renderHeight * 3.0/8,0);
				glVertex3f(-renderWidth * 3.4/8,renderHeight * 3.8/8,0);
				glVertex3f(-renderWidth/2,renderHeight * 3.8/8,0);

				//Record (with record to have less branches)
				/*
					 glBegin(GL_QUADS);
					 glVertex3f(renderWidth * 2.2/8,renderHeight * 3.0/8,0);
					 glVertex3f(renderWidth/2,renderHeight * 3.0/8,0);
					 glVertex3f(renderWidth/2,renderHeight * 3.8/8,0);
					 glVertex3f(renderWidth * 2.2/8,renderHeight * 3.8/8,0);
					 glEnd(); */


				//Energy
				glVertex3f(-65,renderHeight * 3.0/8,0);
				glVertex3f(65,renderHeight * 3.0/8,0);
				glVertex3f(65,renderHeight * 3.8/8,0);
				glVertex3f(-65,renderHeight * 3.8/8,0);

				//Lap Time
				glVertex3f(-renderWidth/2, -renderHeight*4.0/8,0);
				glVertex3f(-renderWidth*2.5/8, -renderHeight*4.0/8,0);
				glVertex3f(-renderWidth*2.5/8, -renderHeight*3.0/8,0);
				glVertex3f(-renderWidth/2, -renderHeight*3.0/8,0);

				//Velocity
				glVertex3f(renderWidth*2/8, -renderHeight*4.0/8,0);
				glVertex3f(renderWidth/2, -renderHeight*4.0/8,0);
				glVertex3f(renderWidth/2, -renderHeight*3.0/8,0);
				glVertex3f(renderWidth*2/8, -renderHeight*3.0/8,0);
				glEnd();

				//Actual stuff

				//Velocity
				buff.str("");
				buff << "Velocity";
				glColor4f(.188235294,.474509804,1,0.9);
				renderengine->drawString(evolutionBufferFont, buff.str(),renderWidth*2.3/8,-renderHeight*3.4/8 + 5); 

				//Velocity Bar
				glColor4f(.188235294,.474509804,1,0.5);
				glBegin(GL_QUADS);
				glVertex3f(renderWidth*2.1/8,-renderHeight*3.8/8,0); // bottom left
				glVertex3f(renderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.8/8,0); //bottom right
				glVertex3f(renderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.4/8,0); //top right
				glVertex3f(renderWidth*2.1/8,-renderHeight*3.4/8,0); //top left
				glEnd();

				//Velocity Text
				buff.str("");
				double velocity = vehicle->getVelocityScalar();
				buff << setfill('0') << setw(3) << int(velocity * 10) << ".";
				buff << setfill('0') << setw(3) << (int(velocity * 10000) % 1000);
				glColor4f(.9,.9,1,0.8);
				renderengine->drawString(digitalNinjaFont, buff.str(),renderWidth*2.3/8,-renderHeight*3.3/8 - 44); 

				//Air Break Bar
				if(vehicle->isAirBrake()) {

						glColor4f(.901960784,.160784314,.160784314,0.5);  

						glBegin(GL_QUADS);
						glVertex3f(renderWidth*2.1/8,-renderHeight*3.9/8,0); // bottom left
						glVertex3f(renderWidth*3.8/8 ,-renderHeight*3.9/8,0); //bottom right
						glVertex3f(renderWidth*3.8/8,-renderHeight*3.8/8,0); //top right
						glVertex3f(renderWidth*2.1/8,-renderHeight*3.8/8,0); //top left
						glEnd();
				} 

				//Lap Time
				buff.str("");
				buff << "Lap Time";
				glColor4f(1,1,1,0.75);
				renderengine->drawString(evolutionBufferFont, buff.str(),-renderWidth*3.8/8,-renderHeight*3.4/8); 

				buff.str("");
				int msTime = glutGet(GLUT_ELAPSED_TIME) - vehicle->getLapStartTime();
				int sTime = msTime/1000;
				int mTime  = sTime/60;
				buff << setfill('0') << setw(2) << mTime << ".";
				buff << setfill('0') << setw(2) << (sTime%60) << ".";
				buff << setfill('0') << setw(3) << (msTime%1000);
				glColor4f(1,1,1,0.75);
				renderengine->drawString(digitalNinjaFont, buff.str(),-renderWidth*3.8/8,-renderHeight*3.7/8); 

				//Lap Number
				buff.str("");
				buff << "Lap"; // << vehicle->getLap();
				glColor4f(1,1,1,0.75);
				renderengine->drawString(evolutionBufferFont, buff.str(),-renderWidth*3.9/8,renderHeight*3.5/8); 

				buff.str("");
				buff << vehicle->getLap();
				renderengine->drawString(digitalNinjaFont, buff.str(), -renderWidth * 3.9/8, renderHeight*3.2/8);

				//Record
				buff.str("");
				msTime = vehicle->getBestLapTime();
				if(msTime != -1) {
						//backdrop
						glColor4f(.250980392,.458823529,.631372549,0.3);
						glBegin(GL_QUADS);
						glVertex3f(renderWidth * 2.2/8,renderHeight * 3.0/8,0);
						glVertex3f(renderWidth*2/2,renderHeight * 3.0/8,0);
						glVertex3f(renderWidth*2/2,renderHeight * 3.8/8,0);
						glVertex3f(renderWidth * 2.2/8,renderHeight * 3.8/8,0);
						glEnd();

						sTime = msTime/1000;
						mTime  = sTime/60;
						buff << "Lap Record:";
						renderengine->drawString(evolutionBufferFont, buff.str(),renderWidth*2.4/8, renderHeight*3.5/8);

						buff.str("");
						buff << setfill('0') << setw(2) << mTime << ".";
						buff << setfill('0') << setw(2) << (sTime%60) << ".";
						buff << setfill('0') << setw(3) << (msTime%1000);
						glColor4f(1,1,1,0.75);
						renderengine->drawString(digitalNinjaFont, buff.str(),renderWidth*2.4/8, renderHeight*3.2/8);

				}

				//Name 
				buff.str("");
				buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
				renderengine->drawString(accidentalPresidencyBufferFont, buff.str(), -renderWidth*3.9/8, renderHeight*3.85/8);

				//Energy
				buff.str("");
				buff << "Energy: ";
				//buff << vehicle->getEnergy();
				renderengine->drawString(evolutionBufferFont, buff.str(), -55,renderHeight*3.5/8);

				buff.str("");
				double energy = vehicle->getEnergy();
				buff << int(energy) << ".";
				buff  << setfill('0') << setw(3) << (int(1000 * energy) % 1000);
				renderengine->drawString(digitalNinjaFont, buff.str(), -45,renderHeight*3.2/8);


				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
		}

		void drawObjects(GeometryShader * curShade) {
				//Track/City
				sweep->renderWithDisplayList(*curShade,50, true, 0.3,20);

				//Vehicle Location
				vec3 vehLoc = vehicle->worldSpacePos();

				//Put matrices together for smaller stack size
				mat4 transformation = vehicle->orientationBasis() *
						translation3D(vehLoc).transpose();

				pushMat4(transformation);
				vehicle->draw(curShade); 
				popTransform();
		}

		void processNormalKeys(unsigned char key, int x, int y) {
				switch (key) {
						case 'Q':
						case 'q':
						case 27:	
								soundengine->playSelectSource2();
								setMode(MODE_TITLE);
								break;
						case 'A':
						case 'a':
								renderengine->toggleDeathScatter();
								break;
						case ' ':
								vehicle->setAirBrake(0.00008);
								break;
				}
		}

		void processNormalKeysUp(unsigned char key, int x, int y) {
				switch(key) {
						case ' ':
								vehicle->setAirBrake(0);
								break;
				}
		}

		void processSpecialKeys(int key, int x, int y) {
				switch(key) {
						case GLUT_KEY_UP:
								soundengine->playEngineSound();
								vehicle->setAccel(0.2);
								break;
						case GLUT_KEY_DOWN:
								soundengine->playEngineSound();
								vehicle->setAccel(-0.1);
								break;
						case GLUT_KEY_LEFT:
								vehicle->turnLeft(3);
								break;
						case GLUT_KEY_RIGHT:
								vehicle->turnRight(3);
								break;
				}
		}

		void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
				//cout << (buttonMask) << endl;
				//cout << (buttonMask & 16384) << endl;
				if(buttonMask & 16384 || buttonMask & 1) { //button 14: X on DualShock3, button 1 is trigger on Chris' joystick
						vehicle->setAccel(0.2);
						soundengine->playEngineSound();
				} else if(buttonMask & 8192) { //button 13: O on DualShock3
						vehicle->setAccel(-0.1);
						soundengine->playEngineSound(); 
				} else {
						soundengine->stopEngineSound();
						vehicle->setAccel(0.0);
				}
				if(buttonMask & 256 && buttonMask & 512) { //256:L2, 512: R2
						vehicle->setAirBrake(0.0001);
				} else if(buttonMask & 256) { //TODO: left and right airbrake
						vehicle->setAirBrake(0.00005);
				} else if(buttonMask & 512) {
						vehicle->setAirBrake(0.00005);
				} else {
						vehicle->setAirBrake(0.0);
				}
				//cout << x << endl;
				vehicle->turnRight(x/500);

				if(buttonMask & 8 && (glutGet(GLUT_ELAPSED_TIME) - lastStartPress > 1000)){ //start button
						lastStartPress = glutGet(GLUT_ELAPSED_TIME);
						setMode(MODE_TITLE);
				}
		}


		void processSpecialKeysUp(int key, int x, int y) {
				switch(key) {
						case GLUT_KEY_UP:
						case GLUT_KEY_DOWN:
								soundengine->stopEngineSound();
								vehicle->setAccel(0.0);
								break;
						case GLUT_KEY_LEFT:
						case GLUT_KEY_RIGHT:
								vehicle->turnLeft(0.0);
								break;
				}
		}


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

		void myPassiveMotionFunc(int x, int y) {
				//Record the mouse location for drawing crosshairs
				viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

				//Force a redraw of the window.
				glutPostRedisplay();
		}

}







//======TITLE SCENE
//sorry about the code mess but its glut's fault and we dont have enough time to make something complex to make it look nice
namespace titleScene {

		void drawObjects(GeometryShader * curShade) {

				vec3 rotAxis(0.3,0.9,0);

				mat4 transformation = rotation3D(rotAxis, glutGet(GLUT_ELAPSED_TIME)/200.0);
				pushMat4(transformation);
				vehMesh->draw(*curShade); 
				popTransform();


		}
		void drawTitleOverlay() {
				//fudging this...
				//const float maxVelocityWidth = renderWidth * 2.5/8 /20;

				glEnable (GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glUseProgramObjectARB(0);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//==Actual HUD stuff
				std::ostringstream buff;
				glTranslated(0,0,-5);

				glColor4f(1,1,1,0.75);
				glBegin(GL_QUADS);
				glTexCoord2d(0,0);glVertex3f(-20,-renderHeight*3/24,0);
				glTexCoord2d(1,0);glVertex3f(renderWidth*0.45,-renderHeight*3/24,0);
				glTexCoord2d(1,1);glVertex3f(renderWidth*0.45,renderHeight/6,0);
				glTexCoord2d(0,1);glVertex3f(-20,renderHeight/6,0);


				glTexCoord2d(0,0);glVertex3f(-renderWidth/2,-renderHeight/2,0);
				glTexCoord2d(1,0);glVertex3f(-renderWidth*3/16,-renderHeight/2,0);
				glTexCoord2d(1,1);glVertex3f(-renderWidth*3/16,-renderHeight*5/16,0);
				glTexCoord2d(0,1);glVertex3f(-renderWidth/2,-renderHeight*5/16,0);
				glEnd();

				buff.str("");
				buff << "INERTIA";
				glColor4f(.188235294,.474509804,1,0.9);


				evolutionFont->FaceSize(150);
				renderengine->drawString(evolutionFont, buff.str(),0,0); 
				evolutionFont->FaceSize(36);

				accidentalPresidencyFont->FaceSize(36);
				buff.str("");
				buff << "CS184Sp11 Final Project";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),0,-50); 
				accidentalPresidencyFont->FaceSize(20);
				buff.str("");
				buff << "Brandon Wang, Andrew Lee, Chris Tandiono";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),0,-75); 

				if((glutGet(GLUT_ELAPSED_TIME)/500)%2) {
						glColor4f(1,1,1,0.75);
						glBegin(GL_QUADS);
						glTexCoord2d(0,0);glVertex3f(-renderWidth*0.075,-renderHeight*3/8 - renderHeight/64,0);
						glTexCoord2d(1,0);glVertex3f(renderWidth*0.075,-renderHeight*3/8 - renderHeight/64,0);
						glTexCoord2d(1,1);glVertex3f(renderWidth*0.075,-renderHeight*3/8 + renderHeight/64,0);
						glTexCoord2d(0,1);glVertex3f(-renderWidth*0.075,-renderHeight*3/8 + renderHeight/64,0);
						glEnd();

						glColor4f(.188235294,.474509804,1,0.9);
						buff.str("");
						buff << "Press Space or Start";
						renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth*0.065,-renderHeight*3/8 - 5); 
				}

				buff.str("");
				buff << "Controls";
				renderengine->drawString(evolutionFont, buff.str(),-renderWidth/2 + 10, -renderHeight*2.85/8); 

				buff.str("");
				buff << "Left and Right Arrow/Analog Stick: Turn";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth/2 + 10, -renderHeight*3.05/8);

				buff.str("");
				buff << "Up Arrow/X Button: Accelerate";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth/2 + 10, -renderHeight*3.25/8);

				buff.str("");
				buff << "Down Arrow/O Button: Decelerate";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth/2 + 10, -renderHeight*3.45/8);  

				buff.str("");
				buff << "Space/L2 or R2: Air Brake";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth/2 + 10, -renderHeight*3.65/8);  

				buff.str("");
				buff << "Esc/Start: Quit";
				renderengine->drawString(accidentalPresidencyFont, buff.str(),-renderWidth/2 + 10, -renderHeight*3.85/8);  






				//Name 
				buff.str("");
				buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
				renderengine->drawString(accidentalPresidencyBufferFont, buff.str(), -renderWidth*3.9/8, renderHeight*3.85/8);

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
		}

		void processNormalKeys(unsigned char key, int x, int y) {

				switch (key) {
						case 'Q':
						case 'q':
						case 27:	
								exit(0);
								break;
						case 'A':
						case 'a':
								renderengine->toggleDeathScatter();
								break;
								//temp keys for debugging
						case ' ':
								soundengine->playSelectSource1();
								setMode(MODE_TRACK_SELECT);
								break;
				}
		}

		void processNormalKeysUp(unsigned char key, int x, int y) {
				switch(key) {
						case ' ':
								vehicle->setAirBrake(0);
								break;
				}
		}

		void processSpecialKeys(int key, int x, int y) {
				return;
		}

		void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
				//cout << (buttonMask) << endl;
				//cout << (buttonMask & 16384) << endl;
				if(buttonMask & 8 && (glutGet(GLUT_ELAPSED_TIME) - lastStartPress > 1000)) { //start button
						lastStartPress = glutGet(GLUT_ELAPSED_TIME);
						setMode(MODE_RACE);
				}

				//		cout << buttonMask << endl;
		}


		void processSpecialKeysUp(int key, int x, int y) {
				switch(key) {
						case GLUT_KEY_UP:
						case GLUT_KEY_DOWN:
								break;
						case GLUT_KEY_LEFT:
						case GLUT_KEY_RIGHT:
								break;
				}
		}


		void activeMotionFunc(int x, int y) {

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

		void passiveMotionFunc(int x, int y) {
				//Record the mouse location for drawing crosshairs
				viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

				//Force a redraw of the window.
				glutPostRedisplay();
		}
}






//======TRACK SELECTION SCREEN
////sorry about the code mess but its glut's fault and we dont have enough time to make something complex to make it look nice
namespace trackSelectScene {

		void drawObjects(GeometryShader * curShade) {

				vec3 rotAxis(0.3,0.9,0);

				mat4 transformation = rotation3D(rotAxis, glutGet(GLUT_ELAPSED_TIME)/200.0);
				pushMat4(transformation);
				//sweep->renderWithDisplayList(*curShade,50,0.3,20);
				popTransform();

				pushMat4(scaling3D(vec3(0.1,0.1,0.1)).transpose() * transformation * translation3D(vec3(0,-3,0)).transpose());
				sweep->renderWithDisplayList(*curShade, 50, false, 0.3, 20);
				popTransform();
		}
		void drawTrackOverlay() {
				//fudging this...
				//const float maxVelocityWidth = renderWidth * 2.5/8 /20;
				glEnable (GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glUseProgramObjectARB(0);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				//==Actual HUD stuff
				std::ostringstream buff;
				glTranslated(0,0,-5);

				glColor4f(1,1,1,0.75);

				glBegin(GL_QUADS);
				glTexCoord2d(0,0);glVertex3f(-20,-renderHeight/2,0);
				glTexCoord2d(1,0);glVertex3f(renderWidth*0.45,-renderHeight/2,0);
				glTexCoord2d(1,1);glVertex3f(renderWidth*0.45,-renderHeight*25/64,0);
				glTexCoord2d(0,1);glVertex3f(-20,-renderHeight*25/64,0);
				glEnd();

				glColor4f(.188235294,.474509804,1,0.9);
				accidentalPresidencyFont->FaceSize(36);
				buff.str("");
				buff << "Press G/X to generate a new track";
				renderengine->drawString(accidentalPresidencyFont, buff.str(), 0, -renderHeight * 28/64);

				buff.str("");
				buff << "Press Space/Start to start!";
				renderengine->drawString(accidentalPresidencyFont, buff.str(), 0, -renderHeight * 31/64);

				//Name 
				buff.str("");
				buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
				renderengine->drawString(accidentalPresidencyBufferFont, buff.str(), -renderWidth*3.9/8, renderHeight*3.85/8);

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
		}

		void generateNewTrack() {
				Sweep * oldSweep = sweep;
				Vehicle * oldVehicle = vehicle;
				sweep = new Sweep();
				vehicle = new Vehicle(sweep, vehMesh);
				delete oldSweep;
				delete oldVehicle;

		}

		void processNormalKeys(unsigned char key, int x, int y) {
				switch (key) {
						case 'Q':
						case 'q':
						case 27:	
								soundengine->playSelectSource2();
								setMode(MODE_TITLE);
								break;
						case 'A':
						case 'a':
								renderengine->toggleDeathScatter();
								break;
						case 'G':
						case 'g':
								soundengine->playSelectSource2();
								generateNewTrack();
								break;
								//temp keys for debugging
						case ' ':
								soundengine->playSelectSource1();
								setMode(MODE_RACE);
								break;
				}
		}

		void processNormalKeysUp(unsigned char key, int x, int y) {
				switch(key) {
						case ' ':
								break;
				}
		}

		void processSpecialKeys(int key, int x, int y) {
				return;
		}

		void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
				//cout << (buttonMask) << endl;
				//cout << (buttonMask & 16384) << endl;
				if(lastStartPress > 1000) {
						if(buttonMask & 16384) {
								lastStartPress = glutGet(GLUT_ELAPSED_TIME);
								generateNewTrack();
						}
						if(buttonMask & 8) { //start button
								lastStartPress = glutGet(GLUT_ELAPSED_TIME);
								setMode(MODE_RACE);
						}

				}  
				//		cout << buttonMask << endl;
		}


		void processSpecialKeysUp(int key, int x, int y) {
				switch(key) {
						case GLUT_KEY_UP:
						case GLUT_KEY_DOWN:
								break;
						case GLUT_KEY_LEFT:
						case GLUT_KEY_RIGHT:
								break;
				}
		}


		void activeMotionFunc(int x, int y) {

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

		void passiveMotionFunc(int x, int y) {
				//Record the mouse location for drawing crosshairs
				viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

				//Force a redraw of the window.
				glutPostRedisplay();
		}
}

namespace deathScene {
		//just displays a YOU DIED message, everything else the same as title scene
		void drawDeathOverlay() {
				titleScene::drawTitleOverlay();



				glEnable (GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glUseProgramObjectARB(0);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				std::ostringstream buff;
				glTranslated(0,0,-5);


				glColor4f(1,1,1,0.75);
				glBegin(GL_QUADS);
				glTexCoord2d(0,0);glVertex3f(-renderWidth*0.45,-renderHeight*3/24,0);
				glTexCoord2d(1,0);glVertex3f(-20,-renderHeight*3/24,0);
				glTexCoord2d(1,1);glVertex3f(-20,renderHeight/6,0);
				glTexCoord2d(0,1);glVertex3f(-renderWidth*0.45,renderHeight/6,0);
				glEnd();


				buff.str("");
				buff << "YOU DIED!";
				glColor4f(1,0,0,0.9);


				evolutionFont->FaceSize(100);
				renderengine->drawString(evolutionFont, buff.str(),-renderWidth*0.45+20,0); 
				evolutionFont->FaceSize(36);

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);

		}
}


//Steps a vehicle 
//because of glut's callback limitation in the fact that we can't interrupt, we have to keep polling if we're in the right mode........
void stepVehicle(int x) {
		//call at beginning for consistency..
		int newTime =glutGet(GLUT_ELAPSED_TIME);
		if(gameMode == MODE_RACE) {
				int timeDif = newTime - lastTimeStep;
				vehicle->step(0.01 * timeDif/10.0);

				renderengine->p_camera = vehicle->cameraPos();
				renderengine->l_camera = vehicle->cameraLookAt();
				renderengine->u_camera = vehicle->getUp();

				//p_light = vehicle->lightPos();
				renderengine->l_light = vehicle->worldSpacePos();

				if(vehicle->getEnergy() < 0.0000001) {
						soundengine->playDeathSound();
						setMode(MODE_DEATH);
				}
		}
		//redo this every 15ms
		lastTimeStep = newTime;
		glutTimerFunc(15,stepVehicle, 0);

}

void setMode(int newMode) {
		switch(newMode) {
				case MODE_RACE:
						gameMode = MODE_RACE;
						vehMesh->centerAndScale(0.8);
						renderengine->setDrawObject(raceScene::drawObjects);
						renderengine->setDrawOverlay(raceScene::drawHud);
						glutKeyboardFunc(raceScene::processNormalKeys);
						glutKeyboardUpFunc(raceScene::processNormalKeysUp);
						glutMotionFunc(raceScene::myActiveMotionFunc);
						glutPassiveMotionFunc(raceScene::myPassiveMotionFunc);

						glutSpecialFunc(raceScene::processSpecialKeys);
						glutSpecialUpFunc(raceScene::processSpecialKeysUp);
						glutJoystickFunc(raceScene::joystickFunc,10);

						renderengine->skyColor = vec4(.764705882,.890196078,1,1);
						renderengine->p_light = vec3(110,60,0);
						renderengine->p_light_scatter = vec3(110,60,0);


						vehicle->reset();
						//Lap time 
						vehicle->setLapStartTime(glutGet(GLUT_ELAPSED_TIME));

						//Step Vehicle once (and it will recurse on timer)
						lastTimeStep = glutGet(GLUT_ELAPSED_TIME);
						stepVehicle(0);

						renderengine->setDeathScatter(false);

						break;

				case MODE_TITLE:
						gameMode = MODE_TITLE;
						soundengine->stopEngineSound();


						vehMesh->centerAndScale(40);
						renderengine->setDrawObject(titleScene::drawObjects);
						renderengine->setDrawOverlay(titleScene::drawTitleOverlay);

						glutKeyboardFunc(titleScene::processNormalKeys);
						glutKeyboardUpFunc(titleScene::processNormalKeysUp);
						glutMotionFunc(titleScene::activeMotionFunc);
						glutPassiveMotionFunc(titleScene::passiveMotionFunc);

						glutSpecialFunc(titleScene::processSpecialKeys);
						glutSpecialUpFunc(titleScene::processSpecialKeysUp);
						glutJoystickFunc(titleScene::joystickFunc,10);

						renderengine->skyColor = vec4(0,.152941176,.282352941,1);

						renderengine->p_camera = vec3(0,10,16);
						renderengine->l_camera = vec3(0,0,0);
						renderengine->u_camera = vec3(0,1,0);

						renderengine->p_light_scatter = vec3(0,-30,-40);
						renderengine->l_light = vec3(0,0,0);        
						renderengine->p_light = vec3(0,30,6);

						renderengine->setDeathScatter(false);

						break;

				case MODE_DEATH:
						gameMode = MODE_DEATH;  
						soundengine->stopEngineSound();

						vehMesh->centerAndScale(40);
						renderengine->setDrawObject(titleScene::drawObjects);
						renderengine->setDrawOverlay(deathScene::drawDeathOverlay);

						glutKeyboardFunc(titleScene::processNormalKeys);
						glutKeyboardUpFunc(titleScene::processNormalKeysUp);
						glutMotionFunc(titleScene::activeMotionFunc);
						glutPassiveMotionFunc(titleScene::passiveMotionFunc);

						glutSpecialFunc(titleScene::processSpecialKeys);
						glutSpecialUpFunc(titleScene::processSpecialKeysUp);
						glutJoystickFunc(titleScene::joystickFunc,10);

						renderengine->skyColor = vec4(0,.152941176,.282352941,1);

						renderengine->p_camera = vec3(0,10,16);
						renderengine->l_camera = vec3(0,0,0);
						renderengine->u_camera = vec3(0,1,0);

						renderengine->p_light_scatter = vec3(0,-30,-40);
						renderengine->l_light = vec3(0,0,0);        
						renderengine->p_light = vec3(0,30,6);

						renderengine->setDeathScatter(false);

						break;


				case MODE_TRACK_SELECT:
						gameMode = MODE_TRACK_SELECT;

						soundengine->stopEngineSound();

						vehMesh->centerAndScale(40);
						renderengine->setDrawObject(trackSelectScene::drawObjects);
						renderengine->setDrawOverlay(trackSelectScene::drawTrackOverlay);

						glutKeyboardFunc(trackSelectScene::processNormalKeys);
						glutKeyboardUpFunc(trackSelectScene::processNormalKeysUp);
						glutMotionFunc(trackSelectScene::activeMotionFunc);
						glutPassiveMotionFunc(trackSelectScene::passiveMotionFunc);

						glutSpecialFunc(trackSelectScene::processSpecialKeys);
						glutSpecialUpFunc(trackSelectScene::processSpecialKeysUp);
						glutJoystickFunc(trackSelectScene::joystickFunc,10);

						renderengine->skyColor = vec4(0,.152941176,.282352941,1);

						renderengine->p_camera = vec3(0,10,16);
						renderengine->l_camera = vec3(0,0,0);
						renderengine->u_camera = vec3(0,1,0);

						// Modify to not blind player?
						renderengine->p_light = vec3(0,30,6);
						renderengine->p_light_scatter = vec3(0,-90,-100);
						renderengine->l_light = vec3(0,0,0);

						renderengine->setDeathScatter(false);

						break;

		}
}


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

		// Load the wav data.
		soundengine = new SoundEngine();
		soundengine->initialise();


		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
		viewport.w = 1024;
		viewport.h = 768;
		glutInitWindowSize(viewport.w,viewport.h);
		glutInitWindowPosition(0,0);
		glutCreateWindow("Inertia Alpha");


		//load render data
		renderengine = new RenderEngine();
		renderengine->init();


		// Set the fonts size and render a small text.
		evolutionFont = new FTGLTextureFont("resources/fonts/Evolution.ttf");
		digitalNinjaFont = new FTGLTextureFont("resources/fonts/DigitalNinja.ttf");
		accidentalPresidencyFont = new FTGLTextureFont("resources/fonts/AccidentalPresidency.ttf");
		evolutionBufferFont = new FTBufferFont("resources/fonts/Evolution.ttf");
		accidentalPresidencyBufferFont = new FTBufferFont("resources/fonts/AccidentalPresidency.ttf");

		evolutionFont->FaceSize(36);
		evolutionBufferFont->FaceSize(36);
		digitalNinjaFont->FaceSize(27);
		accidentalPresidencyFont->FaceSize(16);
		accidentalPresidencyBufferFont->FaceSize(16);


		//// If something went wrong, bail out.
		if(evolutionFont->Error() || digitalNinjaFont->Error() || accidentalPresidencyFont->Error() ||
						evolutionBufferFont->Error() || accidentalPresidencyBufferFont->Error())
				return -1;


		sweep = new Sweep();


		vehMesh = new Mesh();
		vehMesh->loadFile("resources/models/wipeoutship.obj");
		vehMesh->loadTextures("resources/textures/wipeoutship.png","resources/textures/wipeoutship.png");

		vehicle = new Vehicle(sweep, vehMesh);

		setMode(MODE_TITLE);

		//And Go!
		soundengine->start();
		glutMainLoop();
}

