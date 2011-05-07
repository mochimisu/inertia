#include "main.h"
#include "functions.h"

// Constants (some issues with aspect ratio; and i think defines will speed some stuff up. keep it?)
const float renderWidth = 1024.0;
const float renderHeight = 768.0;
const float shadowMapCoef = 0.5;
const float blurCoef = 0.25;
const float lightScatteringCoef = 0.5;

const float shadowMapWidth = renderWidth * shadowMapCoef;
const float shadowMapHeight = renderHeight * shadowMapCoef;
const float shadowMapBlurWidth = shadowMapWidth * blurCoef;
const float shadowMapBlurHeight = shadowMapHeight * blurCoef;
const float lightScatterWidth = renderWidth * lightScatteringCoef;
const float lightScatterHeight = renderHeight * lightScatteringCoef;

//===SCENE DESCRIPTORS
//Camera position
vec3 p_camera(16,10,0);
//Camera lookAt
vec3 l_camera(0,0,0);
//Camera up
vec3 u_camera(0,1,0);
//Light position
vec3 p_light(10,20,0);
//Light lookAt
vec3 l_light(0,0,0);

/* scattering + shadow demo
//Camera position
vec3 p_camera(-5,10,30);
//Camera lookAt
vec3 l_camera(0,0,0);
//Camera up
vec3 u_camera(0,1,0);
//Light position
//vec3 p_light(4,30,0);
vec3 p_light(3,4,-7);
//Light lookAt
vec3 l_light(0,0,0);
*/

//===WINDOW PROPERTIES
Viewport viewport;
//background texture
GLuint bgText;
enum { DISPLAY_DEPTH_BUFFER,
       DISPLAY_DEPTH_SQUARED_HALF_BLUR_BUFFER,
       DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER };

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

//==OBJECTS
Sweep *sweep;
Vehicle *vehicle;
//for the sake of cleanliness
RenderOptions renderOpt;

//==LIGHT SCATTERING STUFF
GLuint scatterTextureId;
GLuint scatterFboId;

//==USER INTERACTION/GAMEPLAY
int lastTimeStep;
int lapStartTime;

//draw text (temporarily here until i figure out sdl
void drawString(string str, float x, float y) {
	void * font = GLUT_BITMAP_HELVETICA_10;
		glRasterPos2f(x,y);
		for (string::iterator i = (&str)->begin(); i != (&str)->end(); ++i){
			char c = *i;
			glutBitmapCharacter(font, c);
		}
	}

/* 
 * light scattering stuff. testingg!!!!!
 */

vec2 getLightScreenCoor() {
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

  gluProject(	p_light[0],
		p_light[1],
		p_light[2],
		modelView,
		projection,
		viewport,
		&winX,
		&winY,
		&winZ);
	
	
  return vec2(winX/renderWidth,  winY/renderHeight);
}


/*
 * Shadow stuff. Will probably move somewhere else.
 */
void generateShadowFBO() {
	
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
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
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

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, shadowMapWidth, shadowMapHeight, 0, GL_RGB, GL_FLOAT, 0);
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

void generateBlurFBO() {
	
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
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, shadowMapBlurWidth, shadowMapBlurHeight, 0, GL_RGB, GL_FLOAT, 0);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, blurFboIdColorTextureId, 0);
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for blur FBO, CANNOT use FBO\n");

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void generateLightFBO() {
  GLenum FBOstatus;
  glGenFramebuffersEXT(1, &scatterFboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, scatterFboId);
  
  glGenTextures(1, &scatterTextureId);
  glBindTexture(GL_TEXTURE_2D, scatterTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F_ARB, lightScatterWidth, lightScatterHeight, 0 , GL_RGB, GL_FLOAT, 0);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, scatterTextureId, 0);

  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for light FBO, CANNOT use FBO\n");

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z,float up_x,float up_y,float up_z, float zNear, float zFar, float fovy)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //gluPerspective(45,renderWidth/renderHeight,zNear,zFar);
  gluPerspective(fovy,renderWidth/renderHeight,zNear,zFar);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,up_x,up_y,up_z);
}

