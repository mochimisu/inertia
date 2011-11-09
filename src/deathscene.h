#ifndef DEATHSCENE_H_
#define DEATHSCENE_H_

#include "titlescene.h"

class DeathScene : public TitleScene {
	public:
		DeathScene(GameEngine * ge) : TitleScene(ge) {};
		virtual void DrawOverlay();
};

#endif


