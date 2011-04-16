#include "main.h"

using namespace std;


//****************************************************
// Some Classes
//****************************************************
class Viewport {
public:
  Viewport(): mousePos(0.0,0.0) { orientation = identity3D(); };
  int w, h; // width and height
  vec2 mousePos;
  mat4 orientation;
};

//****************************************************
// Global Variables
//****************************************************
Viewport viewport;
UCB::ImageSaver * imgSaver;
int frameCount = 0;
Sweep *sweep;
Mesh *mesh;
Shader *shade;

GLuint skyText, skyTextCube;

//fps counter
int fpsFrame=0, fpsTime, fpsTimebase = 0;
double curFps=0;
bool dispFps;

//to save a movie
int framesToSave = 0;
int rotationDeg = 0;

//static console output
Console staticConsole;

//shadow map resolution
#define SHADOW_MAP_RATIO 2
				
// Hold id of the framebuffer for light POV rendering
GLuint fboId;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint depthTextureId;

//display extra stuff on screen
bool dispDepthBuffer, dispConsole;
bool dispSkybox, dispGround;

//shadow mapping FBO generously contributed by Fabien Sanglard			
void generateShadowFBO()
{
  int shadowMapWidth = viewport.w * SHADOW_MAP_RATIO;
  int shadowMapHeight = viewport.h * SHADOW_MAP_RATIO;
  shade->setPixelOffset(1.0/ shadowMapWidth, 1.0/ shadowMapHeight);
	
  GLenum FBOstatus;

  // Try to use a texture depth component
  glGenTextures(1, &depthTextureId);
  glBindTexture(GL_TEXTURE_2D, depthTextureId);

  // GL_LINEAR does not make sense for depth texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Remove artifact on the edges of the shadowmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

  // No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  // create a framebuffer object
  glGenFramebuffersEXT(1, &fboId);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

  // Instruct openGL that we won't bind a color texture with the currently binded FBO
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
 
  // attach the texture to FBO depth attachment point
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthTextureId, 0);

  // check FBO status
  FBOstatus = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT)
    printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

  // switch back to window-system-provided framebuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//Camera position
vec3 p_camera(16,20,0);

//Camera lookAt
vec3 l_camera(0,0,-5);

//Light position
vec3 p_light(4,25,0);

//Light lookAt
vec3 l_light(0,0,-5);

void setupMatrices(float position_x,float position_y,float position_z,float lookAt_x,float lookAt_y,float lookAt_z)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45,viewport.w/viewport.h,10,40000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(position_x,position_y,position_z,lookAt_x,lookAt_y,lookAt_z,0,1,0);
}



void setTextureMatrix(void)
{
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
  glActiveTextureARB(GL_TEXTURE4);
	
  glLoadIdentity();	
  glLoadMatrixd(bias);
	
  // concatating all matrice into one.
  glMultMatrixd (projection);
  glMultMatrixd (modelView);
	
  // Go back to normal matrix mode
  glMatrixMode(GL_MODELVIEW);
}

// A simple helper function to load a mat4 into opengl
void applyMat4(mat4 &mat) {
  double glmat[16];
  int k = 0;
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      glmat[k++] = mat[j][i];
    }
  }
  glMultMatrixd(glmat);
}


// During translation, we also have to maintain the GL_TEXTURE4, used in the shadow shader
// to determine if a vertex is in the shadow.
void startTranslate(float x,float y,float z)
{
  glPushMatrix();
  glTranslatef(x,y,z);
  applyMat4(viewport.orientation);
  glMatrixMode(GL_TEXTURE);
  glActiveTextureARB(GL_TEXTURE4);
  glPushMatrix();
  glTranslatef(x,y,z);
  applyMat4(viewport.orientation);
}

void endTranslate()
{
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void drawInvertedTextureCube(double size) {
  glColor3f(1,1,1);
	
  glBegin(GL_QUADS);
  // Front Face
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, size);	// Top Left Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, size);	// Top Right Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -size, size);	// Bottom Right Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, size);	// Bottom Left Of The Texture and Quad
  // Back Face
  glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -size, -size);	// Bottom Left Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( size,  size, -size);	// Top Left Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, size, -size);	// Top Right Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, -size);	// Bottom Right Of The Texture and Quad
  // Top Face
  glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, -size);	// Top Right Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f( size,  size,  size);	// Bottom Right Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, size, size);	// Bottom Left Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, size, -size);	// Top Left Of The Texture and Quad
  // Bottom Face
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, size);	// Bottom Right Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -size, size);	// Bottom Left Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( size, -size, -size);	// Top Left Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-size, -size, -size);	// Top Right Of The Texture and Quad
  // Right face
  glTexCoord2f(0.0f, 0.0f); glVertex3f( size, -size, size);	// Bottom Left Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( size,  size,  size);	// Top Left Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f( size,  size, -size);	// Top Right Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f( size, -size, -size);	// Bottom Right Of The Texture and Quad
  // Left Face
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-size,  size, -size);	// Top Left Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-size,  size, size);	// Top Right Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-size, -size, size);	// Bottom Right Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -size, -size);	// Bottom Left Of The Texture and Quad
  glEnd();

}

