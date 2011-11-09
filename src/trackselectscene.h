#ifndef TRACKSELECTSCENE_H_
#define TRACKSELECTSCENE_H_

#include "scene.h"

class Vehicle;
class Mesh;
class Sweep;

class TrackSelectScene : public Scene {
	public:
		TrackSelectScene(GameEngine * ge) : Scene(ge) {};
		virtual int Init();
		virtual void DrawObjects(GeometryShader * curShade);
		virtual void DrawOverlay();
		virtual void KeyboardNormalDown(unsigned char key, int x, int y);
		virtual void GenerateNewTrack();
		virtual void Joystick(unsigned int buttonMask, int x, int y, int z);
		void set_sweep(Sweep * swp) { sweep = swp; };
		void set_vehicle(Vehicle * veh) { vehicle = veh; };
		void set_vehicle_mesh(Mesh * meh) { vehMesh = meh; };
	protected:
		Vehicle * vehicle;
		Mesh * vehMesh;
		Sweep * sweep;
};

#endif
