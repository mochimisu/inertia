#include "shaders.h"

// from www.lighthouse3d.com
char *textFileRead(const char *fn) {
  FILE *fp;
  char *content = NULL;

  int count=0;

  if (fn != NULL) {
#ifdef WIN32
	fopen_s(&fp, fn, "rt");
#else
    fp = fopen(fn,"rt");
#endif

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

  tangentAttrib = glGetAttribLocationARB(program, "tangent");
  bitangentAttrib = glGetAttribLocationARB(program, "bitangent");
}

ShadowShader::ShadowShader(string vertProg, string fragProg) : GeometryShader(vertProg, fragProg) {
  xPixelOffset = 0.01;
  yPixelOffset = 0.01;
}

BlurShader::BlurShader(string vertProg, string fragProg) : Shader(vertProg, fragProg) {
  scaleUa = 0.0;
  scaleUb = 0.0;
  textureSource = 0;

  scaleUniform = glGetUniformLocationARB(program, "ScaleU");
}

ScatterShader::ScatterShader(string vertProg, string fragProg) : GeometryShader(vertProg, fragProg) {
  exposure = 0.5;
  decay = 0.5;
  density = 0.5;
  weight = 0.5;
  lightPositionOnScreen = vec2(0.5,0.5);
  texture = 0;

  exposureUniform = glGetUniformLocationARB(program, "exposure");
  decayUniform = glGetUniformLocationARB(program, "decay");
  densityUniform = glGetUniformLocationARB(program, "density");
  weightUniform = glGetUniformLocationARB(program, "weight");
  lightPositionOnScreenUniform = glGetUniformLocationARB(program, "lightPositionOnScreen");
  textureUniform = glGetUniformLocationARB(program, "myTexture");
  
}

void Shader::setUniformValues() {
  glUseProgramObjectARB(program);
}

void GeometryShader::setUniformValues() {
  Shader::setUniformValues();
  glUniform1iARB(glGetUniformLocationARB(program, "heightMap"), 1);
  glUniform1iARB(glGetUniformLocationARB(program, "normalMap"), 2);
}

void ShadowShader::setUniformValues() {
  GeometryShader::setUniformValues();
  glUniform1iARB(glGetUniformLocationARB(program, "textureMap"), 0);

  glUniform1iARB(glGetUniformLocationARB(program, "shadowMap"), 7);
}

void BlurShader::setUniformValues() {
  Shader::setUniformValues();
  glUniform1iARB(glGetUniformLocationARB(program, "textureSource"), 0);
  glUniform2fARB(scaleUniform,scaleUa,scaleUb);
  glUniform1iARB(textureSourceUniform, textureSource);
}

void ScatterShader::setUniformValues() {
  GeometryShader::setUniformValues();
  glUniform1iARB(exposureUniform, exposure);
  glUniform1iARB(decayUniform, decay);
  glUniform1iARB(densityUniform, density);
  glUniform1iARB(weightUniform, weight);
  glUniform2fARB(lightPositionOnScreenUniform, lightPositionOnScreen[0], lightPositionOnScreen[1]);
  glUniform1iARB(textureUniform, texture);
}
