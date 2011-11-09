#include "renderengine.h"

RenderEngine * activeEngine;


void renderFunction() {
		activeEngine->renderScene();
}

RenderEngine::RenderEngine() {
}

void RenderEngine::activate() { activeEngine = this; };

int RenderEngine::init() {


		p_camera = vec3(16,10,0);
		//Camera lookAt
		l_camera = vec3(0,0,0);
		//Camera up
		u_camera = vec3(0,1,0);
		//Light position
		p_light = vec3(110,60,0);
		//Light lookAt
		l_light = vec3(0,0,0);
		//Light Scatter Physical Light Location (used for demonstrations like title screen, where illumination is not same place as physical light in light scatter render)
		p_light_scatter = vec3(110,60,0);
		//Sky Color
		skyColor = vec4(0,0,0,0);
		//Trippy Light Scattering Mode
		deathScatter = false;

		FreeImage_Initialise();

		// set some lights
		{
				//float ambient[4] = { .5f, .5f, .5f, 1.f };
				float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
				float pos[4] = { p_light[0], p_light[1], p_light[2], 0 };
				//glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
				glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
				glLightfv(GL_LIGHT0, GL_POSITION, pos);
				glEnable(GL_LIGHT0);
		}

		glEnable(GL_DEPTH_TEST);
		glClearColor(0,0,0,1.0f);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

		this->activate();	

		glutDisplayFunc(renderFunction);
		glutIdleFunc(renderFunction);


		glewInit();

		if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
				printf("Ready for GLSL\n");
		else {
				printf("No GLSL support\n");
				exit(1);
		}

		//Generate FBOs
		generateShadowFBO();
		generateBlurFBO();
		generateLightFBO();

		//Load Shaders
		shade = new ShadowShader("shaders/MainVertexShader.c", "shaders/MainFragmentShader.c");
		blurShade = new BlurShader("shaders/GaussianBlurVertexShader.c", "shaders/GaussianBlurFragmentShader.c");
		depthShade = new GeometryShader("shaders/DepthVertexShader.c", "shaders/DepthFragmentShader.c");
		scatterShade = new ScatterShader("shaders/ScatteringVertexShader.c", "shaders/ScatteringFragmentShader.c");
		darkShade = new GeometryShader("shaders/SimpleDarkVertexShader.c", "shaders/SimpleDarkFragmentShader.c");

		//FTGL checking


}

//Draw Text
void RenderEngine::drawString(FTFont *font, string str, float x, float y) {
		FTPoint pos(x,y);
		font->Render(str.c_str(), -1, pos);
}


/*
 * Light Scattering stuff.
 */

//Grab the screen space location of the light
vec2 RenderEngine::getLightScreenCoor() {
		double modelView[16];
		double projection[16];
		GLint viewport[4];
		double depthRange[2];

		glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_DEPTH_RANGE, depthRange);

		GLdouble winX=0;
		GLdouble winY=0;
		GLdouble winZ=0;

		gluProject(p_light_scatter[0], p_light_scatter[1], p_light_scatter[2],
						modelView, projection, viewport,
						&winX, &winY, &winZ);

		return vec2(winX/kRenderWidth,  winY/kRenderHeight);
}

//Generate the FBO the light scatter shader will write to
void RenderEngine::generateLightFBO() {
		GLenum FBOstatus;
		glGenFramebuffersEXT(1, &scatterFboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, scatterFboId);

		glGenTextures(1, &scatterTextureId);
		glBindTexture(GL_TEXTURE_2D, scatterTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, kLightScatterWidth, kLightScatterHeight, 0 , GL_RGB, GL_FLOAT, 0);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, scatterTextureId, 0);

		FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
				printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for light FBO, CANNOT use FBO\n");

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}



/*
 * Shadow stuff. Will probably move somewhere else.
 */
