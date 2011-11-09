/*
 * Sweep.h
 *
 *  Created on: March 19, 2009
 *      Author: jima (referencing jfhamlin) 
 */

#ifndef SWEEP_H_
#define SWEEP_H_


#include <cmath>
#include <vector>
#include <map>

#include "algebra3.h"
//#include "main.h"
#include "TrackGenerator.h"
#include "Cityscape.h"
#include "shaders.h"

#include "LoadImage.h"

using namespace std;



template<typename Pt> 
Pt sampleBSpline(const vector<Pt>& cps, double t, bool closed = true, int degree = 3);


// class to render and sample track
class Sweep {
 public:
  Sweep();



  GLuint setSky(string *filenames) {
    loadCube(filenames, skyMap);
    return skyMap;
  }

	void Generate();
	void Regenerate();

  // renders the coaster
  void render(GeometryShader &shade, int pathSamplesPerPt, double crossSectionScale=.2, int xsectSamplesPerPt=3);
  // renders the coaster with a cache.  Ignores parameters after display list is set; use clearDisplayList() before updating parameters
  void renderWithDisplayList(GeometryShader &shade, int pathSamplesPerPt, bool renderCity, double crossSectionScale=.2, int xsectSamplesPerPt=3);
  void clearDisplayList();

  // --- these functions can provide a local frame along the track
  // sample the curve at a point
  PathPoint sample(double t);
  // get the forward direction
  vec3 sampleForward(double t, double step = .001);
  // get the forward direction, with some additional search if the first approximation turns out to be zero
  vec3 sampleForward_nonzero(double t, double step = .01, int searchdist = 50);
  // get the up direction
  vec3 sampleUp(double t, double step = .01);

  mat4 tbnHomogenizedBasis(double t, vec3 worldLoc = vec3(0), double step = 0.01);
  mat3 tbnBasis(double t, double step = 0.01);

  // use this to check for an invalid (empty) coaster
  bool bad() {
    return (pathPts.size() == 0);
  }

private:
  vector<PathPoint> pathPts; // control points for the sweep path
  double globalTwist; // twists the whole frame
  double globalAzimuth; // global azimuth rotates the whole frame
  vector<vec2> profilePts; // control points for cross section to be swept
  Cityscape* cscape;

  // display list for caching sweep geometry
  //y hashmap not a c++ standard :<
  map<int, GLuint> shaderDL;
  bool renderingCity;

  GLuint texture, normalMap, heightMap, skyMap; // texture data
  int lengthRepeats, widthRepeats;

  // --- internal helper functions
  void renderSweep(GeometryShader &shade, vector<PathPoint> &pts, vector<pair<vec2, double> > &profile, double crossSectionScale);
  vec3 getFirstUp(); // helper to get initial frame (default to frenet, fallback to 'up=+Y')
  // rotates vector from RMF to account for twist, azimuth, etc:
  void orientVectorInFrame(const vec3 &dir, double percent, double localAz, vec3 &inFrame);

};


#endif

