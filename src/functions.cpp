#include "functions.h"
  
// A simple helper function to load a mat4 into opengl
// changed to const because 1. we shouldn't be modifying it, and 2. if it changes we want an error.
void applyMat4(const mat4 &mat) {
  double glmat[16];
  makeFromMat4(glmat, mat);
  glMultMatrixd(glmat);
}

void makeFromMat4(double * glmat, const mat4 &mat) {
  int k = 0;
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < 4; i++) {
      glmat[k++] = mat[j][i];
    }
  }
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
