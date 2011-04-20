#include "shaders.h"

// from www.lighthouse3d.com
char *textFileRead(const char *fn) {
  FILE *fp;
  char *content = NULL;

  int count=0;

  if (fn != NULL) {
    fp = fopen(fn,"rt");

    if (fp != NULL) {

      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

      if (count > 0) {
	content = (char *)malloc(sizeof(char) * (count+1));
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
      }
      fclose(fp);
    }
  }
  return content;
}


void printInfoLog(GLhandleARB obj)
{
  int infologLength = 0;
  char *infoLog;

  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
			    &infologLength);

  if (infologLength > 0) {
    infoLog = new char[infologLength];
    glGetInfoLogARB(obj, infologLength, &infologLength, infoLog);
    cout << infoLog << endl;
    delete [] infoLog;
  }
}

bool setShader(GLhandleARB p, const char *srcfile, GLenum type) {
  GLhandleARB shader = glCreateShaderObjectARB(type);
  char *shaderText = textFileRead(srcfile);
  if (!shaderText) {
    cout << "File " << srcfile << " failed to load." << endl;
    return false;
  }
  glShaderSourceARB(shader, 1, (const char**)(&shaderText),NULL);
  free(shaderText);

  glCompileShaderARB(shader);
  GLint compiled = 0;
  glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
  if (!compiled) {
    printInfoLog(shader);
    return false;
  }

  glAttachObjectARB(p,shader);

  glDeleteObjectARB(shader);

  return true;
}

bool linkAndValidateShader(GLhandleARB p) {
  glLinkProgramARB(p);

  GLint linked;
  glGetObjectParameterivARB(p, GL_OBJECT_LINK_STATUS_ARB, &linked);
  if (!linked) {
    cout << "Link failed: " << endl;
    printInfoLog(p);
    return false;
  }

  glValidateProgramARB(p);
  GLint validated;
  glGetObjectParameterivARB(p, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
  if (!validated) {
    cout << "Validation failed: " << endl;
    printInfoLog(p);
    return false;
  }

  return true;
}

Shader::Shader(string vertProg, string fragProg) {
  static int tempId = 0;
  shadeId = tempId;
  ++tempId;
  // compile link and validate shader programs
  shadersFailed = false;
  program = glCreateProgramObjectARB();
  if (!setShader(program, vertProg.c_str(), GL_VERTEX_SHADER_ARB))
    shadersFailed = true;
  if (!setShader(program, fragProg.c_str(), GL_FRAGMENT_SHADER_ARB))
    shadersFailed = true;
  if (!linkAndValidateShader(program))
    shadersFailed = true;

  if (shadersFailed) {
    cout << "Shaders failed to initialize correctly" << endl;
  }

  //By default, shaders are on
  shadersOn = true;
}

GeometryShader::GeometryShader(string vertProg, string fragProg) : Shader(vertProg, fragProg){
  displacementEnabled = true;

  glUniform1iARB(glGetUniformLocationARB(program, "heightMap"), 1);
  glUniform1iARB(glGetUniformLocationARB(program, "normalMap"), 2);

  tangentAttrib = glGetAttribLocationARB(program, "tangent");
  bitangentAttrib = glGetAttribLocationARB(program, "bitangent");
  displacementEnabledUniform = glGetUniformLocationARB(program, "displacementEnabled");
}

ShadowShader::ShadowShader(string vertProg, string fragProg) : GeometryShader(vertProg, fragProg) {
  bumpMapEnabled = true;
  textureMapEnabled = true;
  phongEnabled = true;
  displacementEnabled = true;
  shadowMapEnabled = true;
  ambientOcclusionEnabled = true;
  pcfEnabled = true;
  //envEnabled = false;
  dispAmbientLayer = false;

  xPixelOffset = 0.01;
  yPixelOffset = 0.01;

  glUniform1iARB(glGetUniformLocationARB(program, "textureMap"), 0);
  glUniform1iARB(glGetUniformLocationARB(program, "skyMap"), 3);

  bumpMapEnabledUniform = glGetUniformLocationARB(program, "bumpMapEnabled");
  textureMapEnabledUniform = glGetUniformLocationARB(program, "textureMapEnabled");
  phongEnabledUniform = glGetUniformLocationARB(program, "phongEnabled");

  shadowMapUniform = glGetUniformLocationARB(program,"shadowMap");
  shadowMapEnabledUniform = glGetUniformLocationARB(program,"shadowMapEnabled");
  ambientOcclusionEnabledUniform = glGetUniformLocationARB(program,"ambientOcclusionEnabled");
  dispAmbientLayerUniform = glGetUniformLocationARB(program,"dispAmbientLayer");
  pcfEnabledUniform = glGetUniformLocationARB(program,"pcfEnabled");
  //envEnabledUniform = glGetUniformLocationARB(program,"envEnabled");

  
  xPixelOffsetUniform = glGetUniformLocationARB(program,"xPixelOffset");
  yPixelOffsetUniform = glGetUniformLocationARB(program,"yPixelOffset");
}

BlurShader::BlurShader(string vertProg, string fragProg) : Shader(vertProg, fragProg) {
  scaleUa = 0.0;
  scaleUb = 0.0;
  textureSource = 0;

  scaleUniform = glGetUniformLocationARB(program, "ScaleU");
  //textureSourceUniform = glGetUniformLocationARB(program, "textureSource");
  glUniform1iARB(glGetUniformLocationARB(program, "textureSource"), 0);
}

void Shader::setUniformValues() {
  glUseProgramObjectARB(program);
}

void GeometryShader::setUniformValues() {
  Shader::setUniformValues();
  glUniform1iARB(displacementEnabledUniform, displacementEnabled);
}

void ShadowShader::setUniformValues() {
  GeometryShader::setUniformValues();
  glUniform1iARB(bumpMapEnabledUniform, bumpMapEnabled);
  glUniform1iARB(textureMapEnabledUniform, textureMapEnabled);
  glUniform1iARB(phongEnabledUniform, phongEnabled);
  glUniform1iARB(shadowMapEnabledUniform, shadowMapEnabled);
  glUniform1iARB(ambientOcclusionEnabledUniform, ambientOcclusionEnabled);
  glUniform1iARB(dispAmbientLayerUniform, dispAmbientLayer);
  glUniform1iARB(pcfEnabledUniform, pcfEnabled);
  glUniform1fARB(xPixelOffsetUniform, xPixelOffset);
  glUniform1fARB(yPixelOffsetUniform, yPixelOffset);
  glUniform1iARB(envEnabledUniform, envEnabled);
}

void BlurShader::setUniformValues() {
  Shader::setUniformValues();
  glUniform2fARB(scaleUniform,scaleUa,scaleUb);
  glUniform1iARB(textureSourceUniform, textureSource);
}
