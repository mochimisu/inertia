#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "algebra3.h"
#include "GL/glew.h"

void applyMat4(const mat4 &mat);

void makeFromMat4(double * glmat, const mat4 &mat);


// Use these instead of pushMatrix() and popMatrix(). These maintain the orientations of the depth buffer in transformed objects.
void pushTranslate(float x,float y,float z);

void pushMat4(mat4 xform);
void pushXformd(const GLdouble* m);
void pushXformf(const GLfloat* m);
void popTransform();
#endif
