#include "main.h"
#include "functions.h"

// Constants (some issues with aspect ratio; and i think defines will speed some stuff up. keep it?)
#define RENDER_WIDTH 1024.0
#define RENDER_HEIGHT 768.0
#define SHADOW_MAP_COEF 0.5
#define BLUR_COEF 1

//===SCENE DESCRIPTORS
//Camera position
vec3 p_camera(16,20,0);
//Camera lookAt
vec3 l_camera(0,0,-5);
//Light position
vec3 p_light(4,25,0);
//Light lookAt
vec3 l_light(0,0,-5);

//===WINDOW PROPERTIES
Viewport viewport;
//background texture
GLuint bgText;
enum { DISPLAY_DEPTH_BUFFER,
       DISPLAY_DEPTH_SQUARED_HALF_BLUR_BUFFER,
       DISPLAY_DEPTH_SQUARED_COMPLETE_BUFFER };

//===SHADOW STUFF
// Hold id of the framebuffer for light POV rendering
GLuint fboId;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint depthTextureId;
GLuint colorTextureId;
// Bluring FBO
GLuint blurFboId;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint blurFboIdColorTextureId;

//===SHADERS
ShadowShader* shade;
BlurShader *blurShade;
GeometryShader *depthShade;

//==OBJECTS
Sweep *sweep;
Vehicle *vehicle;
//for the sake of cleanliness
RenderOptions renderOpt;
int frameCount;

/*
 * Shadow stuff. Will probably move somewhere else.
 */
void generateShadowFBO() {
  int shadowMapWidth = RENDER_WIDTH * SHADOW_MAP_COEF;
  int shadowMapHeight = RENDER_HEIGHT * SHADOW_MAP_COEF;
	
  //GLfloat borderColor[4] = {0,0,0,0};
	
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	
  //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	
  // Remove artefact on the edges of the shadowmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, shadowMapWidth, shadowMapHeight, 0, GL_RGB, GL_FLOAT, 0);
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  // create a framebuffer object
  glGenFramebuffersEXT(1, &fboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
	
  // attach the texture to FBO depth attachment point
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT ,GL_TEXTURE_2D, depthTextureId, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, colorTextureId, 0);

  // check FBO status
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for shadowmap FBO, CANNOT use FBO\n");
	
  // switch back to window-system-provided framebuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  // Creating the blur FBO
  glGenFramebuffersEXT(1, &blurFboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, blurFboId);

  glGenTextures(1,&blurFboIdColorTextureId);
  glBindTexture(GL_TEXTURE_2D, blurFboIdColorTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB16F_ARB, shadowMapWidth*BLUR_COEF, shadowMapHeight*BLUR_COEF, 0, GL_RGB, GL_FLOAT, 0);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, blurFboIdColorTextureId, 0);
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed for blur FBO, CANNOT use FBO\n");

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45,RENDER_WIDTH/RENDER_HEIGHT,10,120);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,0,1,0);
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
void startTranslate(float x,float y,float z) {
  glPushMatrix();

  glTranslatef(x,y,z);
  applyMat4(viewport.orientation);
	
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE7);
  glPushMatrix();

  glTranslatef(x,y,z);
  applyMat4(viewport.orientation);
}

void endTranslate() {
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void startTeapotMove(mat4 whack) {
  glPushMatrix();

  glTranslatef(0,0,-5);
  applyMat4(whack.transpose());
  applyMat4(viewport.orientation);

  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE8);
  glPushMatrix();
  glTranslatef(0,0,-5);
  applyMat4(whack.transpose());
  applyMat4(viewport.orientation);
}


