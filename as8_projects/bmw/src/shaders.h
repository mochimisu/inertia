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

// helpful class to set up shader
class Shader {
 public:
  Shader(string vertShader, string fragmentShader);
  ~Shader() {
    glDeleteObjectARB(program);
  }

  void toggleShader() { shadersOn = !shadersOn; }
  void toggleBumpMap() { bumpMapEnabled = !bumpMapEnabled; }
  void toggleTextureMap() { textureMapEnabled = !textureMapEnabled; }
  void togglePhong() { phongEnabled = !phongEnabled; }
  void toggleDisplacement() { displacementEnabled = !displacementEnabled; }

  void toggleShadows() { shadowMapEnabled = !shadowMapEnabled; }
  void toggleAmbientOcclusion() { ambientOcclusionEnabled = !ambientOcclusionEnabled; }
  void toggleDispAmbientLayer() { dispAmbientLayer = !dispAmbientLayer; }
  void togglePcf() { pcfEnabled = !pcfEnabled; }
  void toggleEnv() { envEnabled = !envEnabled; }
  
  //return bools to unify for console output
  bool isShader() { return shadersOn; }
  bool isBumpMap() { return bumpMapEnabled; }
  bool isTextureMap() { return textureMapEnabled; }
  bool isPhong() { return phongEnabled; }
  bool isDisplacement() { return displacementEnabled; }

  bool isShadows() { return shadowMapEnabled; }
  bool isAmbientOcclusion() { return ambientOcclusionEnabled; }
  bool isDispAmbientLayer() { return dispAmbientLayer; }
  bool isPcf() { return pcfEnabled; }
  bool isEnv() { return envEnabled; }

  void setPixelOffset(float x, float y) { xPixelOffset = x; yPixelOffset = y; }
  

  GLint getTangentAttrib() { return tangentAttrib; }
  GLint getBitangentAttrib() { return bitangentAttrib; }

  GLhandleARB getProgram() { return program; }
  GLint getShadowMapAttrib() { return shadowMapUniform; }

  bool on() { return shadersOn; }

  void set(bool forceOff = false) {
    if (!forceOff && on()) {
      glUseProgramObjectARB(program);
      glUniform1iARB(bumpMapEnabledUniform, bumpMapEnabled);
      glUniform1iARB(textureMapEnabledUniform, textureMapEnabled);
      glUniform1iARB(phongEnabledUniform, phongEnabled);
      glUniform1iARB(displacementEnabledUniform, displacementEnabled);
      glUniform1iARB(shadowMapEnabledUniform, shadowMapEnabled);
      glUniform1iARB(ambientOcclusionEnabledUniform, ambientOcclusionEnabled);
      glUniform1iARB(dispAmbientLayerUniform, dispAmbientLayer);
      glUniform1iARB(pcfEnabledUniform, pcfEnabled);
      glUniform1fARB(xPixelOffsetUniform, xPixelOffset);
      glUniform1fARB(yPixelOffsetUniform, yPixelOffset);
      glUniform1iARB(envEnabledUniform, envEnabled);
    } else {
      glUseProgramObjectARB(0);
    }
  }

 private:
  bool shadersFailed;
  GLhandleARB program; // shaders
  GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
  GLint bumpMapEnabledUniform, textureMapEnabledUniform, phongEnabledUniform;
  GLint displacementEnabledUniform, shadowMapEnabledUniform, shadowMapUniform;
  GLint ambientOcclusionEnabledUniform, dispAmbientLayerUniform, pcfEnabledUniform;
  GLint xPixelOffsetUniform, yPixelOffsetUniform, envEnabledUniform;

  bool shadowMapEnabled, ambientOcclusionEnabled, dispAmbientLayer;
  bool displacementEnabled, pcfEnabled, envEnabled;

  float xPixelOffset, yPixelOffset;

  bool bumpMapEnabled, textureMapEnabled, phongEnabled;
  bool shadersOn;
};

#endif 

