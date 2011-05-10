#include "main.h"

// Buffers hold sound data.
ALuint noiseBuffer;
ALuint musicBuffer;
ALuint noiseBuffer2;
ALuint musicBuffer2;

// Sources are points emitting sound.
ALuint noiseSource;
ALuint musicSource;
ALuint noiseSource2;
ALuint musicSource2;

ALuint currentMusic;
ALuint currentNoise;

// Position of the source sound.
ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };

// Velocity of the source sound.
ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };

// Position of the listener.
ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };

// Velocity of the listener.
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };

// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };

//===RENDER CONSTANTS
const float shadowMapCoef = 1.0;
const float blurCoef = 0.5;
const float lightScatteringCoef = 1.0;

const float renderWidth = 1024.0;
const float renderHeight = 768.0;
const float shadowMapWidth = renderWidth * shadowMapCoef;
const float shadowMapHeight = renderHeight * shadowMapCoef;
const float shadowMapBlurWidth = shadowMapWidth * blurCoef;
const float shadowMapBlurHeight = shadowMapHeight * blurCoef;
const float lightScatterWidth = renderWidth * lightScatteringCoef;
const float lightScatterHeight = renderHeight * lightScatteringCoef;

//===WINDOW PROPERTIES
Viewport viewport;
FTGLTextureFont * evolutionFont;
FTGLTextureFont * digitalNinjaFont;
FTGLTextureFont * accidentalPresidencyFont;
FTBufferFont * evolutionBufferFont;
FTBufferFont * accidentalPresidencyBufferFont;

//===SCENE DESCRIPTORS
//Camera position
vec3 p_camera(16,10,0);
//Camera lookAt
vec3 l_camera(0,0,0);
//Camera up
vec3 u_camera(0,1,0);
//Light position
vec3 p_light(110,60,0);
//Light lookAt
vec3 l_light(0,0,0);                                                                                                                
//===DEBUG STUFF 
//background texture
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
int gameMode;
void (*drawObjectTarget)(GeometryShader *);
void (*drawOverlayTarget)();
enum { MODE_RACE, MODE_TRACK_SELECT, MODE_TITLE };


