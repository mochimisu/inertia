#include "racescene.h"


//Only 1 engine is available at a time...
//GLUT/C++ limitation, timerfunc must not be a class method...
GameEngine * active_game_engine;

//Steps a vehicle 
//because of glut's callback limitation in the fact that we can't interrupt, we have to keep polling if we're in the right mode........
//How to reference game_engine??
void stepVehicle(int x) {
	//call at beginning for consistency..
	int newTime =glutGet(GLUT_ELAPSED_TIME);
	if(active_game_engine->IsRacing()) {
		int timeDif = newTime - x;
		Vehicle * vehicle = active_game_engine->get_race_scene()->get_vehicle();
		vehicle->step(0.01 * timeDif/10.0);

		active_game_engine->render_engine()->p_camera = vehicle->cameraPos();
		active_game_engine->render_engine()->l_camera = vehicle->cameraLookAt();
		active_game_engine->render_engine()->u_camera = vehicle->getUp();

		//p_light = vehicle->lightPos();
		active_game_engine->render_engine()->l_light = vehicle->worldSpacePos();

		if(vehicle->getEnergy() < 0.0000001) {
			active_game_engine->sound_engine()->playDeathSound();
			active_game_engine->SetDeathMode();
		}

		//redo this every 15ms
		glutTimerFunc(15,stepVehicle, newTime);
	}

}

int RaceScene::Init() {

	Scene::Init();

	vehMesh->centerAndScale(0.8);	

	game_engine->render_engine()->skyColor = vec4(.764705882,.890196078,1,1);
	game_engine->render_engine()->p_light = vec3(110,60,0);
	game_engine->render_engine()->p_light_scatter = vec3(110,60,0);


	vehicle->reset();
	//Lap time 
	vehicle->setLapStartTime(glutGet(GLUT_ELAPSED_TIME));
	
	game_engine->render_engine()->setDeathScatter(false);

	//Step Vehicle once (and it will recurse on timer)
	stepVehicle(glutGet(GLUT_ELAPSED_TIME));

	lastStartPress = 0;

	return 0;

}

void RaceScene::SetActiveEngine(GameEngine * game_engine) {
	active_game_engine = game_engine;
}