void drawSkyBox2() {
  glPushMatrix();
  glTranslatef(0,0,-5);
  applyMat4(viewport.orientation);
  glActiveTextureARB(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,skyText);
  drawInvertedTextureCube(100);
  glPopMatrix();
}

void drawSkyBox() {
    glUseProgramObjectARB(NULL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewport.w/2,viewport.w/2,-viewport.h/2,viewport.h/2,1,20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,skyText);
    glEnable(GL_TEXTURE_2D);
    glTranslatef(0,0,-15);
    glBegin(GL_QUADS);

    glTexCoord2d(0,0);glVertex3f(-viewport.w/2,-viewport.h/2,0);
    glTexCoord2d(1,0);glVertex3f(viewport.w/2,-viewport.h/2,0);
    glTexCoord2d(1,1);glVertex3f(viewport.w/2,viewport.h/2,0);
    glTexCoord2d(0,1);glVertex3f(-viewport.w/2,viewport.h/2,0);
	 
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawStuff() {

  if(dispGround) {
    // Ground
    glColor4f(1.0f,1.0f,1.0f,1);
  
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,skyText);
    glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex3f(-50,-10,-50);
    glTexCoord2d(1,0);glVertex3f(-50,-10, 50);
    glTexCoord2d(1,1);glVertex3f( 50,-10, 50);
    glTexCoord2d(0,1);glVertex3f( 50,-10,-50);
    glEnd();
  }


  //maintain orientation with depth buffer
  startTranslate(0,0,-5);

  mesh->draw(*shade);

  sweep->renderWithDisplayList(*shade,20,0.3,20);
  endTranslate();
}