void blurShadowMap() {
  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  // Bluring the shadow map  horinzontaly
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,blurFboId);
  //	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	
  glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_COEF *BLUR_COEF ,RENDER_HEIGHT* SHADOW_MAP_COEF*BLUR_COEF);
  glUseProgramObjectARB(blurShade->getProgram());
  //glUniform2fARB( blurShade->getScaleAttrib(),1.0/ (RENDER_WIDTH * SHADOW_MAP_COEF * BLUR_COEF),0.0);
  glUniform2fARB( blurShade->getScaleAttrib(),1.0/512.0,0.0);		// horiz
  glUniform1iARB(blurShade->getTextureSourceAttrib(),0);
  glActiveTextureARB(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,colorTextureId);

  //Preparing to draw quad
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-RENDER_WIDTH/2,RENDER_WIDTH/2,-RENDER_HEIGHT/2,RENDER_HEIGHT/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //Drawing quad 
  glTranslated(0,0,-5);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(-RENDER_WIDTH/2,-RENDER_HEIGHT/2,0);
  glTexCoord2d(1,0);glVertex3f(RENDER_WIDTH/2,-RENDER_HEIGHT/2,0);
  glTexCoord2d(1,1);glVertex3f(RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
  glTexCoord2d(0,1);glVertex3f(-RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
  glEnd();
  //glGenerateMipmapEXT(GL_TEXTURE_2D);
		
		 
  // Bluring vertically
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	
  //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	
  glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_COEF ,RENDER_HEIGHT* SHADOW_MAP_COEF);
  //glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (RENDER_HEIGHT * SHADOW_MAP_COEF ) );	
  glUniform2fARB( blurShade->getScaleAttrib(),0.0, 1.0/ (512.0 ) );
  glBindTexture(GL_TEXTURE_2D,blurFboIdColorTextureId);
  //glBindTexture(GL_TEXTURE_2D,colorTextureId);
  glBegin(GL_QUADS);
  glTexCoord2d(0,0);glVertex3f(-RENDER_WIDTH/2,-RENDER_HEIGHT/2,0);
  glTexCoord2d(1,0);glVertex3f(RENDER_WIDTH/2,-RENDER_HEIGHT/2,0);
  glTexCoord2d(1,1);glVertex3f(RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
  glTexCoord2d(0,1);glVertex3f(-RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
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
  glOrtho(-RENDER_WIDTH/2,RENDER_WIDTH/2,-RENDER_HEIGHT/2,RENDER_HEIGHT/2,1,20);
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
  glTexCoord2d(1,0);glVertex3f(RENDER_WIDTH/2,0,0);
  glTexCoord2d(1,1);glVertex3f(RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
  glTexCoord2d(0,1);glVertex3f(0,RENDER_HEIGHT/2,0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void drawObjects(GeometryShader * curShade) {
  //viewport.orientation = vehicle->getCurrentLocation();

  // Ground [double for face culling]
  if(renderOpt.isDispGround()) {
      glColor4f(1.0f,1.0f,1.0f,1);
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

  startTranslate(0,0,-5);
  //cout << "Dumping in main" << endl;
  //cout << glGetError() << endl;
  sweep->renderWithDisplayList(*curShade,20,0.3,20);
  //cout << glGetError() << endl;
  endTranslate();



  mat4 vehLoc = vehicle->getCurrentLocation();
  cout << vehLoc[0] << endl;
  cout << vehLoc[1] << endl;
  cout << vehLoc[2] << endl;
  cout << vehLoc[3] << endl;
  vehicle->setSweepTime(frameCount / 100.0);
  frameCount = ++frameCount % 100;
  cout << frameCount << endl;
  vec3 location = vehicle->getPerspectiveLocation();
  vec3 center = vehicle->getPerspectiveCenter();
  vec4 uVec = vehicle->uVec();

  //gluLookAt(location[VX], location[VY], location[VZ], center[VX], center[VY], center[VZ], uVec[VX], uVec[VY], uVec[VZ]);

  //startTranslate(vehLoc[0][3], vehLoc[1][3], vehLoc[2][3]-5);
  startTeapotMove(vehicle->getCurrentLocation());
  vehicle->draw();
  endTranslate();

  //Chris: i dont know if you want to store position inside of vehicle, but you would change reference by
  //p_camera = something
  //l_camera = something

  //l_camera = location;
  //p_camera = center;
}

void renderScene() 
{
  //First step: Render from the light POV to a FBO, store depth and square depth in a 32F frameBuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen
  //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	//Rendering onscreen
	
  //Using the depth shader to do so
  glUseProgramObjectARB(depthShade->getProgram());
  // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
  glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_COEF,RENDER_HEIGHT* SHADOW_MAP_COEF);

  // Clear previous frame values
  glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
  setupMatrices(p_light[0],p_light[1],p_light[2],l_light[0],l_light[1],l_light[2]);
	
  // Culling switching, rendering only backface, this is done to avoid self-shadowing and improve efficiency
  glCullFace(GL_FRONT);
  //draw objects using the depth shader
  drawObjects(depthShade);
	
  glGenerateMipmapEXT(GL_TEXTURE_2D);
  //Save modelview/projection matrice into texture7, also add a biais
  setTextureMatrix();
	
  //BLURRRRRR
  blurShadowMap();
     
  // Now rendering from the camera POV, using the FBO to generate shadows
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	
  glViewport(0,0,RENDER_WIDTH,RENDER_HEIGHT);
		
  // Clear previous frame values
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //Using the shadow shader
  glUseProgramObjectARB(shade->getProgram());
  glUniform1iARB(shade->getShadowMapAttrib(),7);
  glUniform1fARB(shade->getXPixelOffsetAttrib(),1.0/ (RENDER_WIDTH * SHADOW_MAP_COEF));
  glUniform1fARB(shade->getYPixelOffsetAttrib(),1.0/ (RENDER_HEIGHT * SHADOW_MAP_COEF));
  glActiveTextureARB(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D,colorTextureId);
	
  setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2]);
  
  //okay seriously, why do we have vec and float[] is required by openGL -_-
  float tempLight[4] = {p_light[0], p_light[1], p_light[2], 1};
  glLightfv(GL_LIGHT0, GL_POSITION, tempLight);

  glCullFace(GL_BACK);
  //draw objects using our shadow shader
  drawObjects(shade);
  
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
  case 'D':
  case 'd':
    renderOpt.toggleDispDepthBuffer();
    break;
  case 'G':
  case 'g':
    renderOpt.toggleDispGround();
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
  glutMotionFunc(myActiveMotionFunc);
  glutPassiveMotionFunc(myPassiveMotionFunc);

  glewInit();

  if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    printf("Ready for GLSL\n");
  else {
    printf("No GLSL support\n");
    exit(1);
  }

  //generate the shadow FBO 
  generateShadowFBO();

  shade = new ShadowShader("shaders/MainVertexShader.c", "shaders/MainFragmentShader.c");
  blurShade = new BlurShader("shaders/GaussianBlurVertexShader.c", "shaders/GaussianBlurFragmentShader.c");
  depthShade = new GeometryShader("shaders/DepthVertexShader.c", "shaders/DepthFragmentShader.c");

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

  vehicle = new Vehicle(sweep, mat4(vec4(1,0,0,0), vec4(0,1,0,0), vec4(0,0,1,0), vec4(0,0,0,1)), vec3(1,0,0));
  vehicle->setAccelerate(true);
  vehicle->setVelocity(0.1);

  //And Go!
  glutMainLoop();
}