void RaceScene::DrawObjects(GeometryShader * curShade) {
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

void RaceScene::DrawOverlay() {
	//fudging this... hack hack hack hack hack
	const float maxVelocityWidth = kRenderWidth * 2.5/8 /20;

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

	//backdrops 

	//header
	glColor4f(.250980392,.458823529,.631372549,0.95);
	glBegin(GL_QUADS);
	glVertex3f(-kRenderWidth/2,kRenderHeight * 3.8/8,0);
	glVertex3f(kRenderWidth/2,kRenderHeight * 3.8/8,0);
	glVertex3f(kRenderWidth/2,kRenderHeight/2,0);
	glVertex3f(-kRenderWidth/2,kRenderHeight/2,0);

	glColor4f(.250980392,.458823529,.631372549,0.1);

	//Lap
	glVertex3f(-kRenderWidth/2,kRenderHeight * 3.0/8,0);
	glVertex3f(-kRenderWidth * 3.4/8,kRenderHeight * 3.0/8,0);
	glVertex3f(-kRenderWidth * 3.4/8,kRenderHeight * 3.8/8,0);
	glVertex3f(-kRenderWidth/2,kRenderHeight * 3.8/8,0);

	//Record (with record to have less branches)
	/*
		 glBegin(GL_QUADS);
		 glVertex3f(kRenderWidth * 2.2/8,kRenderHeight * 3.0/8,0);
		 glVertex3f(kRenderWidth/2,kRenderHeight * 3.0/8,0);
		 glVertex3f(kRenderWidth/2,kRenderHeight * 3.8/8,0);
		 glVertex3f(kRenderWidth * 2.2/8,kRenderHeight * 3.8/8,0);
		 glEnd(); */


	//Energy
	glVertex3f(-65,kRenderHeight * 3.0/8,0);
	glVertex3f(65,kRenderHeight * 3.0/8,0);
	glVertex3f(65,kRenderHeight * 3.8/8,0);
	glVertex3f(-65,kRenderHeight * 3.8/8,0);

	//Lap Time
	glVertex3f(-kRenderWidth/2, -kRenderHeight*4.0/8,0);
	glVertex3f(-kRenderWidth*2.5/8, -kRenderHeight*4.0/8,0);
	glVertex3f(-kRenderWidth*2.5/8, -kRenderHeight*3.0/8,0);
	glVertex3f(-kRenderWidth/2, -kRenderHeight*3.0/8,0);

	//Velocity
	glVertex3f(kRenderWidth*2/8, -kRenderHeight*4.0/8,0);
	glVertex3f(kRenderWidth/2, -kRenderHeight*4.0/8,0);
	glVertex3f(kRenderWidth/2, -kRenderHeight*3.0/8,0);
	glVertex3f(kRenderWidth*2/8, -kRenderHeight*3.0/8,0);
	glEnd();

	//Actual stuff

	//Velocity
	buff.str("");
	buff << "Velocity";
	glColor4f(.188235294,.474509804,1,0.9);
	game_engine->render_engine()->drawString(evolutionBufferFont, buff.str(),kRenderWidth*2.3/8,-kRenderHeight*3.4/8 + 5); 

	//Velocity Bar
	glColor4f(.188235294,.474509804,1,0.5);
	glBegin(GL_QUADS);
	glVertex3f(kRenderWidth*2.1/8,-kRenderHeight*3.8/8,0); // bottom left
	glVertex3f(kRenderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-kRenderHeight*3.8/8,0); //bottom right
	glVertex3f(kRenderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-kRenderHeight*3.4/8,0); //top right
	glVertex3f(kRenderWidth*2.1/8,-kRenderHeight*3.4/8,0); //top left
	glEnd();

	//Velocity Text
	buff.str("");
	double velocity = vehicle->getVelocityScalar();
	buff << setfill('0') << setw(3) << int(velocity * 10) << ".";
	buff << setfill('0') << setw(3) << (int(velocity * 10000) % 1000);
	glColor4f(.9,.9,1,0.8);
	game_engine->render_engine()->drawString(digitalNinjaFont, buff.str(),kRenderWidth*2.3/8,-kRenderHeight*3.3/8 - 44); 

	//Air Break Bar
	if(vehicle->isAirBrake()) {

		glColor4f(.901960784,.160784314,.160784314,0.5);  

		glBegin(GL_QUADS);
		glVertex3f(kRenderWidth*2.1/8,-kRenderHeight*3.9/8,0); // bottom left
		glVertex3f(kRenderWidth*3.8/8 ,-kRenderHeight*3.9/8,0); //bottom right
		glVertex3f(kRenderWidth*3.8/8,-kRenderHeight*3.8/8,0); //top right
		glVertex3f(kRenderWidth*2.1/8,-kRenderHeight*3.8/8,0); //top left
		glEnd();
	} 

	//Lap Time
	buff.str("");
	buff << "Lap Time";
	glColor4f(1,1,1,0.75);
	game_engine->render_engine()->drawString(evolutionBufferFont, buff.str(),-kRenderWidth*3.8/8,-kRenderHeight*3.4/8); 

	buff.str("");
	int msTime = glutGet(GLUT_ELAPSED_TIME) - vehicle->getLapStartTime();
	int sTime = msTime/1000;
	int mTime  = sTime/60;
	buff << setfill('0') << setw(2) << mTime << ".";
	buff << setfill('0') << setw(2) << (sTime%60) << ".";
	buff << setfill('0') << setw(3) << (msTime%1000);
	glColor4f(1,1,1,0.75);
	game_engine->render_engine()->drawString(digitalNinjaFont, buff.str(),-kRenderWidth*3.8/8,-kRenderHeight*3.7/8); 

	//Lap Number
	buff.str("");
	buff << "Lap"; // << vehicle->getLap();
	glColor4f(1,1,1,0.75);
	game_engine->render_engine()->drawString(evolutionBufferFont, buff.str(),-kRenderWidth*3.9/8,kRenderHeight*3.5/8); 

	buff.str("");
	buff << vehicle->getLap();
	game_engine->render_engine()->drawString(digitalNinjaFont, buff.str(), -kRenderWidth * 3.9/8, kRenderHeight*3.2/8);

	//Record
	buff.str("");
	msTime = vehicle->getBestLapTime();
	if(msTime != -1) {
		//backdrop
		glColor4f(.250980392,.458823529,.631372549,0.3);
		glBegin(GL_QUADS);
		glVertex3f(kRenderWidth * 2.2/8,kRenderHeight * 3.0/8,0);
		glVertex3f(kRenderWidth*2/2,kRenderHeight * 3.0/8,0);
		glVertex3f(kRenderWidth*2/2,kRenderHeight * 3.8/8,0);
		glVertex3f(kRenderWidth * 2.2/8,kRenderHeight * 3.8/8,0);
		glEnd();

		sTime = msTime/1000;
		mTime  = sTime/60;
		buff << "Lap Record:";
		game_engine->render_engine()->drawString(evolutionBufferFont, buff.str(),kRenderWidth*2.4/8, kRenderHeight*3.5/8);

		buff.str("");
		buff << setfill('0') << setw(2) << mTime << ".";
		buff << setfill('0') << setw(2) << (sTime%60) << ".";
		buff << setfill('0') << setw(3) << (msTime%1000);
		glColor4f(1,1,1,0.75);
		game_engine->render_engine()->drawString(digitalNinjaFont, buff.str(),kRenderWidth*2.4/8, kRenderHeight*3.2/8);

	}

	//Name 
	buff.str("");
	buff << "cs184sp11 final project: inertia. submission version. brandon wang, andrew lee, chris tandiono";
	game_engine->render_engine()->drawString(accidentalPresidencyBufferFont, buff.str(), -kRenderWidth*3.9/8, kRenderHeight*3.85/8);

	//Energy
	buff.str("");
	buff << "Energy: ";
	//buff << vehicle->getEnergy();
	game_engine->render_engine()->drawString(evolutionBufferFont, buff.str(), -55,kRenderHeight*3.5/8);

	buff.str("");
	double energy = vehicle->getEnergy();
	buff << int(energy) << ".";
	buff  << setfill('0') << setw(3) << (int(1000 * energy) % 1000);
	game_engine->render_engine()->drawString(digitalNinjaFont, buff.str(), -45,kRenderHeight*3.2/8);


	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void RaceScene::KeyboardNormalDown(unsigned char key, int x, int y) {
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
		case ' ':
			vehicle->setAirBrake(0.00008);
			break;
	}
}

void RaceScene::KeyboardNormalUp(unsigned char key, int x, int y) {
	switch(key) {
		case ' ':
			vehicle->setAirBrake(0);
			break;
	}
}

void RaceScene::KeyboardSpecialDown(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			game_engine->sound_engine()->playEngineSound();
			vehicle->setAccel(0.2);
			break;
		case GLUT_KEY_DOWN:
			game_engine->sound_engine()->playEngineSound();
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

void RaceScene::KeyboardSpecialUp(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			game_engine->sound_engine()->stopEngineSound();
			vehicle->setAccel(0.0);
			break;
		case GLUT_KEY_LEFT:
		case GLUT_KEY_RIGHT:
			vehicle->turnLeft(0.0);
			break;
	}
}

void RaceScene::Joystick(unsigned int buttonMask, int x, int y, int z) {
	//cout << (buttonMask) << endl;
	//cout << (buttonMask & 16384) << endl;
	if(buttonMask & 16384 || buttonMask & 1) { //button 14: X on DualShock3, button 1 is trigger on Chris' joystick
		vehicle->setAccel(0.2);
		game_engine->sound_engine()->playEngineSound();
	} else if(buttonMask & 8192) { //button 13: O on DualShock3
		vehicle->setAccel(-0.1);
		game_engine->sound_engine()->playEngineSound(); 
	} else {
		game_engine->sound_engine()->stopEngineSound();
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
		game_engine->SetTitleMode();
	}
}