void drawDepthBuffer() {
    glUseProgramObjectARB(NULL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewport.w/2,viewport.w/2,-viewport.h/2,viewport.h/2,1,20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,depthTextureId);
    //glBindTexture(GL_TEXTURE_2D,skyTextCube); //testing cube maps
    glEnable(GL_TEXTURE_2D);
    glTranslatef(0,0,-1);
    glBegin(GL_QUADS);

    glTexCoord2d(0,0);glVertex3f(0,0,0);
    glTexCoord2d(1,0);glVertex3f(viewport.w/2,0,0);
    glTexCoord2d(1,1);glVertex3f(viewport.w/2,viewport.h/2,0);
    glTexCoord2d(0,1);glVertex3f(0,viewport.h/2,0);
	 
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawTestBuffer() {
    glUseProgramObjectARB(NULL);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewport.w/2,viewport.w/2,-viewport.h/2,viewport.h/2,1,20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glActiveTextureARB(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D,depthTextureId);
    glBindTexture(GL_TEXTURE_2D,skyTextCube); //testing cube maps
    glEnable(GL_TEXTURE_2D);
    glTranslatef(0,0,-1);
    glBegin(GL_QUADS);

    glTexCoord2d(0,0);glVertex3f(0,0,0);
    glTexCoord2d(1,0);glVertex3f(viewport.w/2,0,0);
    glTexCoord2d(1,1);glVertex3f(viewport.w/2,viewport.h/2,0);
    glTexCoord2d(0,1);glVertex3f(0,viewport.h/2,0);
	 
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawConsole() {

  //enable the console

  staticConsole.config(-viewport.w/2+10,-viewport.w/2+15*12+10,15,12);
  //staticConsole.config(-viewport.w/2 + 1,0,9,10);

  std::ostringstream buff;

  glUseProgramObjectARB(NULL);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-viewport.w/2,viewport.w/2,-viewport.h/2,viewport.h/2,1,20);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
    glActiveTextureARB(GL_TEXTURE0);

  glTranslatef(0,0,-1); // put console in view

  //static console
  buff.str("");
  if(dispFps) {
    buff << "FPS: ";
    buff << curFps;
  } else {
    buff << "Display FPS";
  }
  buff << " [ r ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Shaders: ";
  if(shade->isShader())
    buff << "On";
  else
    buff << "Off";
  buff << " [ z ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Shading: ";
  if(shade->isPhong())
    buff << "Phong";
  else
    buff << "Gourad";
  buff << " [ p ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Texture Map: ";
  if(shade->isTextureMap())
    buff << "On";
  else
    buff << "Off";
  buff << " [ t ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Bump Map: ";
  if(shade->isBumpMap())
    buff << "On";
  else
    buff << "Off";
  buff << " [ b ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Displacement Map: ";
  if(shade->isDisplacement())
    buff << "On";
  else
    buff << "Off";
  buff << " [ d ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Shadows: ";
  if(shade->isShadows())
    buff << "On";
  else
    buff << "Off";
  buff << " [ h ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Ambient Occlusion: ";
  if(shade->isAmbientOcclusion())
    buff << "On";
  else
    buff << "Off";
  buff << " [ a ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Percentage Closer Filtering: ";
  if(shade->isPcf())
    buff << "On";
  else
    buff << "Off";
  buff << " [ f ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Skybox: ";
  if(dispSkybox)
    buff << "On";
  else
    buff << "Off";
  buff << " [ y ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Display Ground: ";
  if(dispGround)
    buff << "On";
  else
    buff << "Off";
  buff << " [ g ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Display Depth Buffer: ";
  if(dispDepthBuffer)
    buff << "On";
  else
    buff << "Off";
  buff << " [ u ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "Display Ambient Layer: ";
  if(shade->isDispAmbientLayer())
    buff << "On";
  else
    buff << "Off";
  buff << " [ m ]";
  staticConsole.output(buff.str());

  buff.str("");
  buff << "[ s:save ], [ q:save 100 frames ], [ c:toggle console ], [ esc: quit ]";
  staticConsole.output(buff.str());
  buff.str("");
  buff << "[ arrow keys+pgup/pgdn:move camera ], [ home:reset camera ], [ end:reset light ], [ numpad:move lights ]";
  staticConsole.output(buff.str());

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glColor4f(1,1,1,1);
  staticConsole.draw();
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnd();

}

//-------------------------------------------------------------------------------
/// You will be calling all of your drawing-related code from this function.
/// Nowhere else in your code should you use glBegin(...) and glEnd() except code
/// called from this method.
///
/// To force a redraw of the screen (eg. after mouse events or the like) simply call
/// glutPostRedisplay();
void displayold() {
  //Clear Buffers
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);					// indicate we are specifying camera transformations
  glLoadIdentity();							// make sure transformation is "zero'd"

  drawStuff();
  // feel free to remove the mesh; it's mainly there as a demo
  //mesh->draw(*shade);

  if(framesToSave > 0) {
    imgSaver->saveFrame();
    framesToSave--;
  }

  //Now that we've drawn on the buffer, swap the drawing buffer and the displaying buffer.
  glutSwapBuffers();

}


void display() 
{
  fpsFrame++;
  fpsTime=glutGet(GLUT_ELAPSED_TIME);
  if(fpsFrame%10 == 0)
    curFps = (fpsFrame*1000.0/(fpsTime-fpsTimebase));
  if (fpsTime - fpsTimebase > 1000) {
    fpsTimebase = fpsTime;
    fpsFrame = 0;
  }


  //First step: Render from the light POV to a FBO, story depth values only
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen
	
  //Using the fixed pipeline to render to the depthbuffer
  glUseProgramObjectARB(0);
	
  // In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
  glViewport(0,0,viewport.w * SHADOW_MAP_RATIO,viewport.h* SHADOW_MAP_RATIO);
	
  // Clear previous frame values
  glClear( GL_DEPTH_BUFFER_BIT);
	
  //Disable color rendering, we only want to write to the Z-Buffer
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
	
  setupMatrices(p_light[0],p_light[1],p_light[2],l_light[0],l_light[1],l_light[2]);
	
  // Culling switching, rendering only backface, this is done to avoid self-shadowing
  glCullFace(GL_FRONT);
  //glCullFace(GL_BACK);
  drawStuff();
	
  //Save modelview/projection matrix into texture 4, also add a biais
  setTextureMatrix();
	
	
  // Now rendering from the camera POV, using the FBO to generate shadows
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	
  glViewport(0,0,viewport.w,viewport.h);
	
  //Enabling color write (previously disabled for light POV z-buffer rendering)
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
	
  // Clear previous frame values
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Using the shadow shader
  glUseProgramObjectARB(shade->getProgram());
  glUniform1iARB(shade->getShadowMapAttrib(),4);
  glActiveTextureARB(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D,depthTextureId);
	
  setupMatrices(p_camera[0],p_camera[1],p_camera[2],l_camera[0],l_camera[1],l_camera[2]);
  glCullFace(GL_BACK);
  drawStuff();
  if(dispSkybox)
    drawSkyBox2();


  if(dispConsole)
    drawConsole();

  //display depth buffer
  if(dispDepthBuffer)
    drawDepthBuffer();
  //drawTestBuffer();

	
  if(framesToSave > 0) {
    imgSaver->saveFrame();
    framesToSave--;
  }

  if(rotationDeg > 0) {
    vec3 axis;
    if(rotationDeg > 90) 
      axis =vec3(-1, 0.5, 0);
    else
      axis=vec3(0.5,-1,0);
    viewport.orientation = viewport.orientation * rotation3D(axis, 4);
    imgSaver->saveFrame();
    rotationDeg--;
  }
	
  glutSwapBuffers();
}


