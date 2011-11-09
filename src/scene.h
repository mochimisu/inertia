#ifndef SCENE_H_
#define SCENE_H_

#include "global.h"

class GeometryShader;
class FTGLTextureFont;
class FTBufferFont;
class GameEngine;

class Scene {
	public:
		Scene(GameEngine * ge) { game_engine = ge; };
		virtual int Init();
		virtual void DrawOverlay() {}
		virtual void DrawObjects(GeometryShader * curShade) {}
		virtual void KeyboardNormalDown(unsigned char key, int x, int y) {}
		virtual void KeyboardNormalUp(unsigned char key, int x, int y) {}
		virtual void KeyboardSpecialDown(int key, int x, int y) {}
		virtual void KeyboardSpecialUp(int key, int x, int y) {}
		virtual void Joystick(unsigned int buttonMask, int x, int y, int z) {}
		virtual void ActiveMotion(int x, int y) {}
		virtual void PassiveMotion(int x, int y) {}
		virtual ~Scene() {}
	protected: 
		GameEngine * game_engine;

		static FTGLTextureFont * evolutionFont;
		static FTGLTextureFont * digitalNinjaFont;
		static FTGLTextureFont * accidentalPresidencyFont;
		static FTBufferFont * evolutionBufferFont;
		static FTBufferFont * accidentalPresidencyBufferFont;

		static int lastStartPress;

		static bool initialised;
};

#endif
