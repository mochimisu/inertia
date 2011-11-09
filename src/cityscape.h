/*
* The city landscape generator.
*
* Implements greebling
*
*/

#ifndef CITYSCAPE_H_
#define CITYSCAPE_H_

#define CARVE_RADIUS 2.0

#include <vector>
#include "global.h"

using namespace std;

/*
* BuildingSubdivision is the the data structure that stores
* the subdivisions of the cityscape, and once the subdivisions
* get small enough, terminate the subdivision process.
*/
struct BuildingSubdivision {
  BuildingSubdivision* one;
  BuildingSubdivision* two;
  double xMin;
  double xMax;
  double zMin;
  double zMax;
  double height;
  bool leaf;
  bool draw;

  BuildingSubdivision(int setXMin, int setXMax, int setZMin, int setZMax, double setHeight, int area);
};

/*
* The Cityscape class stores the root BuildingSubdivision and
* other things for use as the actual cityscape, like rendering.
*/
class Cityscape {
public:
  Cityscape(int xWidth, int zWidth, int area);
  void render();
  void carve(vector<vec3> carveAway);

private:
  void renderBuildingSubdivision(BuildingSubdivision* bsd);
  double getAverageHeight(BuildingSubdivision* bsd);
  void setHeights(BuildingSubdivision* bsd, double scale);
  void carveHelper(vec3 point, BuildingSubdivision* bsd);

  BuildingSubdivision* rootBSD;
  GLuint DLid;
  GLuint myTexture; // for now, all buildings look the same
  GLuint myHeightTexture;
  GLuint myNormalTexture;
  bool hasDL;

  float xWidth;
  float zWidth;

};

#endif