//-------------------------------------------------------------------------------
/// \brief	Called when the screen gets resized.
/// This gives you the opportunity to set up all the relevant transforms.
///
void reshape(int w, int h) {
  glViewport(0, 0, w, h);

  int newDim = min(w,h);
  viewport.w = newDim;
  viewport.h = newDim;
  generateShadowFBO();


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, ((double)w / MAX(h, 1)), 1.0, 100.0);
  //glOrtho(-10,10,-10,10,1,100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}


//-------------------------------------------------------------------------------
/// Called to handle keyboard events.
  const float deltaMovement = 1;
void myKeyboardFunc (unsigned char key, int x, int y) {
  switch (key) {
  case 27:			// Escape key
    exit(0);
    break;
  case 'Q':
  case 'q':
    framesToSave=100;
    break;
  case 'S':
  case 's':
    imgSaver->saveFrame();
    break;
  case 'T':
  case 't':
    shade->toggleTextureMap();
    sweep->clearDisplayList();
    break;
  case 'B':
  case 'b':
    shade->toggleBumpMap();
    sweep->clearDisplayList();
    break;
  case 'P':
  case 'p':
    shade->togglePhong();
    sweep->clearDisplayList();
    break;
  case 'D':
  case 'd':
    shade->toggleDisplacement();
    sweep->clearDisplayList();
    break;
  case 'U':
  case 'u':
    dispDepthBuffer = !dispDepthBuffer;
    glutPostRedisplay();
    break;
  case 'H':
  case 'h':
    shade->toggleShadows();
    sweep->clearDisplayList();
    break;
  case 'A':
  case 'a':
    shade->toggleAmbientOcclusion();
    sweep->clearDisplayList();
    break;
  case 'C':
  case 'c':
    dispConsole = !dispConsole;
    glutPostRedisplay();
    break;
  case 'Z':
  case 'z':
    shade->toggleShader();
    sweep->clearDisplayList();
    break;
  case 'M':
  case 'm':
    shade->toggleDispAmbientLayer();
    sweep->clearDisplayList();
    break;
  case 'F':
  case 'f':
    shade->togglePcf();
    sweep->clearDisplayList();
    break;
  case 'G':
  case 'g':
    dispGround = !dispGround;
    glutPostRedisplay();
    break;
  case 'R':
  case 'r':
    dispFps = !dispFps;
    glutPostRedisplay();
    break;
  case 'Y':
  case 'y':
    dispSkybox = !dispSkybox;
    glutPostRedisplay();
    break;
  case 'E':
  case 'e':
    shade->toggleEnv();
    sweep->clearDisplayList();
    break;
  case '[':
    rotationDeg=180;
    break;
  case '9':
    p_light[1] -= deltaMovement;
    glutPostRedisplay();
    break;
  case '7':
    p_light[1] += deltaMovement;
    glutPostRedisplay();
    break;
  case '2':
    p_light[0] -= deltaMovement;
    glutPostRedisplay();
    break;
  case '8':
    p_light[0] += deltaMovement;
    glutPostRedisplay();
    break;
  case '4':
    p_light[2] -= deltaMovement;
    glutPostRedisplay();
    break;
  case '6':
    p_light[2] += deltaMovement;
    glutPostRedisplay();
    break;
  }
}
void myKeyboardFunc2 (int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_UP:
    p_camera[1] -= deltaMovement;
    glutPostRedisplay();
    break;
  case GLUT_KEY_DOWN:
    p_camera[1] += deltaMovement;
    generateShadowFBO();
    glutPostRedisplay();
    break;
  case GLUT_KEY_LEFT:
    p_camera[0] += deltaMovement;
    glutPostRedisplay();
    break;
  case GLUT_KEY_RIGHT:
    p_camera[0] -= deltaMovement;
    generateShadowFBO();
    glutPostRedisplay();
    break;
  case GLUT_KEY_PAGE_UP:
    p_camera[2] += deltaMovement;
    glutPostRedisplay();
    break;
  case GLUT_KEY_PAGE_DOWN:
    p_camera[2] -= deltaMovement;
    generateShadowFBO();
    glutPostRedisplay();
    break;

  case GLUT_KEY_HOME:
    p_camera = vec3(16,20,0);
    glutPostRedisplay();
    break;

  case GLUT_KEY_END:
    p_light = vec3(4,25,0);
    glutPostRedisplay();
    break;
  }
}

