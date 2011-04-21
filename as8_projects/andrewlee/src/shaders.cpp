#include "shaders.h"

extern GLint inverseOrientation;

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

    // start with all the nice settings on
    shadersOn = true;
    bumpMapEnabled = true;
    textureMapEnabled = true;
	environmentEnabled = false;

    // set up variables for the shaders to use
    glUseProgramObjectARB(program);
    glUniform1iARB(glGetUniformLocationARB(program, "textureMap"), 0);
    glUniform1iARB(glGetUniformLocationARB(program, "heightMap"), 1);
    glUniform1iARB(glGetUniformLocationARB(program, "normalMap"), 2);
	
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxNorth"), 4);
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxSouth"), 5);
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxEast"), 6);
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxWest"), 7);
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxUp"), 8);
    glUniform1iARB(glGetUniformLocationARB(program, "skyboxDown"), 9);

    bumpMapEnabledUniform = glGetUniformLocationARB(program, "bumpMapEnabled");
    textureMapEnabledUniform = glGetUniformLocationARB(program, "textureMapEnabled");
    phongEnabledUniform = glGetUniformLocationARB(program, "phongEnabled");
    displacementEnabledUniform = glGetUniformLocationARB(program, "displacementEnabled");
    environmentEnabledUniform = glGetUniformLocationARB(program, "environmentEnabled");
	inverseOrientation = glGetUniformLocationARB(program, "inverseOrientation");
    tangentAttrib = glGetAttribLocationARB(program, "tangent");
    bitangentAttrib = glGetAttribLocationARB(program, "bitangent");
}