//Generate the FBO the depth shader will write to
void RenderEngine::generateShadowFBO() {

		GLenum FBOstatus;

		// Try to use a texture depth component
		glGenTextures(1, &depthTextureId);
		glBindTexture(GL_TEXTURE_2D, depthTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Remove artefact on the edges of the shadowmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowMapWidth, kShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1,&colorTextureId);
		glBindTexture(GL_TEXTURE_2D, colorTextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

		// Remove artifact on the edges of the shadowmap
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, kShadowMapWidth, kShadowMapHeight, 0, GL_RGB, GL_FLOAT, 0);
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		// create a framebuffer object
		glGenFramebuffersEXT(1, &shadowFboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadowFboId);

		// attach the texture to FBO depth attachment point
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT ,GL_TEXTURE_2D, depthTextureId, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, colorTextureId, 0);

		// check FBO status
		FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
				printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for shadowmap FBO, CANNOT use FBO\n");

		// switch back to window-system-provided framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//Generate the FBO the gaussian blur shader will write to
void RenderEngine::generateBlurFBO() {

		GLenum FBOstatus;

		// Creating the blur FBO
		glGenFramebuffersEXT(1, &blurFboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, blurFboId);

		glGenTextures(1,&blurFboIdColorTextureId);
		glBindTexture(GL_TEXTURE_2D, blurFboIdColorTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, kShadowMapBlurWidth, kShadowMapBlurHeight, 0, GL_RGB, GL_FLOAT, 0);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, blurFboIdColorTextureId, 0);
		FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
				printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for blur FBO, CANNOT use FBO\n");

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//Take the depth texture and blur it.
void RenderEngine::blurShadowMap() {
		//glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		// Bluring the shadow map  horizontaly
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,blurFboId);	
		glViewport(0,0,kRenderWidth * kShadowMapCoef *kBlurCoef ,kRenderHeight* kShadowMapCoef*kBlurCoef);
		glUseProgramObjectARB(blurShade->getProgram());
		//glUniform2fARB( blurShade->getScaleAttrib(),1.0/ (kRenderWidth * kShadowMapCoef * kBlurCoef),0.0);
		glUniform2fARB( blurShade->getScaleAttrib(),1.0/512.0,0.0);		// horiz
		//glUniform1iARB(blurShade->getTextureSourceAttrib(),0);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,colorTextureId);


		//Preparing to draw quad
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-kRenderWidth/2,kRenderWidth/2,-kRenderHeight/2,kRenderHeight/2,1,20);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Drawing quad 
		glTranslated(0,0,-5);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0);glVertex3f(-kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,0);glVertex3f(kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,1);glVertex3f(kRenderWidth/2,kRenderHeight/2,0);
		glTexCoord2d(0,1);glVertex3f(-kRenderWidth/2,kRenderHeight/2,0);
		glEnd();
		//glGenerateMipmapEXT(GL_TEXTURE_2D);


		// Bluring vertically
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,shadowFboId);	
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	
		glViewport(0,0,kRenderWidth * kShadowMapCoef ,kRenderHeight* kShadowMapCoef);
		//glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (kRenderHeight * kShadowMapCoef ) );	
		glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (512.0 ) );
		glBindTexture(GL_TEXTURE_2D,blurFboIdColorTextureId);
		//glBindTexture(GL_TEXTURE_2D,colorTextureId);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0);glVertex3f(-kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,0);glVertex3f(kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,1);glVertex3f(kRenderWidth/2,kRenderHeight/2,0);
		glTexCoord2d(0,1);glVertex3f(-kRenderWidth/2,kRenderHeight/2,0);
		glEnd();

		glEnable(GL_CULL_FACE);
}

void RenderEngine::setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z,float up_x,float up_y,float up_z, float zNear, float zFar, float fovy)
{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//gluPerspective(45,kRenderWidth/kRenderHeight,zNear,zFar);
		gluPerspective(fovy,kRenderWidth/kRenderHeight,zNear,zFar);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,up_x,up_y,up_z);
}

