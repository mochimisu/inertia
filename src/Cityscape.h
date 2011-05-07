/*
 * The city landscape generator.
 *
 * Implements greebling
 *
 */

#ifndef CITYSCAPE_H_
#define CITYSCAPE_H_

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
	void cull(double x, double z);

private:
	void renderBuildingSubdivision(BuildingSubdivision* bsd);
	void cullHelper(double x, double z, BuildingSubdivision* bsd);

	BuildingSubdivision* rootBSD;
    GLuint DLid;
    bool hasDL;

    float xWidth;
    float zWidth;
};

#endif
