#include "gameengine.h"

Scene * active_scene;

void ActiveKeyboardNormalUp(unsigned char key, int x, int y) {
	return active_scene->KeyboardNormalUp(key, x, y);
}

void ActiveKeyboardNormalDown(unsigned char key, int x, int y) {
	return active_scene->KeyboardNormalDown(key, x, y);
}

void ActiveKeyboardSpecialDown(int key, int x, int y) {
	return active_scene->KeyboardSpecialDown(key, x, y);
}

void ActiveKeyboardSpecialUp(int key, int x, int y) {
	return active_scene->KeyboardSpecialUp(key, x, y);
}

void ActiveJoystick(unsigned int buttonMask, int x, int y, int z ) {
	return active_scene->Joystick(buttonMask, x, y, z);
}

void ActiveActiveMotion(int x, int y) {
	return active_scene->ActiveMotion(x, y);
}

void ActivePassiveMotion(int x, int y) {
	return active_scene->PassiveMotion(x, y);
}

void ActiveDrawObjects(GeometryShader * curShade) {
	return active_scene->DrawObjects(curShade);
}

void ActiveDrawOverlay() {
	return active_scene->DrawOverlay();
}

void GameEngine::Init() {
	// Load the wav data.
	sound_engine_ = new SoundEngine();
	sound_engine_->initialise();

	//load render data
	render_engine_ = new RenderEngine();
	render_engine_->init();


	sweep = new Sweep();


	vehMesh = new Mesh();
	vehMesh->loadFile("resources/models/wipeoutship.obj");
	vehMesh->loadTextures("resources/textures/wipeoutship.png","resources/textures/wipeoutship.png");

	vehicle = new Vehicle(sweep, vehMesh);

	track_select_scene = new TrackSelectScene(this);
	race_scene = new RaceScene(this);
	title_scene = new TitleScene(this);
	death_scene = new DeathScene(this);


	track_select_scene->set_sweep(sweep);
	race_scene->set_sweep(sweep);

	title_scene->set_vehicle_mesh(vehMesh);
	race_scene->set_vehicle_mesh(vehMesh);
	track_select_scene->set_vehicle_mesh(vehMesh);

	race_scene->set_vehicle(vehicle);
	track_select_scene->set_vehicle(vehicle);

	render_engine_->setDrawObjects(ActiveDrawObjects);
	render_engine_->setDrawOverlay(ActiveDrawOverlay);
	glutKeyboardFunc(ActiveKeyboardNormalDown);
	glutKeyboardUpFunc(ActiveKeyboardNormalUp);
	glutSpecialFunc(ActiveKeyboardSpecialDown);
	glutSpecialUpFunc(ActiveKeyboardSpecialUp);
	glutJoystickFunc(ActiveJoystick,10);
	glutMotionFunc(ActiveActiveMotion);
	glutPassiveMotionFunc(ActivePassiveMotion);

}

void GameEngine::Go() {
	SetTitleMode();
	sound_engine_->start();
}

void GameEngine::SetTitleMode() {
	is_racing = false;
	SetSceneFunc(title_scene);
}

void GameEngine::SetRaceMode() { 
	is_racing = true;
	race_scene->SetActiveEngine(this);
	SetSceneFunc(race_scene);
}

void GameEngine::SetTrackSelectMode() {
	is_racing = false;
	SetSceneFunc(track_select_scene);
}

void GameEngine::SetDeathMode() {
	is_racing = false;
	SetSceneFunc(death_scene);
}

void GameEngine::SetSceneFunc(Scene * scene) {
	sound_engine_->stopEngineSound();
	active_scene = scene;
	scene->Init();
}