void RenderEngine::setTextureMatrix() {
		static double modelView[16];
		static double projection[16];

		// This is matrix transform every coordinate x,y,z
		// x = x* 0.5 + 0.5 
		// y = y* 0.5 + 0.5 
		// z = z* 0.5 + 0.5 
		// Moving from unit cube [-1,1] to [0,1]  
		const GLdouble bias[16] = {	
				0.5, 0.0, 0.0, 0.0, 
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0};

		// Grab modelview and transformation matrices
		glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);

		glMatrixMode(GL_TEXTURE);
		glActiveTextureARB(GL_TEXTURE7);

		glLoadIdentity();	
		glLoadMatrixd(bias);

		// concatating all matrice into one.
		glMultMatrixd (projection);
		glMultMatrixd (modelView);

		// Go back to normal matrix mode
		glMatrixMode(GL_MODELVIEW);
}

//Render stuff
void RenderEngine::drawDebugBuffer(int option) {
		glUseProgramObjectARB(0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-kRenderWidth/2,kRenderWidth/2,-kRenderHeight/2,kRenderHeight/2,1,20);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor4f(1,1,1,1);
		glActiveTextureARB(GL_TEXTURE0);

		switch(option) {
				default:
				case DISPLAY_DEPTH_BUFFER:
						glBindTexture(GL_TEXTURE_2D,depthTextureId);
						break;
				case DISPLAY_DEPTH_SQUARED_HALF_BLUR_BUFFER:
						glBindTexture(GL_TEXTURE_2D,blurFboIdColorTextureId);
						break;
				case DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER:
						glBindTexture(GL_TEXTURE_2D,colorTextureId);
						break;
		}

		glEnable(GL_TEXTURE_2D);
		glTranslated(0,0,-1);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0);glVertex3f(0,0,0);
		glTexCoord2d(1,0);glVertex3f(kRenderWidth/2,0,0);
		glTexCoord2d(1,1);glVertex3f(kRenderWidth/2,kRenderHeight/2,0);
		glTexCoord2d(0,1);glVertex3f(0,kRenderHeight/2,0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
}



/*
 * The main display function. (Where all the magic happens).
 * Five renders done here.
 * 1) Depth render. Stores depth and depth squared for VSM
 * 2) Gaussian Blur render. Blurs depth render's output
 * 3) Light Path Tracing render. Path traces light rays for light scattering effect
 * 4) Main render. Rendered with shadows and without light scattering. Has phong illumination and shading, bump map.
 * 5) Light Scattering Overlay render. Overlays the output of the Light Path Tracing render as an additive overlay over the current scene.
 *
 */
