/*
 * The city landscape generator.
 *
 * Implements greebling
 *
 */

#ifndef CITYSCAPE_H_
#define CITYSCAPE_H_

#include <vector>
#include "main.h"

using namespace std;

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

	BuildingSubdivision(int setXMin, int setXMax, int setZMin, int setZMax, double setHeight, int area) {
		xMin = setXMin;
		xMax = setXMax;
		zMin = setZMin;
		zMax = setZMax;

		int xWidth = xMax - xMin;
		int zWidth = zMax - zMin;
		
		if (zWidth * xWidth < area) {
			leaf = true;
			draw = true;
			height = setHeight;
		}
		else {
			leaf = false;
			if (zWidth > xWidth) {
				int division = (rand() % (zWidth - 1)) + 1 + zMin;
				one = new BuildingSubdivision(xMin, xMax, zMin, division, double(rand() % zWidth) / zWidth * 5 + setHeight, area);
				two = new BuildingSubdivision(xMin, xMax, division, zMax, double(rand() % zWidth) / zWidth * 5 + setHeight, area);
			}
			else {
				int division = (rand() % (xWidth - 1)) + 1 + xMin;
				one = new BuildingSubdivision(xMin, division, zMin, zMax, double(rand() % xWidth) / xWidth * 5 + setHeight, area);
				two = new BuildingSubdivision(division, xMax, zMin, zMax, double(rand() % xWidth) / xWidth * 5 + setHeight, area);
			}
		}
	}
};

class Cityscape {
public:
	Cityscape(int width, int height, int area);
	void render();
	void cull(double x, double z);
private:
	void renderBuildingSubdivision(BuildingSubdivision* bsd);
	void cullHelper(double x, double z, BuildingSubdivision* bsd);
	BuildingSubdivision* rootBSD;
    GLuint DLid;
    bool hasDL;
};

#endif