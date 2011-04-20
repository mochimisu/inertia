#include "main.h"

Viewport viewport; GLuint bgText;
// Expressed as float so gluPerspective division returns a float and not 0 (640/480 != 640.0/480.0).
#define RENDER_WIDTH 1024.0
#define RENDER_HEIGHT 768.0
#define SHADOW_MAP_COEF 1
#define BLUR_COEF 1

//Camera position
float p_camera[4] = {16,20,0,1};

//Camera lookAt
float l_camera[3] = {0,0,-5};

//Light position
float p_light[4] = {4,25,0,1};

//Light lookAt
float l_light[3] = {0,0,-5};

//Light mouvement circle radius
float light_mvnt = 35.0f;

// Hold id of the framebuffer for light POV rendering
GLuint fboId;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint depthTextureId;
GLuint colorTextureId;

// Bluring FBO
GLuint blurFboId;
// Z values will be rendered to this texture when using fboId framebuffer
GLuint blurFboIdColorTextureId;

ShadowShader* shade;
BlurShader *blurShade;
GeometryShader *depthShade;


Sweep *sweep, *sweep2;

void generateShadowFBO()
{
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


// This update only change the position of the light.
//int elapsedTimeCounter = 0;
void update(void)
{
	//printf("%d\n",glutGet(GLUT_ELAPSED_TIME));
	p_light[0] = light_mvnt * cos(glutGet(GLUT_ELAPSED_TIME)/2000.0);
	p_light[2] = light_mvnt * sin(glutGet(GLUT_ELAPSED_TIME)/2000.0);
	
	//p_light[0] = light_mvnt * cos(4000/1000.0);
	//p_light[2] = light_mvnt * sin(4000/1000.0);

	//p_light[0] = light_mvnt * cos(3000/1000.0);
	//p_light[2] = light_mvnt * sin(3000/1000.0);


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
	glActiveTextureARB(GL_TEXTURE7);
	
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


// During translation, we also have to maintain the GL_TEXTURE8, used in the shadow shader
// to determine if a vertex is in the shadow.
void startTranslate(float x,float y,float z)
{
	glPushMatrix();
	glTranslatef(x,y,z);
  applyMat4(viewport.orientation);
	
	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
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

void drawObjects(void)
{
	// Ground
  
	glColor4f(0.3f,0.3f,0.3f,1);
	glBegin(GL_QUADS);
	glVertex3f(-45,2,-45);
	glVertex3f(-45,2, 55);
	glVertex3f( 55,2, 55);
	glVertex3f( 55,2,-45);
	glEnd();
	glBegin(GL_QUADS);
	glVertex3f( 55,2,-45);
	glVertex3f( 55,2, 55);
	glVertex3f(-45,2, 55);
	glVertex3f(-45,2,-45);
	glEnd();
  
	glColor4f(0.9f,0.9f,0.9f,1);
	
	// Instead of calling glTranslatef, we need a custom function that also maintain the light matrix
	startTranslate(0,8,-10);
	glutSolidCube(4);
	endTranslate();

	startTranslate(-8,4,-10);
	glutSolidCube(4);
	endTranslate();

	startTranslate(8,4,-10);
	glutSolidCube(4);
	endTranslate();
	
	startTranslate(0,8,-5);
	//glutSolidCube(4);
	//glutSolidSphere(4,40,40);
	endTranslate();
	
	
}

void blurShadowMap(void)
{
  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
	// Bluring the shadow map  horinzontaly
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,blurFboId);
  //	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	
		glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_COEF *BLUR_COEF ,RENDER_HEIGHT* SHADOW_MAP_COEF*BLUR_COEF);
		glUseProgramObjectARB(blurShade->getProgram());
		//glUniform2fARB( blurShade->getScaleAttrib(),1.0/ (RENDER_WIDTH * SHADOW_MAP_COEF * BLUR_COEF),0.0);
		glUniform2fARB( blurShade->getScaleAttrib(),1.0/512.0,0.0);		// Bluring horinzontaly
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


void renderScene(void) 
{
  //update();
	
	//First step: Render from the light POV to a FBO, store depth and square depth in a 32F frameBuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);	//Rendering offscreen
	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);	//Rendering onscreen
	
	//Using the custom shader to do so
	glUseProgramObjectARB(depthShade->getProgram());
	
	// In the case we render the shadowmap to a higher resolution, the viewport must be modified accordingly.
	glViewport(0,0,RENDER_WIDTH * SHADOW_MAP_COEF,RENDER_HEIGHT* SHADOW_MAP_COEF);
	
	// Clear previous frame values
	glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
		
	setupMatrices(p_light[0],p_light[1],p_light[2],l_light[0],l_light[1],l_light[2]);
	
	// Culling switching, rendering only backface, this is done to avoid self-shadowing
	glCullFace(GL_FRONT);
	//drawObjects();
    // Ground
    glColor4f(1.0f,1.0f,1.0f,1);
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,bgText);
    glBegin(GL_QUADS);
    glTexCoord2d(0,1);glVertex3f( 50,-10,-50);
    glTexCoord2d(1,1);glVertex3f( 50,-10, 50);
    glTexCoord2d(1,0);glVertex3f(-50,-10, 50);
    glTexCoord2d(0,0);glVertex3f(-50,-10,-50);
    glEnd();
	startTranslate(0,0,-5);
	sweep2->renderWithDisplayList(*depthShade,20,0.3,20);
	endTranslate();

	
	glGenerateMipmapEXT(GL_TEXTURE_2D);
	//Save modelview/projection matrice into texture7, also add a biais
	setTextureMatrix();
	
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
	
	glLightfv(GL_LIGHT0, GL_POSITION, p_light);

	glCullFace(GL_BACK);
	//drawObjects();

    // Ground
    glColor4f(1.0f,1.0f,1.0f,1);
    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,bgText);
    glBegin(GL_QUADS);
    glTexCoord2d(0,0);glVertex3f(-50,-10,-50);
    glTexCoord2d(1,0);glVertex3f(-50,-10, 50);
    glTexCoord2d(1,1);glVertex3f( 50,-10, 50);
    glTexCoord2d(0,1);glVertex3f( 50,-10,-50);
    glEnd();
	startTranslate(0,0,-5);
	sweep->renderWithDisplayList(*shade,20,0.3,20);
	endTranslate();
	
	// DEBUG only. this piece of code draw the depth buffer onscreen
	
	 glUseProgramObjectARB(0);
	 glMatrixMode(GL_PROJECTION);
	 glLoadIdentity();
	 glOrtho(-RENDER_WIDTH/2,RENDER_WIDTH/2,-RENDER_HEIGHT/2,RENDER_HEIGHT/2,1,20);
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();
	 glColor4f(1,1,1,1);
	 glActiveTextureARB(GL_TEXTURE0);
	 glBindTexture(GL_TEXTURE_2D,blurFboIdColorTextureId);
	 //glBindTexture(GL_TEXTURE_2D,colorTextureId);
	 glEnable(GL_TEXTURE_2D);
	 glTranslated(0,0,-1);
	 glBegin(GL_QUADS);
	 glTexCoord2d(0,0);glVertex3f(0,0,0);
	 glTexCoord2d(1,0);glVertex3f(RENDER_WIDTH/2,0,0);
	 glTexCoord2d(1,1);glVertex3f(RENDER_WIDTH/2,RENDER_HEIGHT/2,0);
	 glTexCoord2d(0,1);glVertex3f(0,RENDER_HEIGHT/2,0);
	 glEnd();
	 glDisable(GL_TEXTURE_2D);
	
	
	
	glutSwapBuffers();
}