void RenderEngine::renderScene() {
		//==FIRST RENDER: DEPTH BUFFER
		//Render from the light POV to a FBO, store depth and square depth in a 32F frameBuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,shadowFboId);

		//Using the depth shader to do so
		glUseProgramObjectARB(depthShade->getProgram());
		// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
		glViewport(0,0,kRenderWidth * kShadowMapCoef,kRenderHeight* kShadowMapCoef);
		//try to make shadow view "bigger" than normal view

		// Clear previous frame values
		glClearColor(0,0,0,1.0f);
		glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
		setupMatrices(p_light[0],p_light[1],p_light[2],l_light[0],l_light[1],l_light[2],0,1,0,10,100,120);

		// Culling switching, rendering only backface, this is done to avoid self-shadowing and improve efficiency
		glCullFace(GL_FRONT);
		//draw objects using the depth shader
		//drawObjects(depthShade);
		drawObjectTarget(depthShade);

		//cout << "0 " << glGetError() << endl;
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		//Save modelview/projection matrice into texture7, also add a biais
		setTextureMatrix();

		//==SECOND (and a half) RENDER: DOUBLE PASS GAUSSIAN BLUR
		blurShadowMap(); 

		//==THIRD RENDER: PATH TRACED LIGHT SCATTERING EFFECT (CREPUSCULAR RAYS)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,scatterFboId);

		glViewport(0,0,kLightScatterWidth,kLightScatterHeight);

		// Clear previous frame values
		if(deathScatter) {  
				//glClearColor(1,1,1,1.0f);
				glClearColor(1,0,0,1.0f);

		} else {
				glClearColor(0,0,0,1.0f);
		}
		//glClearColor(1,1,1,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Using the shadow shader
		glUseProgramObjectARB(darkShade->getProgram());
		glBindTexture(GL_TEXTURE_2D,colorTextureId);

		setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2],u_camera[0],u_camera[1],u_camera[2],0.5,120,70);

		glCullFace(GL_BACK);

		//Draw light
		glPushMatrix();
		glTranslatef(p_light_scatter[0],p_light_scatter[1],p_light_scatter[2]);
		glColor4f(1.0,1.0,1.0,1.0);
		glutSolidSphere(30,20,20);
		glPopMatrix();

		//Draw objects in black
		glColor4f(0.0f,0.0f,0.0f,1);
		//drawObjects(darkShade);
		drawObjectTarget(darkShade);


		//==FOURTH RENDER: MAIN RENDER (without light scattering)
		// Now rendering from the camera POV, using the FBO to generate shadows
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

		glViewport(0,0,kRenderWidth,kRenderHeight);

		// Clear previous frame values
		//glClearColor(.764705882,.890196078,1,1.0f);
		glClearColor(skyColor[0], skyColor[1], skyColor[2], skyColor[3]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Using the shadow shader
		glUseProgramObjectARB(shade->getProgram());
		//glUniform1iARB(shade->getShadowMapAttrib(),7);
		glActiveTextureARB(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D,colorTextureId);

		//declared in third pass
		//setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2],u_camera[0],u_camera[1],u_camera[2],1,120);

		//okay seriously, why do we have vec and float[] is required by openGL -_-
		float tempLight[4] = {p_light[0], p_light[1], p_light[2], 1};
		glLightfv(GL_LIGHT0, GL_POSITION, tempLight);

		glCullFace(GL_BACK);
		//draw objects using our shadow shader
		//drawObjects(shade);
		drawObjectTarget(shade);

		//==FIFTH PASS: LIGHT SCATTERING OVERLAY
		//uses main screen
		//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
		//glViewport(0,0,kRenderWidth,kRenderHeight);
		glClear (GL_DEPTH_BUFFER_BIT );

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgramObjectARB(scatterShade->getProgram());
		//default values

		vec2 cameraSpaceLightPos = getLightScreenCoor();
		glUniform1fARB(scatterShade->getExposureAttrib(),0.0034);
		glUniform1fARB(scatterShade->getDecayAttrib(),1.0);
		glUniform1fARB(scatterShade->getDensityAttrib(),0.84);
		glUniform1fARB(scatterShade->getWeightAttrib(),5.65);
		glUniform1iARB(scatterShade->getTextureAttrib(),0);
		glUniform2fARB(scatterShade->getLightPositionOnScreenAttrib(),cameraSpaceLightPos[0],cameraSpaceLightPos[1]);

		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,scatterTextureId);
		glEnable(GL_BLEND); //blend the resulting render
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-kRenderWidth/2,kRenderWidth/2,-kRenderHeight/2,kRenderHeight/2,1,20);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor4f(1.0f,1.0f,1.0f,1); //rectangle to display texture
		glPushMatrix();
		glTranslated(0,0,-5);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0);glVertex3f(-kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,0);glVertex3f(kRenderWidth/2,-kRenderHeight/2,0);
		glTexCoord2d(1,1);glVertex3f(kRenderWidth/2,kRenderHeight/2,0);
		glTexCoord2d(0,1);glVertex3f(-kRenderWidth/2,kRenderHeight/2,0);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);

		drawOverlayTarget();

		/*

			 if(renderOpt.isDepthBuffer())
			 drawDebugBuffer(renderOpt.getDepthBufferOption());
			 */

		glutSwapBuffers();    
}

