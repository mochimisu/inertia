#include "titlescene.h"
#include "mesh.h"
#include "gameengine.h"

int TitleScene::Init() {
	Scene::Init();
	vehMesh->centerAndScale(40);

	game_engine->render_engine()->skyColor = vec4(0,.152941176,.282352941,1);

	game_engine->render_engine()->p_camera = vec3(0,10,16);
	game_engine->render_engine()->l_camera = vec3(0,0,0);
	game_engine->render_engine()->u_camera = vec3(0,1,0);

	game_engine->render_engine()->p_light_scatter = vec3(0,-30,-40);
	game_engine->render_engine()->l_light = vec3(0,0,0);        
	game_engine->render_engine()->p_light = vec3(0,30,6);

	game_engine->render_engine()->setDeathScatter(false);

	return 0;
}

void TitleScene::DrawObjects(GeometryShader * curShade) {

	vec3 rotAxis(0.3,0.9,0);

	mat4 transformation = rotation3D(rotAxis, glutGet(GLUT_ELAPSED_TIME)/200.0);
	pushMat4(transformation);
	vehMesh->draw(*curShade); 
	popTransform();


}
void TitleScene::DrawOverlay() {
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
	glTexCoord2d(0,0);glVertex3f(-20,-kRenderHeight*3/34,0);
	glTexCoord2d(1,0);glVertex3f(kRenderWidth*0.30,-kRenderHeight*3/34,0);
	glTexCoord2d(1,1);glVertex3f(kRenderWidth*0.30,kRenderHeight/6,0);
	glTexCoord2d(0,1);glVertex3f(-20,kRenderHeight/6,0);


  /*
	glTexCoord2d(0,0);glVertex3f(-kRenderWidth/2,-kRenderHeight/2,0);
	glTexCoord2d(1,0);glVertex3f(-kRenderWidth*3/16,-kRenderHeight/2,0);
	glTexCoord2d(1,1);glVertex3f(-kRenderWidth*3/16,-kRenderHeight*5/16,0);
	glTexCoord2d(0,1);glVertex3f(-kRenderWidth/2,-kRenderHeight*5/16,0);
  */
	glEnd();

	buff.str("");
	buff << "INERTIA";
	//glColor4f(.188235294,.474509804,1,0.9);
  glColor4f(0,0,0,0.9);


	evolutionFont->FaceSize(130);
	game_engine->render_engine()->drawString(evolutionFont, buff.str(),0,0); 
	evolutionFont->FaceSize(36);

  buff.str("");
  buff << "Real-time Racing Game";
  accidentalPresidencyFont->FaceSize(35);
  game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),0,-30); 
	buff.str("");
	buff << "C++, GLSL, GLUT, OpenAL, OpenGL";
  accidentalPresidencyFont->FaceSize(20);
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),0,-53); 

	if((glutGet(GLUT_ELAPSED_TIME)/500)%2) {
		glColor4f(.188235294,.474509804,1,0.9);
		buff.str("");
		buff << "Press Space or Start";
		//game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth*0.065,-kRenderHeight*3/8 - 5); 
	}
  glColor4f(0,0,0,0.9);

  /*
	buff.str("");
	buff << "Controls";
	game_engine->render_engine()->drawString(evolutionFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*2.85/8); 

	buff.str("");
	buff << "Left and Right Arrow/Analog Stick: Turn";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*3.05/8);

	buff.str("");
	buff << "Up Arrow/X Button: Accelerate";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*3.25/8);

	buff.str("");
	buff << "Down Arrow/O Button: Decelerate";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*3.45/8);  

	buff.str("");
	buff << "Space/L2 or R2: Air Brake";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*3.65/8);  

	buff.str("");
	buff << "Esc/Start: Quit";
	game_engine->render_engine()->drawString(accidentalPresidencyFont, buff.str(),-kRenderWidth/2 + 10, -kRenderHeight*3.85/8);  






	//Name 
	buff.str("");
	buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
	game_engine->render_engine()->drawString(accidentalPresidencyBufferFont, buff.str(), -kRenderWidth*3.9/8, kRenderHeight*3.85/8);
  */

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void TitleScene::KeyboardNormalDown(unsigned char key, int x, int y) {

	switch (key) {
		case 'Q':
		case 'q':
		case 27:	
			exit(0);
			break;
		case 'A':
		case 'a':
			game_engine->render_engine()->toggleDeathScatter();
			break;
			//temp keys for debugging
		case ' ':
			game_engine->sound_engine()->playSelectSource1();
			game_engine->SetTrackSelectMode();
			break;
	}
}


void TitleScene::Joystick(unsigned int buttonMask, int x, int y, int z) {
	//cout << (buttonMask) << endl;
	//cout << (buttonMask & 16384) << endl;
	if(buttonMask & 8 && (glutGet(GLUT_ELAPSED_TIME) - lastStartPress > 1000)) { //start button
		lastStartPress = glutGet(GLUT_ELAPSED_TIME);
		game_engine->SetRaceMode();
	}

	//		cout << buttonMask << endl;
}