void setTextureMatrix() {
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



// During translation, we also have to maintain the GL_TEXTURE7, used in the shadow shader
// to determine if a vertex is in the shadow.
void pushTranslate(float x,float y,float z) {

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(x,y,z);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  glTranslatef(x,y,z);
}

void pushViewportOrientation() {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  applyMat4(viewport.orientation);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  applyMat4(viewport.orientation);
}

void pushMat4(mat4 xform) {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  applyMat4(xform);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  applyMat4(xform);
}

void pushXformd(const GLdouble* m) {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixd(m);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  glMultMatrixd(m);
}

void pushXformf(const GLfloat* m) {
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixf(m);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  glMultMatrixf(m);
}

void popTransform() {
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void blurShadowMap() {
  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  // Bluring the shadow map  horizontaly
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,blurFboId);	
  glViewport(0,0,renderWidth * shadowMapCoef *blurCoef ,renderHeight* shadowMapCoef*blurCoef);
  glUseProgramObjectARB(blurShade->getProgram());
  glUniform2fARB( blurShade->getScaleAttrib(),1.0/ (renderWidth * shadowMapCoef * blurCoef),0.0);
  //glUniform2fARB( blurShade->getScaleAttrib(),1.0/512.0,0.0);		// horiz
  //glUniform1iARB(blurShade->getTextureSourceAttrib(),0);
  glActiveTextureARB(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,colorTextureId);


  //Preparing to draw quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //Drawing quad 
  glTranslated(0,0,-5);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(-renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,0);glVertex3f(renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,1);glVertex3f(renderWidth/2,renderHeight/2,0);
  glTexCoord2d(0,1);glVertex3f(-renderWidth/2,renderHeight/2,0);
  glEnd();
  //glGenerateMipmapEXT(GL_TEXTURE_2D);
		
		 
  // Bluring vertically
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,shadowFboId);	
  //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	
  glViewport(0,0,renderWidth * shadowMapCoef ,renderHeight* shadowMapCoef);
  glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (renderHeight * shadowMapCoef ) );	
  //glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (512.0 ) );
  glBindTexture(GL_TEXTURE_2D,blurFboIdColorTextureId);
  //glBindTexture(GL_TEXTURE_2D,colorTextureId);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(-renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,0);glVertex3f(renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,1);glVertex3f(renderWidth/2,renderHeight/2,0);
  glTexCoord2d(0,1);glVertex3f(-renderWidth/2,renderHeight/2,0);
  glEnd();
		 
  glEnable(GL_CULL_FACE);
}



/*
 * Actual render stuff
 */

