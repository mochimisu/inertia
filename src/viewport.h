#ifndef VIEWPORT_H_
#define VIEWPORT_H_

class Viewport {
public:
  Viewport(): mousePos(0.0,0.0) { orientation = identity3D(); };
  int w, h; // width and height
  vec2 mousePos;
  mat4 orientation;
};

#endif
