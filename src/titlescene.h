#ifndef TITLESCENE_H_
#define TITLESCENE_H_

#include "scene.h"

class Mesh;

class TitleScene : public Scene {
	
	public:
		TitleScene(GameEngine * ge) : Scene(ge) {};
		virtual int Init();
		virtual void DrawObjects(GeometryShader * curShade);
		virtual void DrawOverlay();
		virtual void KeyboardNormalDown(unsigned char key, int x, int y);
		virtual void Joystick(unsigned int buttonMask, int x, int y, int z);
		void set_vehicle_mesh(Mesh * meh) { vehMesh = meh; };
	protected:
		Mesh * vehMesh;
};

#endif

