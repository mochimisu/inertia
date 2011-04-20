class Viewport {
public:
  Viewport(): mousePos(0.0,0.0) { orientation = identity3D(); };
  int w, h; // width and height
  vec2 mousePos;
  mat4 orientation;
};
