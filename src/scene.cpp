#include "scene.h"
#include <FTGL/ftgl.h>

bool Scene::initialised = false;

FTGLTextureFont * Scene::evolutionFont = 0;
FTGLTextureFont * Scene::digitalNinjaFont = 0;
FTGLTextureFont * Scene::accidentalPresidencyFont = 0;
FTBufferFont * Scene::evolutionBufferFont = 0;
FTBufferFont * Scene::accidentalPresidencyBufferFont = 0;
int Scene::lastStartPress = 0;

int Scene::Init() {
	if(!initialised) {
		// Set the fonts size and render a small text.
		evolutionFont = new FTGLTextureFont("resources/fonts/Evolution.ttf");
		digitalNinjaFont = new FTGLTextureFont("resources/fonts/DigitalNinja.ttf");
		accidentalPresidencyFont = new FTGLTextureFont("resources/fonts/AccidentalPresidency.ttf");
		evolutionBufferFont = new FTBufferFont("resources/fonts/Evolution.ttf");
		accidentalPresidencyBufferFont = new FTBufferFont("resources/fonts/AccidentalPresidency.ttf");

		evolutionFont->FaceSize(36);
		evolutionBufferFont->FaceSize(36);
		digitalNinjaFont->FaceSize(27);
		accidentalPresidencyFont->FaceSize(16);
		accidentalPresidencyBufferFont->FaceSize(16);

		//// If something went wrong, bail out.
		if(evolutionFont->Error() || digitalNinjaFont->Error() || accidentalPresidencyFont->Error() ||
				evolutionBufferFont->Error() || accidentalPresidencyBufferFont->Error())
			return -1;
		initialised = true;
	}
	return 0;
}
