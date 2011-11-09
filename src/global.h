/*
 * global.h
 *
 *  Created on: Feb 2, 2009
 *      Author: njoubert
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define IMPLEMENT_ME(file,line)    cout << "METHOD NEEDS TO BE IMPLEMENTED AT " << file << ":" << line << endl; exit(1);

//===RENDER CONSTANTS
const float shadowMapCoef = 0.5;
const float blurCoef = 0.5;
const float lightScatteringCoef = 0.5;

const float renderWidth = 1024.0;
const float renderHeight = 768.0;
const float shadowMapWidth = renderWidth * shadowMapCoef;
const float shadowMapHeight = renderHeight * shadowMapCoef;
const float shadowMapBlurWidth = shadowMapWidth * blurCoef;
const float shadowMapBlurHeight = shadowMapHeight * blurCoef;
const float lightScatterWidth = renderWidth * lightScatteringCoef;
const float lightScatterHeight = renderHeight * lightScatteringCoef;

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <vector>
#include <cmath>

#ifdef OSX
#include <GL/glew.h>
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
//... I DON'T HAVE ANY IDEA IF THIS IS RIGHT
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glu.h>
#endif


//Include our math library
#include <algebra3.h>
//Include our Image Saving library.
#include "UCB/ImageSaver.h"
#include "FreeImage.h"

#include "functions.h"

// Sound support
#include <stdlib.h>
#include <al.h>
#include <alc.h>
#include <AL/alut.h>



#endif /* GLOBAL_H_ */
