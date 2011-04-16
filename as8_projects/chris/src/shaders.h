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
		free(vertText);
		free(fragText);
    }

    inline void toggleShader() { shadersOn = !shadersOn; }
    inline void toggleBumpMap() { bumpMapEnabled = !bumpMapEnabled; }
    inline void toggleTextureMap() { textureMapEnabled = !textureMapEnabled; }
    inline void togglePhong() { phongEnabled = !phongEnabled; }
	inline void toggleDisplacement() { 
		displacementEnabled = !displacementEnabled; 
	}
	inline void toggleToonShading() {
		toonShadingEnabled = !toonShadingEnabled;
#ifdef _DEBUG
		cout << "Toon shading: " << toonShadingEnabled << endl;
#endif
	}

    GLint getTangentAttrib() { return tangentAttrib; }
    GLint getBitangentAttrib() { return bitangentAttrib; }

    bool on() { return shadersOn; }

    void set(bool forceOff = false) {
        if (!forceOff && on()) {
			glDeleteObjectARB(program);
			this->constructorHelper(vertProg, fragProg);
            glUseProgramObjectARB(program);
            //glUniform1iARB(bumpMapEnabledUniform, bumpMapEnabled);
            //glUniform1iARB(textureMapEnabledUniform, textureMapEnabled);
            //glUniform1iARB(phongEnabledUniform, phongEnabled);
			//glUniform1iARB(displacementEnabledUniform, displacementEnabled);
			//glUniform1iARB(toonShadingEnabledUniform, toonShadingEnabled);
        } else {
            glUseProgramObjectARB(0);
        }
    }

private:
    bool shadersFailed;
    GLhandleARB program; // shaders
    GLint tangentAttrib, bitangentAttrib; // tangent space for uploading to shaders
    GLint bumpMapEnabledUniform, textureMapEnabledUniform, phongEnabledUniform, displacementEnabledUniform, toonShadingEnabledUniform;
    bool bumpMapEnabled, textureMapEnabled, phongEnabled, displacementEnabled, toonShadingEnabled;
    bool shadersOn;
	string vertProg, fragProg;
	void constructorHelper(string vertprog, string fragprog);
	bool setShader(GLhandleARB p, const char *srcfile, GLenum type);
	char * fragText, * vertText;
};

#endif 