ALboolean LoadALData()
{
    // Variables to load into.

    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;
    // Load wav data into a buffer.
    alGenBuffers(1, &musicBuffer);
    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alutLoadWAVFile("race2.wav", &format, &data, &size, &freq, &loop);
    alBufferData(musicBuffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    // Bind buffer with a source.
    alGenSources(1, &musicSource);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alSourcei (musicSource, AL_BUFFER,   musicBuffer   );
    alSourcef (musicSource, AL_PITCH,    1.0f     );
    alSourcef (musicSource, AL_GAIN,     1.0f     );
    alSourcefv(musicSource, AL_POSITION, SourcePos);
    alSourcefv(musicSource, AL_VELOCITY, SourceVel);
    alSourcei (musicSource, AL_LOOPING,  AL_TRUE     );
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;

    return AL_FALSE;
}

ALboolean LoadALData2()
{
    // Variables to load into.

    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;
    // Load wav data into a buffer.
    alGenBuffers(1, &noiseBuffer);
    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alutLoadWAVFile("Engine.wav", &format, &data, &size, &freq, &loop);
    alBufferData(noiseBuffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    // Bind buffer with a source.
    alGenSources(1, &noiseSource);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alSourcei (noiseSource, AL_BUFFER,   noiseBuffer   );
    cout << alGetString(alGetError()) << endl;
    alSourcef (noiseSource, AL_PITCH,    1.0f     );
    alSourcef (noiseSource, AL_GAIN,     1.0f     );
    alSourcefv(noiseSource, AL_POSITION, SourcePos);
    alSourcefv(noiseSource, AL_VELOCITY, SourceVel);
    alSourcei (noiseSource, AL_LOOPING,  AL_TRUE     );
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;

    return AL_FALSE;
}

ALboolean LoadALData3()
{
    // Variables to load into.

    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;
    // Load wav data into a buffer.
    alGenBuffers(1, &musicBuffer2);
    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alutLoadWAVFile("pacman.wav", &format, &data, &size, &freq, &loop);
    alBufferData(musicBuffer2, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    // Bind buffer with a source.
    alGenSources(1, &musicSource2);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alSourcei (musicSource2, AL_BUFFER,   musicBuffer2   );
    alSourcef (musicSource2, AL_PITCH,    1.0f     );
    alSourcef (musicSource2, AL_GAIN,     1.0f     );
    alSourcefv(musicSource2, AL_POSITION, SourcePos);
    alSourcefv(musicSource2, AL_VELOCITY, SourceVel);
    alSourcei (musicSource2, AL_LOOPING,  AL_FALSE     );
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;

    return AL_FALSE;
}

ALboolean LoadALData4()
{
    // Variables to load into.

    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;
    // Load wav data into a buffer.
    alGenBuffers(1, &noiseBuffer2);
    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alutLoadWAVFile("wakka.wav", &format, &data, &size, &freq, &loop);
    alBufferData(noiseBuffer2, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    // Bind buffer with a source.
    alGenSources(1, &noiseSource2);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alSourcei (noiseSource2, AL_BUFFER,   noiseBuffer2   );
    alSourcef (noiseSource2, AL_PITCH,    1.0f     );
    alSourcef (noiseSource2, AL_GAIN,     1.0f     );
    alSourcefv(noiseSource2, AL_POSITION, SourcePos);
    alSourcefv(noiseSource2, AL_VELOCITY, SourceVel);
    alSourcei (noiseSource2, AL_LOOPING,  AL_TRUE     );
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;

    return AL_FALSE;
}

void SetListenerValues()
{
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);
}

void KillALData()
{
    alDeleteBuffers(1, &noiseBuffer);
    alDeleteSources(1, &noiseSource);
    alDeleteBuffers(1, &musicBuffer);
    alDeleteSources(1, &musicSource);
	alDeleteSources(1, &musicSource2);
	alDeleteBuffers(1, &musicBuffer2);
	alDeleteSources(1, &noiseSource2);
	alDeleteBuffers(1, &noiseBuffer2);
    alutExit();
}

//Draw Text
void drawString(FTFont *font, string str, float x, float y) {
  FTPoint pos(x,y);
  font->Render(str.c_str(), -1, pos);
}

void setMode(int mode);

/*
 * Light Scattering stuff.
 */

//Grab the screen space location of the light
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

  gluProject(p_light[0], p_light[1], p_light[2],
             modelView, projection, viewport,
            &winX, &winY, &winZ);
    
  return vec2(winX/renderWidth,  winY/renderHeight);
}

//Generate the FBO the light scatter shader will write to
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



/*
 * Shadow stuff. Will probably move somewhere else.
 */
//Generate the FBO the depth shader will write to
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

//Generate the FBO the gaussian blur shader will write to
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

//Take the depth texture and blur it.
void blurShadowMap() {
  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  // Bluring the shadow map  horizontaly
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,blurFboId);	
  glViewport(0,0,renderWidth * shadowMapCoef *blurCoef ,renderHeight* shadowMapCoef*blurCoef);
  glUseProgramObjectARB(blurShade->getProgram());
  //glUniform2fARB( blurShade->getScaleAttrib(),1.0/ (renderWidth * shadowMapCoef * blurCoef),0.0);
  glUniform2fARB( blurShade->getScaleAttrib(),1.0/512.0,0.0);		// horiz
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
  //glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (renderHeight * shadowMapCoef ) );	
  glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (512.0 ) );
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

// Use these instead of pushMatrix() and popMatrix(). These maintain the orientations of the depth buffer in transformed objects.
void pushTranslate(float x,float y,float z) {

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(x,y,z);
    
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);

  glPushMatrix();
  glTranslatef(x,y,z);
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



//Render stuff
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
void renderScene() {
  //==FIRST RENDER: DEPTH BUFFER
  //Render from the light POV to a FBO, store depth and square depth in a 32F frameBuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,shadowFboId);
    
  //Using the depth shader to do so
  glUseProgramObjectARB(depthShade->getProgram());
  // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
  glViewport(0,0,renderWidth * shadowMapCoef,renderHeight* shadowMapCoef);
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
    
  glViewport(0,0,lightScatterWidth,lightScatterHeight);
    
  // Clear previous frame values
  glClearColor(0,0,0,1.0f);
  //glClearColor(1,1,1,1.0f);
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
  glutSolidSphere(30,20,20);
  glPopMatrix();

  //Draw objects in black
  glColor4f(0.0f,0.0f,0.0f,1);
  //drawObjects(darkShade);
  drawObjectTarget(darkShade);

    
  //==FOURTH RENDER: MAIN RENDER (without light scattering)
  // Now rendering from the camera POV, using the FBO to generate shadows
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
    
  glViewport(0,0,renderWidth,renderHeight);
    
  // Clear previous frame values
  glClearColor(.764705882,.890196078,1,1.0f);
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
  
  drawOverlayTarget();

  /*
  
  if(renderOpt.isDepthBuffer())
    drawDebugBuffer(renderOpt.getDepthBufferOption());
*/

  glutSwapBuffers();    
}





/*
 * Race Render Scene
 */

namespace raceScene {
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
    glTranslated(0,0,-5);

    //backdrops 
    
    //header
    glColor4f(.250980392,.458823529,.631372549,0.95);
    glBegin(GL_QUADS);
    glVertex3f(-renderWidth/2,renderHeight * 3.8/8,0);
    glVertex3f(renderWidth/2,renderHeight * 3.8/8,0);
    glVertex3f(renderWidth/2,renderHeight/2,0);
    glVertex3f(-renderWidth/2,renderHeight/2,0);

    glColor4f(.250980392,.458823529,.631372549,0.1);

    //Lap
    glVertex3f(-renderWidth/2,renderHeight * 3.0/8,0);
    glVertex3f(-renderWidth * 3.4/8,renderHeight * 3.0/8,0);
    glVertex3f(-renderWidth * 3.4/8,renderHeight * 3.8/8,0);
    glVertex3f(-renderWidth/2,renderHeight * 3.8/8,0);

    //Record (with record to have less branches)
    /*
    glBegin(GL_QUADS);
    glVertex3f(renderWidth * 2.2/8,renderHeight * 3.0/8,0);
    glVertex3f(renderWidth/2,renderHeight * 3.0/8,0);
    glVertex3f(renderWidth/2,renderHeight * 3.8/8,0);
    glVertex3f(renderWidth * 2.2/8,renderHeight * 3.8/8,0);
    glEnd(); */
    
    
    //Energy
    glVertex3f(-65,renderHeight * 3.0/8,0);
    glVertex3f(65,renderHeight * 3.0/8,0);
    glVertex3f(65,renderHeight * 3.8/8,0);
    glVertex3f(-65,renderHeight * 3.8/8,0);

    //Lap Time
    glVertex3f(-renderWidth/2, -renderHeight*4.0/8,0);
    glVertex3f(-renderWidth*2.5/8, -renderHeight*4.0/8,0);
    glVertex3f(-renderWidth*2.5/8, -renderHeight*3.0/8,0);
    glVertex3f(-renderWidth/2, -renderHeight*3.0/8,0);

    //Velocity
    glVertex3f(renderWidth*2/8, -renderHeight*4.0/8,0);
    glVertex3f(renderWidth/2, -renderHeight*4.0/8,0);
    glVertex3f(renderWidth/2, -renderHeight*3.0/8,0);
    glVertex3f(renderWidth*2/8, -renderHeight*3.0/8,0);
    glEnd();

    //Actual stuff

    //Velocity
    buff.str("");
    buff << "Velocity";
    glColor4f(.188235294,.474509804,1,0.9);
    drawString(evolutionBufferFont, buff.str(),renderWidth*2.3/8,-renderHeight*3.4/8 + 5); 

    //Velocity Bar
    glColor4f(.188235294,.474509804,1,0.5);
    glBegin(GL_QUADS);
    glVertex3f(renderWidth*2.1/8,-renderHeight*3.8/8,0); // bottom left
    glVertex3f(renderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.8/8,0); //bottom right
    glVertex3f(renderWidth*2.1/8 + vehicle->getVelocityScalar() * maxVelocityWidth,-renderHeight*3.4/8,0); //top right
    glVertex3f(renderWidth*2.1/8,-renderHeight*3.4/8,0); //top left
    glEnd();

    //Velocity Text
    buff.str("");
    buff << vehicle->getVelocityScalar();
    glColor4f(.9,.9,1,0.8);
    drawString(digitalNinjaFont, buff.str(),renderWidth*2.3/8,-renderHeight*3.3/8 - 44); 

    //Air Break Bar
    if(vehicle->isAirBrake()) {

      glColor4f(.901960784,.160784314,.160784314,0.5);  

      glBegin(GL_QUADS);
      glVertex3f(renderWidth*2.1/8,-renderHeight*3.9/8,0); // bottom left
      glVertex3f(renderWidth*3.8/8 ,-renderHeight*3.9/8,0); //bottom right
      glVertex3f(renderWidth*3.8/8,-renderHeight*3.8/8,0); //top right
      glVertex3f(renderWidth*2.1/8,-renderHeight*3.8/8,0); //top left
      glEnd();
    } 

    //Lap Time
    buff.str("");
    buff << "Lap Time";
    glColor4f(1,1,1,0.75);
    drawString(evolutionBufferFont, buff.str(),-renderWidth*3.8/8,-renderHeight*3.4/8); 

    buff.str("");
    int msTime = glutGet(GLUT_ELAPSED_TIME) - vehicle->getLapStartTime();
    int sTime = msTime/1000;
    int mTime  = sTime/60;
    buff << mTime << ".";
    buff << (sTime%60) << ".";
    buff << (msTime%1000);
    glColor4f(1,1,1,0.75);
    drawString(digitalNinjaFont, buff.str(),-renderWidth*3.8/8,-renderHeight*3.7/8); 

    //Lap Number
    buff.str("");
    buff << "Lap"; // << vehicle->getLap();
    glColor4f(1,1,1,0.75);
    drawString(evolutionBufferFont, buff.str(),-renderWidth*3.9/8,renderHeight*3.5/8); 

    buff.str("");
    buff << vehicle->getLap();
    drawString(digitalNinjaFont, buff.str(), -renderWidth * 3.9/8, renderHeight*3.2/8);

    //Record
    buff.str("");
    msTime = vehicle->getBestLapTime();
    if(msTime != -1) {
      //backdrop
      glColor4f(.250980392,.458823529,.631372549,0.3);
      glBegin(GL_QUADS);
      glVertex3f(renderWidth * 2.2/8,renderHeight * 3.0/8,0);
      glVertex3f(renderWidth*2/2,renderHeight * 3.0/8,0);
      glVertex3f(renderWidth*2/2,renderHeight * 3.8/8,0);
      glVertex3f(renderWidth * 2.2/8,renderHeight * 3.8/8,0);
      glEnd();

      sTime = msTime/1000;
      mTime  = sTime/60;
      buff << "Lap Record:";
      drawString(evolutionBufferFont, buff.str(),renderWidth*2.4/8, renderHeight*3.5/8);

      buff.str("");
      buff << mTime << ".";
      buff << (sTime%60) << ".";
      buff << (msTime%1000);
      glColor4f(1,1,1,0.75);
      drawString(digitalNinjaFont, buff.str(),renderWidth*2.4/8, renderHeight*3.2/8);

    }

    //Name 
    buff.str("");
    buff << "cs184sp11 final project: inertia. pre-submission version. brandon wang, andrew lee, chris tandiono";
    drawString(accidentalPresidencyBufferFont, buff.str(), -renderWidth*3.9/8, renderHeight*3.85/8);

    //Energy
    buff.str("");
    buff << "Energy: ";
    //buff << vehicle->getEnergy();
    drawString(evolutionBufferFont, buff.str(), -55,renderHeight*3.5/8);

    buff.str("");
    buff << vehicle->getEnergy();
    drawString(digitalNinjaFont, buff.str(), -45,renderHeight*3.2/8);


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

  void drawObjects(GeometryShader * curShade) {
    //Track/City
    sweep->renderWithDisplayList(*curShade,20,0.3,20);

    //Vehicle Location
    vec3 vehLoc = vehicle->worldSpacePos();

    //Put matrices together for smaller stack size
    mat4 transformation = vehicle->orientationBasis() *
                          translation3D(vehLoc).transpose();

    pushMat4(transformation);
    vehicle->draw(curShade); 
    popTransform();
  }

  void processNormalKeys(unsigned char key, int x, int y) {
    switch (key) {
    case 'Q':
    case 'q':
    case 27:	
      setMode(MODE_TITLE);
      break;
    case 'B':
    case 'b':
      renderOpt.toggleDispDepthBuffer();
      break;
    case 'G':
    case 'g':
      renderOpt.toggleDispGround();
      break;
    case '1':
      alSourceStop(currentMusic);
      alSourceStop(currentNoise);
      currentMusic = musicSource;
      currentNoise = noiseSource;
      alSourcePlay(currentMusic);
	    break;
    case '2':
      alSourceStop(currentMusic);
      alSourceStop(currentNoise);
      currentMusic = musicSource2;
      currentNoise = noiseSource2;
      alSourcePlay(currentMusic);
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

    ALint sourceState;
    switch(key) {
      case GLUT_KEY_UP:
        alGetSourcei(currentNoise, AL_SOURCE_STATE, &sourceState);
        if (sourceState != AL_PLAYING) {
          alSourcePlay(currentNoise);
        }
        vehicle->setAccel(0.2);
        break;
      case GLUT_KEY_DOWN:
        alGetSourcei(currentNoise, AL_SOURCE_STATE, &sourceState);
        if (sourceState != AL_PLAYING) {
          alSourcePlay(currentNoise);
        }
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

  void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
    //cout << (buttonMask) << endl;
    //cout << (buttonMask & 16384) << endl;
    if(buttonMask & 16384) { //button 14: X on DualShock3
      vehicle->setAccel(0.2);
      alSourcePlay(currentNoise);
    } else if(buttonMask & 8192) { //button 13: O on DualShock3
      vehicle->setAccel(-0.1);
      alSourcePlay(currentNoise);
    } else {
      alSourceStop(currentNoise);
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


  void processSpecialKeysUp(int key, int x, int y) {
    switch(key) {
      case GLUT_KEY_UP:
      case GLUT_KEY_DOWN:
        alSourceStop(currentNoise);
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

}







//======TITLE SCENE
//sorry about the code mess but its glut's fault and we dont have enough time to make something complex to make it look nice
namespace titleScene {

  void drawObjects(GeometryShader * curShade) {
    Mesh * veh = vehicle->mesh;

    vec3 rotAxis(0.3,0.9,0);

    mat4 transformation = rotation3D(rotAxis, glutGet(GLUT_ELAPSED_TIME)/200.0);
    pushMat4(transformation);
    veh->draw(*curShade); 
    popTransform();
  }


  void drawTitleOverlay() {
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
    glTranslated(0,0,-5);

    glColor4f(1,1,1,0.75);
    glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex3f(-20,-renderHeight/6,0);
    glTexCoord2d(1,0);glVertex3f(renderWidth*0.45,-renderHeight/6,0);
    glTexCoord2d(1,1);glVertex3f(renderWidth*0.45,renderHeight/6,0);
    glTexCoord2d(0,1);glVertex3f(-20,renderHeight/6,0);
    glEnd();

    buff.str("");
    buff << "INERTIA";
    glColor4f(.188235294,.474509804,1,0.9);
   
    
    evolutionFont->FaceSize(150);
    drawString(evolutionFont, buff.str(),0,0); 
    evolutionFont->FaceSize(36);
    
    accidentalPresidencyFont->FaceSize(36);
    buff.str("");
    buff << "CS184Sp11 Final Project";
    drawString(accidentalPresidencyFont, buff.str(),0,-50); 
    accidentalPresidencyFont->FaceSize(20);
    buff.str("");
    buff << "Brandon Wang, Andrew Lee, Chris Tandiono";
    drawString(accidentalPresidencyFont, buff.str(),0,-75); 





    //Name 
    buff.str("");
    buff << "cs184sp11 final project: inertia. pre-submission version. brandon wang, andrew lee, chris tandiono";
    drawString(accidentalPresidencyBufferFont, buff.str(), -renderWidth*3.9/8, renderHeight*3.85/8);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }

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
    case '1':
      alSourceStop(currentMusic);
      alSourceStop(currentNoise);
      currentMusic = musicSource;
      currentNoise = noiseSource;
      alSourcePlay(currentMusic);
	  break;
    case '2':
      alSourceStop(currentMusic);
      alSourceStop(currentNoise);
      currentMusic = musicSource2;
      currentNoise = noiseSource2;
      alSourcePlay(currentMusic);
      break;

      //temp keys for debugging
    case ' ':
      setMode(MODE_RACE);
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
    return;
  }

  void joystickFunc(unsigned int buttonMask, int x, int y, int z) {
    //cout << (buttonMask) << endl;
    //cout << (buttonMask & 16384) << endl;
    if(buttonMask & 16384) { //button 14: X on DualShock3
    } else if(buttonMask & 8192) { //button 13: O on DualShock3
    } else {
    }
    if(buttonMask & 256 && buttonMask & 512) { //256:L2, 512: R2
    } else if(buttonMask & 256) { //TODO: left and right airbrake
    } else if(buttonMask & 512) {
    } else {  
    }
      //cout << x << endl
  }


  void processSpecialKeysUp(int key, int x, int y) {
    switch(key) {
      case GLUT_KEY_UP:
      case GLUT_KEY_DOWN:
        break;
      case GLUT_KEY_LEFT:
      case GLUT_KEY_RIGHT:
        break;
    }
  }
          

  void activeMotionFunc(int x, int y) {

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

  void passiveMotionFunc(int x, int y) {
    //Record the mouse location for drawing crosshairs
    viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

    //Force a redraw of the window.
    glutPostRedisplay();
  }
}




//Steps a vehicle 
//because of glut's callback limitation in the fact that we can't interrupt, we have to keep polling if we're in the right mode........
void stepVehicle(int x) {
  //call at beginning for consistency..
  int newTime =glutGet(GLUT_ELAPSED_TIME);
  if(gameMode == MODE_RACE) {
    int timeDif = newTime - lastTimeStep;
    vehicle->step(0.01 * timeDif/10.0);
  
    p_camera = vehicle->cameraPos();
    l_camera = vehicle->cameraLookAt();
    u_camera = vehicle->getUp();

    //p_light = vehicle->lightPos();
    l_light = vehicle->worldSpacePos();
  }
  //redo this every 20ms
  lastTimeStep = newTime;
  glutTimerFunc(20,stepVehicle, 0);

}

void setMode(int newMode) {
  switch(newMode) {
    case MODE_RACE:
        gameMode = MODE_RACE;
        vehicle->mesh->centerAndScale(0.8);
        drawObjectTarget = raceScene::drawObjects;
        drawOverlayTarget = raceScene::drawHud;
        glutKeyboardFunc(raceScene::processNormalKeys);
        glutKeyboardUpFunc(raceScene::processNormalKeysUp);
        glutMotionFunc(raceScene::myActiveMotionFunc);
        glutPassiveMotionFunc(raceScene::myPassiveMotionFunc);

        glutSpecialFunc(raceScene::processSpecialKeys);
        glutSpecialUpFunc(raceScene::processSpecialKeysUp);
        glutJoystickFunc(raceScene::joystickFunc,10);
            
        //Lap time 
        vehicle->setLapStartTime(glutGet(GLUT_ELAPSED_TIME));

        //Step Vehicle once (and it will recurse on timer)
        lastTimeStep = glutGet(GLUT_ELAPSED_TIME);
        stepVehicle(0);


        break;

    case MODE_TITLE:
        gameMode = MODE_TITLE;

        alSourceStop(currentNoise);

        vehicle->mesh->centerAndScale(40);
        drawObjectTarget = titleScene::drawObjects;
        drawOverlayTarget = titleScene::drawTitleOverlay;

        glutKeyboardFunc(titleScene::processNormalKeys);
        glutKeyboardUpFunc(titleScene::processNormalKeysUp);
        glutMotionFunc(titleScene::activeMotionFunc);
        glutPassiveMotionFunc(titleScene::passiveMotionFunc);

        glutSpecialFunc(titleScene::processSpecialKeys);
        glutSpecialUpFunc(titleScene::processSpecialKeysUp);
        glutJoystickFunc(titleScene::joystickFunc,10);

        p_camera = vec3(0,10,16);
        l_camera = vec3(0,0,0);
        u_camera = vec3(0,1,0);

        p_light = vec3(0,30,6);
        l_light = vec3(0,0,0);
        break;
  }
}


/*
 * Main
 */
int main(int argc,char** argv) {
  srand(time(NULL));

  //Initialize OpenGL
  glutInit(&argc, argv);

  //Initialize OpenAL
  alutInit(&argc, argv);
  alGetError(); // zero-out the error status
  // Load the wav data.
  if (LoadALData() == AL_FALSE)
    return -1;
  SetListenerValues();
  if (LoadALData2() == AL_FALSE)
    return -2;
  SetListenerValues();
  if (LoadALData3() == AL_FALSE)
    return -3;
  SetListenerValues();
  if (LoadALData4() == AL_FALSE)
    return -4;
  SetListenerValues();

  // Setup an exit procedure.
  atexit(KillALData);


  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
  viewport.w = 1024;
  viewport.h = 768;
  glutInitWindowSize(viewport.w,viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow("Inertia Alpha");


  FreeImage_Initialise();

  // set some lights
  {
    float ambient[4] = { .5f, .5f, .5f, 1.f };
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

  glutDisplayFunc(renderScene);
  glutIdleFunc(renderScene);  
        

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

  // Set the fonts size and render a small text.
  evolutionFont = new FTGLTextureFont("Evolution.ttf");
  digitalNinjaFont = new FTGLTextureFont("DigitalNinja.ttf");
  accidentalPresidencyFont = new FTGLTextureFont("AccidentalPresidency.ttf");
  evolutionBufferFont = new FTBufferFont("Evolution.ttf");
  accidentalPresidencyBufferFont = new FTBufferFont("AccidentalPresidency.ttf");

  evolutionFont->FaceSize(36);
  evolutionBufferFont->FaceSize(36);
  digitalNinjaFont->FaceSize(27);
  accidentalPresidencyFont->FaceSize(16);
  accidentalPresidencyBufferFont->FaceSize(16);

  
  //// If something went wrong, bail out.
  if(evolutionFont->Error() || digitalNinjaFont->Error() || accidentalPresidencyFont->Error() ||
      evolutionBufferFont->Error() || accidentalPresidencyBufferFont->Error())
          return -1;



  sweep = new Sweep();
  
  //load the default render options
  renderOpt = RenderOptions();
  //but for the sake of coolness we'll display the blurred color depth buffer by default
  renderOpt.setDepthBufferOption(DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER);
  renderOpt.setDepthBufferOption(DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER);

  

  vehicle = new Vehicle(sweep);
  vehicle->mesh->loadFile("test.obj");
  vehicle->mesh->loadTextures("test.png","test.png");

    setMode(MODE_TITLE);

  //And Go!
  currentMusic = musicSource;
  currentNoise = noiseSource;
  alSourcePlay(currentMusic);
  glutMainLoop();
}