void processNormalKeys(unsigned char key, int x, int y) {
	
	if (key == 27) 
		exit(0);
}void myActiveMotionFunc(int x, int y) {

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
}//-------------------------------------------------------------------------------
/// Initialize the environment
int main(int argc,char** argv) {
  //Initialize OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(RENDER_WIDTH,RENDER_HEIGHT);
	glutCreateWindow("Inertia 0.0000001");

  FreeImage_Initialise();

  viewport.w = 600;
  viewport.h = 600;

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
	// This is important, if not here, FBO's depthbuffer won't be populated.
	glEnable(GL_DEPTH_TEST);
	glClearColor(0,0,0,1.0f);
	
	glEnable(GL_CULL_FACE);
	
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

  shade = new ShadowShader("VertexShader.c", "FragmentShader.c");
  //blurShade = new BlurShader("BlurVertexShader.c", "BlurFragmentShader.c");
  blurShade = new BlurShader("blurVertexShader2.c", "blurFragmentShader2.c");
  depthShade = new GeometryShader("StoreDepthVertexShader.c", "StoreDepthFragmentShader.c");

 
  sweep = new Sweep(argv[1]);
  sweep2 = new Sweep(argv[1]);


  string bg = "lantern1.png";
  if(argc > 2)
    bg = argv[2];

  loadTexture(bg,bgText);


  //And Go!
  glutMainLoop();
}