//-------------------------------------------------------------------------------
/// Called whenever the mouse moves while a button is pressed
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


//-------------------------------------------------------------------------------
/// Called whenever the mouse moves without any buttons pressed.
void myPassiveMotionFunc(int x, int y) {
  //Record the mouse location for drawing crosshairs
  viewport.mousePos = vec2((double)x / glutGet(GLUT_WINDOW_WIDTH),(double)y / glutGet(GLUT_WINDOW_HEIGHT));

  //Force a redraw of the window.
  glutPostRedisplay();
}

//-------------------------------------------------------------------------------
/// Called to update the screen at 30 fps.
void frameTimer(int value){

  frameCount++;
  glutPostRedisplay();
  glutTimerFunc(1000/30, frameTimer, 1);
}

void exitFreeimage(void) {
  FreeImage_DeInitialise();
}


//-------------------------------------------------------------------------------
/// Initialize the environment
int main(int argc,char** argv) {
  //Initialize OpenGL
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_MULTISAMPLE);

  FreeImage_Initialise();
  atexit(exitFreeimage);

  //Set up global variables
  viewport.w = 600;
  viewport.h = 600;

  if (argc < 2) {
    cout << "USAGE: sweep sweep.trk" << endl;
    return -1;
  }

  //onscreen stuffs
  dispDepthBuffer = false;
  dispConsole = true;
  dispGround = false;
  dispSkybox = false;

  //Initialize the screen capture class to save BMP captures
  //in the current directory, with the prefix "sweep"
  imgSaver = new UCB::ImageSaver("./", "sweep");

  //Create OpenGL Window
  glutInitWindowSize(viewport.w,viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow("CS184 Framework");



  //Register event handlers with OpenGL.
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(myKeyboardFunc);
  glutMotionFunc(myActiveMotionFunc);
  glutPassiveMotionFunc(myPassiveMotionFunc);
  glutSpecialFunc(myKeyboardFunc2);
  frameTimer(0);
  //glClearColor(1,1,1,0);

  // set some lights
  {
    float ambient[4] = { .1f, .1f, .1f, 1.f };
    float diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.f };
    float pos[4] = { p_light[0], p_light[1], p_light[2], 0 };
    //float pos[4] = { 0, 0, 1, 0 };
    //float dir[4] = { l_light[0] - p_light[0],  l_light[1] - p_light[1],  l_light[2] - p_light[2], 0 };
    //float dir[4] = { l_light[0], l_light[1], l_light[2], 0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
    glEnable(GL_LIGHT0);
  }
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  //multisampling for better quality (but performance is bad)
  //glEnable(GL_MULTISAMPLE_ARB);

  //enable face culling for removal
  glEnable(GL_CULL_FACE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

  glewInit();


  if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
    printf("Ready for GLSL\n");
  else {
    printf("No GLSL support\n");
    exit(1);
  }

  shade = new Shader("sweepShader.vert", "sweepShader.frag");

  //generate the shadow FBO 
  generateShadowFBO();
  
  // Create the coaster
  sweep = new Sweep(argv[1]);
  if (sweep->bad()) {
    cout << "Sweep file appears to not have a proper sweep in it" << endl;
    return -1;
  }

  string background = "lantern1.png";
  if(argc > 2)
    background = argv[2];

  loadTexture(background,skyText);

  //skyTextCube = sweep->setSky(background);
  string skyboxes[6] = { background, background,
			 background, background,
			 background, background };
  sweep->setSky(skyboxes);
  //loadCube(background,skyTextCube);

  mesh = new Mesh();
  //mesh->loadFile("frostwurmnorthrend.obj");
  //mesh->loadTextures("DragonDeepholmMount1Purple.tga", "DragonDeepholmMount2Purple.tga");
  mesh->centerAndScale(20);
  // do two steps of subdivision by swapping through a temp mesh
  Mesh temp; mesh->subdivide(temp); temp.subdivide(*mesh);
  // recompute vertex normals
  mesh->computeVertexNormals();

  //And Go!
  glutMainLoop();
}
