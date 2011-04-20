/*
 * Shaders.h
 *
 *  Created on: March 19, 2009
 *      Author: jima, based on nvidia and lighthouse3D example code.
 */

#ifndef SHADERS_H_
#define SHADERS_H_

#include "global.h"

bool setShader(GLhandleARB p, const char *srcfile, GLenum type);
bool linkAndValidateShader(GLhandleARB p);

class Shader {
 public:
  Shader(string vertShader, string fragmentShader);
  ~Shader() {
    glDeleteObjectARB(program);
  }
  //Toggles
  void toggleShader() { shadersOn = !shadersOn; }

  //Bools
  bool isShader() { return shadersOn; }

  //Setting Shader Options

  //Handles
  GLhandleARB getProgram() { return program; }

  //etc
  bool on() { return shadersOn; }
  void set(bool forceOff = false) {
    if (!forceOff && on()) {
      this->setUniformValues();
    } else {
      glUseProgramObjectARB(0);
    }
  }
  virtual void setUniformValues();

  int getId() { return shadeId; }

 protected:
  int shadeId;
  bool shadersOn;
  GLhandleARB program; // shader
  bool shadersFailed;
  //Uniform handles
  GLint tangentAttrib, bitangentAttrib; 
};

class GeometryShader : public Shader {
 public:
  GeometryShader(string vertShader, string fragmentShader);
  //Toggles
  void toggleDisplacement() { displacementEnabled = !displacementEnabled; }

  //Bools
  bool isDisplacement() { return displacementEnabled; }

  //Handles
  GLint getTangentAttrib() { return tangentAttrib; }
  GLint getBitangentAttrib() { return bitangentAttrib; }

  //etc
  virtual void setUniformValues();

 protected:
  GLint displacementEnabled;
  GLint displacementEnabledUniform;
};

class ShadowShader : public GeometryShader {
 public:
  ShadowShader(string vertShader, string fragmentShader);
  //Toggles
  void toggleBumpMap() { bumpMapEnabled = !bumpMapEnabled; }
  void toggleTextureMap() { textureMapEnabled = !textureMapEnabled; }
  void togglePhong() { phongEnabled = !phongEnabled; }
  void toggleShadows() { shadowMapEnabled = !shadowMapEnabled; }
  void toggleAmbientOcclusion() { ambientOcclusionEnabled = !ambientOcclusionEnabled; }
  void toggleDispAmbientLayer() { dispAmbientLayer = !dispAmbientLayer; }
  void togglePcf() { pcfEnabled = !pcfEnabled; }
  //void toggleEnv() { envEnabled = !envEnabled; }

  //Booleans
  bool isShader() { return shadersOn; }
  bool isBumpMap() { return bumpMapEnabled; }
  bool isTextureMap() { return textureMapEnabled; }
  bool isPhong() { return phongEnabled; }
  bool isShadows() { return shadowMapEnabled; }
  bool isAmbientOcclusion() { return ambientOcclusionEnabled; }
  bool isDispAmbientLayer() { return dispAmbientLayer; }
  bool isPcf() { return pcfEnabled; }
  //bool isEnv() { return envEnabled; }

  //Setting shader options
  void setPixelOffset(float x, float y) { xPixelOffset = x; yPixelOffset = y; }
  void setXPixelOffset(float x) { xPixelOffset = x; }
  void setYPixelOffset(float y) { yPixelOffset = y; }
  void setShadowMap(int x) { shadowMap = x; }

  //Handles
  GLint getShadowMapAttrib() { return shadowMapUniform; }

  //etc
  virtual void setUniformValues();
  
  //temp
  GLint getXPixelOffsetAttrib() { return xPixelOffsetUniform; }
  GLint getYPixelOffsetAttrib() { return yPixelOffsetUniform; }

 protected:
  //Uniform handles
  GLint bumpMapEnabledUniform, textureMapEnabledUniform, phongEnabledUniform;
  GLint shadowMapEnabledUniform, shadowMapUniform;
  GLint ambientOcclusionEnabledUniform, dispAmbientLayerUniform, pcfEnabledUniform;
  GLint xPixelOffsetUniform, yPixelOffsetUniform, envEnabledUniform;

  //bool toggles
  bool bumpMapEnabled, textureMapEnabled, phongEnabled;
  bool shadowMapEnabled, ambientOcclusionEnabled, dispAmbientLayer;
  bool pcfEnabled, envEnabled;

  //values
  float xPixelOffset, yPixelOffset;
  int shadowMap;
  
};

class BlurShader : public Shader {
 public:
  BlurShader(string vertShader, string fragmentShader);
  //Toggles
  //Booleans
  //Shader Options
  void setScale(float a, float b) { scaleUa = a; scaleUb = b; }
  void setTextureSource(int x) { textureSource = x; }

  //etc
  virtual void setUniformValues();


  //temp
  GLint getScaleAttrib() { return scaleUniform; }
  GLint getTextureSourceAttrib() { return textureSourceUniform; }
  
  //Handles
 protected:
  //Uniform Handles
  GLint scaleUniform, textureSourceUniform;

  //values
  float scaleUa, scaleUb;
  int textureSource;
};

#endif 

