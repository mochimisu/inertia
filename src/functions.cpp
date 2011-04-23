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