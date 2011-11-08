#ifndef RENDERENGINE_H_
#define RENDERENGINE_H_

#include "global.h" 
#include "shaders.h"
#include <FTGL/ftgl.h>
#include "Viewport.h"

//===DEBUG STUFF 
//background texture
enum { DISPLAY_DEPTH_BUFFER,
		DISPLAY_DEPTH_SQUARED_HALF_BLUR_BUFFER,
		DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER };



//===RENDER CONSTANTS
const float shadowMapCoef = 0.5;
const float blurCoef = 0.5;
const float lightScatteringCoef = 0.5;

const float renderWidth = 1024.0;
const float renderHeight = 768.0;
const float shadowMapWidth = renderWidth * shadowMapCoef;
const float shadowMapHeight = renderHeight * shadowMapCoef;
const float shadowMapBlurWidth = shadowMapWidth * blurCoef;
const float shadowMapBlurHeight = shadowMapHeight * blurCoef;
const float lightScatterWidth = renderWidth * lightScatteringCoef;
const float lightScatterHeight = renderHeight * lightScatteringCoef;
class RenderEngine;

class RenderEngine {
		public:
				RenderEngine();

				int init();

				void setDrawObject(void (*drawFn)(GeometryShader *)) { drawObjectTarget = drawFn; };
				void setDrawOverlay(void (*drawFn)()) { drawOverlayTarget = drawFn; };

				//Draw Text
				void drawString(FTFont *font, string str, float x, float y);

				// Light Scattering stuff.
				//Grab the screen space location of the light
				vec2 getLightScreenCoor();
				//Generate the FBO the light scatter shader will write to
				void generateLightFBO();

				//Shadow stuff. Will probably move somewhere else.
				//Generate the FBO the depth shader will write to
				void generateShadowFBO();
				//Generate the FBO the gaussian blur shader will write to
				void generateBlurFBO();
				//Take the depth texture and blur it.
				void blurShadowMap(); 
				void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z,float up_x,float up_y,float up_z, float zNear, float zFar, float fovy);
				void setTextureMatrix();

				//Render stuff
				void drawDebugBuffer(int option);
				void renderScene();

				void activate();

				void toggleDeathScatter() { deathScatter = !deathScatter; };
				void setDeathScatter(bool val) { deathScatter = val; };

				//===SCENE DESCRIPTORS
				//Camera position
				vec3 p_camera;
				//Camera lookAt
				vec3 l_camera;
				//Camera up
				vec3 u_camera;
				//Light position
				vec3 p_light;
				//Light lookAt
				vec3 l_light;
				//Light Scatter Physical Light Location (used for demonstrations like title screen, where illumination is not same place as physical light in light scatter render)
				vec3 p_light_scatter;
				//Sky Color
				vec4 skyColor;
		private:

				//Pointer to render functions
				void (*drawObjectTarget)(GeometryShader *);
				void (*drawOverlayTarget)();

				//Trippy Light Scattering Mode
				bool deathScatter;


				//==FRAME BUFFER OBJECTS/RELATED TEXTURES
				// Hold id of the framebuffer for light POV rendering
				GLuint shadowFboId;


				//===SHADOW STUFF
				// Z values will be rendered to this texture when using fboId framebuffer
				GLuint depthTextureId;
				GLuint colorTextureId;
				// Bluring FBO
				GLuint blurFboId;
				// Z values will be rendered to this texture when using fboId framebuffer
				GLuint blurFboIdColorTextureId;

				//===SHADERS
				ShadowShader *shade;
				BlurShader *blurShade;
				GeometryShader *depthShade;
				ScatterShader *scatterShade;
				GeometryShader *darkShade;


				//==LIGHT SCATTERING STUFF
				GLuint scatterTextureId;
				GLuint scatterFboId;



};

#endif
