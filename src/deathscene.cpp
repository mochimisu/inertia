#include "deathscene.h"
#include "gameengine.h"

//just displays a YOU DIED message, everything else the same as title scene
void DeathScene::DrawOverlay() {
	TitleScene::DrawOverlay();


	glEnable (GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgramObjectARB(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	std::ostringstream buff;
	glTranslated(0,0,-5);


	glColor4f(1,1,1,0.75);
	glBegin(GL_QUADS);
	glTexCoord2d(0,0);glVertex3f(-renderWidth*0.45,-renderHeight*3/24,0);
	glTexCoord2d(1,0);glVertex3f(-20,-renderHeight*3/24,0);
	glTexCoord2d(1,1);glVertex3f(-20,renderHeight/6,0);
	glTexCoord2d(0,1);glVertex3f(-renderWidth*0.45,renderHeight/6,0);
	glEnd();


	buff.str("");
	buff << "YOU DIED!";
	glColor4f(1,0,0,0.9);


	evolutionFont->FaceSize(100);
	game_engine->render_engine()->drawString(evolutionFont, buff.str(),-renderWidth*0.45+20,0); 
	evolutionFont->FaceSize(36);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

}