void drawDebugBuffer(int option) {
  glUseProgramObjectARB(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
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
  glTexCoord2d(1,0);glVertex3f(renderWidth/2,0,0);
  glTexCoord2d(1,1);glVertex3f(renderWidth/2,renderHeight/2,0);
  glTexCoord2d(0,1);glVertex3f(0,renderHeight/2,0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void drawHud() {
//fudging this...
  const float maxVelocityWidth = renderWidth * 2.5/8 /20;

  glEnable (GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgramObjectARB(0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //==Actual HUD stuff
  std::ostringstream buff;
  glPushMatrix();
  glTranslated(0,0,-5);

 
  //Velocity
  buff.str("");
  buff << "Velocity";
  glColor4f(.188235294,.474509804,1,0.9);
  drawString(buff.str(),renderWidth/8,-renderHeight*3.2/8 + 5); 

  buff.str("");
  buff << vehicle->getVelocityScalar();
  glColor4f(.9,.9,1,0.8);
  drawString(buff.str(),renderWidth/8,-renderHeight*3.2/8 - 10); 

  //Velocity Bar
  glColor4f(.188235294,.474509804,1,0.5);
  glBegin(GL_QUADS);
  glVertex3f(renderWidth/8,-renderHeight*3.7/8,0); // bottom left
  glVertex3f(renderWidth/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.7/8,0); //bottom right
  glVertex3f(renderWidth/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.2/8,0); //top right
  glVertex3f(renderWidth/8,-renderHeight*3.2/8,0); //top left
  glEnd();

  //Air Break Bar
  if(vehicle->isAirBrake()) {
    glColor4f(.901960784,.160784314,.160784314,0.5);  

    glBegin(GL_QUADS);
    glVertex3f(renderWidth/8,-renderHeight*3.8/8,0); // bottom left
    glVertex3f(renderWidth*3.5/8 ,-renderHeight*3.8/8,0); //bottom right
    glVertex3f(renderWidth*3.5/8,-renderHeight*3.7/8,0); //top right
    glVertex3f(renderWidth/8,-renderHeight*3.7/8,0); //top left
    glEnd();
  }
  

  //Lap Time
  buff.str("");
  buff << "Lap Time";
  glColor4f(1,1,1,0.75);
  drawString(buff.str(),-renderWidth*3.8/8,-renderHeight*3.3/8); 

  buff.str("");
  int msTime = glutGet(GLUT_ELAPSED_TIME) - lapStartTime;
  int sTime = msTime/1000;
  int mTime  = sTime/60;
  buff << mTime << ".";
  buff << (sTime%60) << ".";
  buff << (msTime%1000);
  glColor4f(1,1,1,0.75);
  drawString(buff.str(),-renderWidth*3.8/8,-renderHeight*3.3/8 - 10); 

  //Lap Number [TODO: LAPS]
  buff.str("");
  buff << "Lap" << vehicle->getLap();
  glColor4f(1,1,1,0.75);
  drawString(buff.str(),-renderWidth*3.8/8,renderHeight*3.5/8); 

  //Record [TODO: RECORD]
  buff.str("");
  msTime = glutGet(GLUT_ELAPSED_TIME) - lapStartTime;
  sTime = msTime/1000;
  mTime  = sTime/60;
  buff << "Lap Record:";
  buff << mTime << ".";
  buff << (sTime%60) << ".";
  buff << (msTime%1000);
  glColor4f(1,1,1,0.75);
  drawString(buff.str(),renderWidth*3.2/8,renderHeight*3.5/8);

  //Name [TODO: CENTER]
  buff.str("");
  buff << "INERTIA ALPHA";
  drawString(buff.str(), -10,renderHeight*3.8/8);

  //Energy
  buff.str("");
  buff << "Energy: ";
  buff << vehicle->getEnergy();
  drawString(buff.str(), -10,renderHeight*3.7/8);


  glPopMatrix();
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}

void drawObjects(GeometryShader * curShade) {
  
  // Ground [double for face culling]
  if(renderOpt.isDispGround()) {
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,bgText);
    glBegin(GL_QUADS);
    glTexCoord2d(0,1);glVertex3f( 50,-10,-50);
    glTexCoord2d(1,1);glVertex3f( 50,-10, 50);
    glTexCoord2d(1,0);glVertex3f(-50,-10, 50);
    glTexCoord2d(0,0);glVertex3f(-50,-10,-50);
    glEnd();
    glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex3f(-50,-10,-50);
    glTexCoord2d(1,0);glVertex3f(-50,-10, 50);
    glTexCoord2d(1,1);glVertex3f( 50,-10, 50);
    glTexCoord2d(0,1);glVertex3f( 50,-10,-50);
    glEnd();
  }


  //pushTranslate(0,0,0);
  //pushViewportOrientation();
  sweep->renderWithDisplayList(*curShade,20,0.3,20);

  vec3 vehLoc = vehicle->worldSpacePos();
  pushMat4(translation3D(vehLoc).transpose());
  
  pushMat4(vehicle->orientationBasis());
  pushMat4(scaling3D(vec3(0.2,0.2,0.2)));
  vehicle->draw(curShade);
  //glutSolidCube(1);
  popTransform();

  popTransform();
  popTransform();

//popTransform();
//popTransform();

}

void renderScene() {

  //==CAMERA

  //==FIRST RENDER: DEPTH BUFFER
  //Render from the light POV to a FBO, store depth and square depth in a 32F frameBuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,shadowFboId);
	
  //Using the depth shader to do so
  glUseProgramObjectARB(depthShade->getProgram());
  // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
  glViewport(0,0,renderWidth * shadowMapCoef,renderHeight* shadowMapCoef);
  //try to make shadow view "bigger" than normal view

  // Clear previous frame values
  glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
  setupMatrices(p_light[0],p_light[1],p_light[2],l_light[0],l_light[1],l_light[2],0,1,0,1,200,120);
	
  // Culling switching, rendering only backface, this is done to avoid self-shadowing and improve efficiency
  glCullFace(GL_FRONT);
  //draw objects using the depth shader
  drawObjects(depthShade);
	
  //cout << "0 " << glGetError() << endl;
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  //Save modelview/projection matrice into texture7, also add a biais
  setTextureMatrix();

  //==SECOND (and a half) RENDER: DOUBLE PASS GAUSSIAN BLUR
  blurShadowMap();
    

  //==THIRD RENDER: PATH TRACED LIGHT SCATTERING EFFECT (CREPUSCULAR RAYS)
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,scatterFboId);
	
  glViewport(0,0,lightScatterWidth,lightScatterHeight);
    
  // Clear previous frame values
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //Using the shadow shader
  glUseProgramObjectARB(darkShade->getProgram());
  glBindTexture(GL_TEXTURE_2D,colorTextureId);
	
  setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2],u_camera[0],u_camera[1],u_camera[2],0.5,120,70);
  
  glCullFace(GL_BACK);
   
  //Draw light
  glPushMatrix();
  glTranslatef(p_light[0],p_light[1],p_light[2]);
  glColor4f(1.0,1.0,1.0,1.0);
  glutSolidSphere(2,40,40);
  glPopMatrix();

  //Draw objects in black
  glColor4f(0.0f,0.0f,0.0f,1);
  drawObjects(darkShade);
    
  //==FOURTH RENDER: MAIN RENDER (without light scattering)
  // Now rendering from the camera POV, using the FBO to generate shadows
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	
  glViewport(0,0,renderWidth,renderHeight);
    
  // Clear previous frame values
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //Using the shadow shader
  glUseProgramObjectARB(shade->getProgram());
  glUniform1iARB(shade->getShadowMapAttrib(),7);
  glActiveTextureARB(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D,colorTextureId);

  //declared in third pass
  //setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2],u_camera[0],u_camera[1],u_camera[2],1,120);
  
  //okay seriously, why do we have vec and float[] is required by openGL -_-
  float tempLight[4] = {p_light[0], p_light[1], p_light[2], 1};
  glLightfv(GL_LIGHT0, GL_POSITION, tempLight);
  
  glCullFace(GL_BACK);
  //draw objects using our shadow shader
  drawObjects(shade);
    
    
  //==FIFTH PASS: LIGHT SCATTERING OVERLAY
  //uses main screen
  //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
  //glViewport(0,0,renderWidth,renderHeight);
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
  glOrtho(-renderWidth/2,renderWidth/2,-renderHeight/2,renderHeight/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1.0f,1.0f,1.0f,1); //rectangle to display texture
  glPushMatrix();
  glTranslated(0,0,-5);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(-renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,0);glVertex3f(renderWidth/2,-renderHeight/2,0);
  glTexCoord2d(1,1);glVertex3f(renderWidth/2,renderHeight/2,0);
  glTexCoord2d(0,1);glVertex3f(-renderWidth/2,renderHeight/2,0);
  glEnd();
  glPopMatrix();
  glDisable(GL_BLEND);
  
  drawHud();
  
  if(renderOpt.isDepthBuffer())
    drawDebugBuffer(renderOpt.getDepthBufferOption());

  glutSwapBuffers();
    
}

/*
 * Interface Stuff
 */
void processNormalKeys(unsigned char key, int x, int y) {
  switch (key) {
  case 'Q':
  case 'q':
  case 27:	
    exit(0);
    break;
  case 'B':
  case 'b':
    renderOpt.toggleDispDepthBuffer();
    break;
  case 'G':
  case 'g':
    renderOpt.toggleDispGround();
    break;
  case ' ':
    vehicle->setAirBrake(0.00008);
    break;
  }
}

void processNormalKeysUp(unsigned char key, int x, int y) {
  switch(key) {
  case ' ':
    vehicle->setAirBrake(0);
    break;
  }
}

void processSpecialKeys(int key, int x, int y) {
  switch(key) {
    case GLUT_KEY_UP:
      vehicle->setAccel(0.2);
      break;
    case GLUT_KEY_DOWN:
      vehicle->setAccel(-0.1);
      break;
    case GLUT_KEY_LEFT:
      vehicle->turnLeft(3);
      break;
    case GLUT_KEY_RIGHT:
      vehicle->turnRight(3);
      break;
  }
}

void processSpecialKeysUp(int key, int x, int y) {
  switch(key) {
    case GLUT_KEY_UP:
    case GLUT_KEY_DOWN:
      vehicle->setAccel(0.0);
      break;
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
      vehicle->turnLeft(0.0);
      break;
  }
}
		

void myActiveMotionFunc(int x, int y) {

  // Rotate viewport orientation proportional to mouse motion
  vec2 newMouse = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));
  vec2 diff = (newMouse - viewport.mousePos);
  double len = diff.length();
  if (len > .001) {
    vec3 axis = vec3(diff[1]/len, diff[0]/len, 0);
    viewport.orientation = viewport.orientation * rotation3D(axis, -180 * len);
  }

  //Record the mouse location for drawing crosshairs
  viewport.mousePos = newMouse;

  //Force a redraw of the window.
  glutPostRedisplay();
}
void myPassiveMotionFunc(int x, int y) {
  //Record the mouse location for drawing crosshairs
  viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

  //Force a redraw of the window.
  glutPostRedisplay();
}

