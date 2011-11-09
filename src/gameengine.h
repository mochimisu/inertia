#ifndef GAMEENGINE_H_
#define GAMEENGINE_H_

#include "global.h"
#include "renderengine.h"
#include "soundengine.h"
#include "scene.h"

#include "racescene.h"
#include "trackselectscene.h"
#include "titlescene.h"
#include "deathscene.h"

#include "mesh.h"

class Scene;
class TitleScene;
class TrackSelectScene;
class RaceScene;
class DeathScene;

class GameEngine {
	public:
		void Init();
		void Go();
		RenderEngine * render_engine() { return render_engine_; }
		SoundEngine * sound_engine() { return sound_engine_; }
		void SetTitleMode();
		void SetRaceMode();
		void SetTrackSelectMode();
		void SetDeathMode();
		RaceScene * get_race_scene() { return race_scene; };
		bool IsRacing() { return is_racing; };
	private:
		void SetSceneFunc(Scene * scene);
		
		bool is_racing;
		Sweep * sweep;
		Mesh * vehMesh;
		Vehicle * vehicle;

		RenderEngine * render_engine_;
		SoundEngine * sound_engine_;
		TitleScene * title_scene;
		TrackSelectScene * track_select_scene;
		RaceScene * race_scene;
		DeathScene * death_scene;
};

#endif
