#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "algebra3.h"
#include "GL/glew.h"

void applyMat4(const mat4 &mat);

void makeFromMat4(double * glmat, const mat4 &mat);

#endif