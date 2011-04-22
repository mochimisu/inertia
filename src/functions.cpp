#include "functions.h"
  
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