void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
  //cout << (buttonMask) << endl;
  //cout << (buttonMask & 16384) << endl;
  if(buttonMask & 16384) { //button 14: X on DualShock3
    vehicle->setAccel(0.1);
  } else if(buttonMask & 8192) { //button 13: O on DualShock3
    vehicle->setAccel(-0.1);
  } else {
    vehicle->setAccel(0.0);
  }
  if(buttonMask & 256 && buttonMask & 512) { //256:L2, 512: R2
    vehicle->setAirBrake(0.0001);
  } else if(buttonMask & 256) { //TODO: left and right airbrake
    vehicle->setAirBrake(0.00005);
  } else if(buttonMask & 512) {
    vehicle->setAirBrake(0.00005);
  } else {
    vehicle->setAirBrake(0.0);
  }
	//cout << x << endl;
  vehicle->turnRight(x/500);
}

void stepVehicle(int x) {
	//call at beginning for consistency..
	int newTime =glutGet(GLUT_ELAPSED_TIME);
	int timeDif = newTime - lastTimeStep;
    vehicle->step(0.01 * timeDif/10.0);


    p_camera = vehicle->cameraPos();
    l_camera = vehicle->cameraLookAt();
    u_camera = vehicle->getUp();
    l_light = vehicle->worldSpacePos();

	//redo this every 10ms
	lastTimeStep = newTime;
  glutTimerFunc(10,stepVehicle, 0);

}

