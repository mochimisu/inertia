#ifndef RENDEROPTIONS_H_
#define RENDEROPTIONS_H_

class RenderOptions {
 public:
  RenderOptions() {
    dispGround = true;
    dispDepthBuffer = false;
    depthBufferOption = 0;
  }

  //accessors
  bool isDispGround() { return dispGround; }
  bool isDepthBuffer() { return dispDepthBuffer; }
  int getDepthBufferOption() { return depthBufferOption; }

  //setters
  void setDispGround(bool a) { dispGround = a; }
  void setDispDepthBuffer(bool a) { dispDepthBuffer = a; }
  void setDepthBufferOption(int a) { depthBufferOption = a; }

  //togglers
  void toggleDispGround() { dispGround = !dispGround; }
  void toggleDispDepthBuffer() { dispDepthBuffer = !dispDepthBuffer; }

 private:
  bool dispGround;
  bool dispDepthBuffer;
  int depthBufferOption;
  //..add more when not tired
};

#endif
