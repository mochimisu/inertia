#ifndef RACESCENE_H_
#define RACESCENE_H_

#include "scene.h"

class GameEngine;
class Vehicle;
class Mesh;
class Sweep;

class RaceScene : public Scene {
	
	public:
		RaceScene(GameEngine * ge) : Scene(ge) {};
		virtual int Init();
		virtual void SetActiveEngine(GameEngine * game_engine);
		virtual void DrawObjects(GeometryShader * curShade);
		virtual void DrawOverlay();
		virtual void KeyboardNormalDown(unsigned char key, int x, int y);
		virtual void KeyboardNormalUp(unsigned char key, int x, int y);
		virtual void KeyboardSpecialDown(int key, int x, int y);
		virtual void KeyboardSpecialUp(int key, int x, int y);
		virtual void Joystick(unsigned int buttonMask, int x, int y, int z);
		void set_sweep(Sweep * swp) { sweep = swp; };
		void set_vehicle(Vehicle * veh) { vehicle = veh; };
		void set_vehicle_mesh(Mesh * meh) { vehMesh = meh; };
		Vehicle * get_vehicle() { return vehicle; };
	protected:
		Sweep * sweep;
		Vehicle * vehicle;
		Mesh * vehMesh;
		int last_time_step;
};

#endif