/*
 * Main
 */
int main(int argc,char** argv) {
  //Initialize OpenGL
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
  viewport.w = 1024;
  viewport.h = 768;
  glutInitWindowSize(viewport.w,viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow("Inertia 0.0000001");


  FreeImage_Initialise();

  // set some lights
  {
    float ambient[4] = { .1f, .1f, .1f, 1.f };
    float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
    float pos[4] = { p_light[0], p_light[1], p_light[2], 0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_LIGHT0);
  }

  glEnable(GL_DEPTH_TEST);
  glClearColor(0,0,0,1.0f);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	
  glutDisplayFunc(renderScene);
  glutIdleFunc(renderScene);
  glutKeyboardFunc(processNormalKeys);
  glutKeyboardUpFunc(processNormalKeysUp);
  glutMotionFunc(myActiveMotionFunc);
  glutPassiveMotionFunc(myPassiveMotionFunc);

  glutSpecialFunc(processSpecialKeys);
  glutSpecialUpFunc(processSpecialKeysUp);
  glutJoystickFunc(joystickFunc,10);


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

  sweep = new Sweep(argv[1]);
  
  //load the default render options
  renderOpt = RenderOptions();
  //but for the sake of coolness we'll display the blurred color depth buffer by default
  renderOpt.setDepthBufferOption(DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER);
  
  //hacky quicky way to load a background texture
  string bg = "lantern1.png";
  if(argc > 2)
    bg = argv[2];
  loadTexture(bg,bgText);

  vehicle = new Vehicle(sweep);
  vehicle->mesh->loadFile("test.obj");
  vehicle->mesh->loadTextures("unnamed_object1_auv.bmp","unnamed_object1_auv.bmp");
  vehicle->mesh->centerAndScale(4);

  //Lap time 
  lapStartTime = glutGet(GLUT_ELAPSED_TIME);

  //Step Vehicle once (and it will recurse on timer)
  lastTimeStep = glutGet(GLUT_ELAPSED_TIME);
  stepVehicle(0);

  //And Go!
  glutMainLoop();
}
