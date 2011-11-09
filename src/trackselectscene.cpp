#include "trackselectscene.h"
#include "gameengine.h"
#include "sweep.h"
#include "vehicle.h"

int TrackSelectScene::Init() {
	game_engine->render_engine()->skyColor = vec4(0,.152941176,.282352941,1);

	game_engine->render_engine()->p_camera = vec3(0,10,16);
	game_engine->render_engine()->l_camera = vec3(0,0,0);
	game_engine->render_engine()->u_camera = vec3(0,1,0);

	// Modify to not blind player?
	game_engine->render_engine()->p_light = vec3(0,30,6);
	game_engine->render_engine()->p_light_scatter = vec3(0,-90,-100);
	game_engine->render_engine()->l_light = vec3(0,0,0);

	game_engine->render_engine()->setDeathScatter(false);

	return 0;
}

void TrackSelectScene::DrawObjects(GeometryShader * curShade) {

	vec3 rotAxis(0.3,0.9,0);

	mat4 transformation = rotation3D(rotAxis, glutGet(GLUT_ELAPSED_TIME)/200.0);
	pushMat4(transformation);
	//sweep->renderWithDisplayList(*curShade,50,0.3,20);
	popTransform();

	pushMat4(scaling3D(vec3(0.1,0.1,0.1)).transpose() * transformation * translation3D(vec3(0,-3,0)).transpose());
	sweep->renderWithDisplayList(*curShade, 50, false, 0.3, 20);
	popTransform();
}
void TrackSelectScene::DrawOverlay() {
	//fudging this...
	//const float maxVelocityWidth = kRenderWidth * 2.5/8 /20;
	glEnable (GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgramObjectARB(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-kRenderWidth/2,kRenderWidth/2,-kRenderHeight/2,kRenderHeight/2,1,20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//==Actual HUD stuff
	std::ostringstream buff;
	glTranslated(0,0,-5);

	glColor4f(1,1,1,0.75);

	glBegin(GL_QUADS);
	glTexCoord2d(0,0);glVertex3f(-20,-kRenderHeight/2,0);
	glTexCoord2d(1,0);glVertex3f(kRenderWidth*0.45,-kRenderHeight/2,0);
	glTexCoord2d(1,1);glVertex3f(kRenderWidth*0.45,-kRenderHeight*25/64,0);
	glTexCoord2d(0,1);glVertex3f(-20,-kRenderHeight*25/64,0);
	glEnd();

	glColor4f(.188235294,.474509804,1,0.9);
	accidentalPresidencyFont->FaceSize(36);
	buff.str("");
	buff << "Press G/X to generate a new track";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(), 0, -kRenderHeight * 28/64);

	buff.str("");
	buff << "Press Space/Start to start!";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(), 0, -kRenderHeight * 31/64);

	//Name 
	buff.str("");
	buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
	game_engine->render_engine()->drawString(accidentalPresidencyBufferFont, buff.str(), -kRenderWidth*3.9/8, kRenderHeight*3.85/8);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void TrackSelectScene::GenerateNewTrack() {
	sweep->Regenerate();
	vehicle->reset();
}

void TrackSelectScene::KeyboardNormalDown(unsigned char key, int x, int y) {
	switch (key) {
		case 'Q':
		case 'q':
		case 27:	
			game_engine->sound_engine()->playSelectSource2();
			game_engine->SetTitleMode();
			break;
		case 'A':
		case 'a':
			game_engine->render_engine()->toggleDeathScatter();
			break;
		case 'G':
		case 'g':
			game_engine->sound_engine()->playSelectSource2();
			GenerateNewTrack();
			break;
			//temp keys for debugging
		case ' ':
			game_engine->sound_engine()->playSelectSource1();
			game_engine->SetRaceMode();
			break;
	}
}

void TrackSelectScene::Joystick(unsigned int buttonMask, int x, int y, int z) {
	//cout << (buttonMask) << endl;
	//cout << (buttonMask & 16384) << endl;
	if(lastStartPress > 1000) {
		if(buttonMask & 16384) {
			lastStartPress = glutGet(GLUT_ELAPSED_TIME);
			GenerateNewTrack();
		}
		if(buttonMask & 8) { //start button
			lastStartPress = glutGet(GLUT_ELAPSED_TIME);
			game_engine->SetRaceMode();
		}

	}  
	//		cout << buttonMask << endl;
}
