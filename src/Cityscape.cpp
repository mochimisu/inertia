#include "Cityscape.h"

BuildingSubdivision::BuildingSubdivision(int setXMin, int setXMax, int setZMin, int setZMax, double setHeight, int area) {
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
		xMin += min(1.0, xWidth * 0.1);
		xMax -= min(1.0, xWidth * 0.1);
		zMin += min(1.0, zWidth * 0.1);
		zMax -= min(1.0, zWidth * 0.1);
	}
	else {
		leaf = false;
		if (zWidth > xWidth) {
			int division = (rand() % (zWidth - 3)) + 2 + zMin;
			one = new BuildingSubdivision(xMin, xMax, zMin, division, double(rand() % zWidth) / zWidth * 5 + setHeight, area);
			two = new BuildingSubdivision(xMin, xMax, division, zMax, double(rand() % zWidth) / zWidth * 5 + setHeight, area);
		}
		else {
			int division = (rand() % (xWidth - 3)) + 2 + xMin;
			one = new BuildingSubdivision(xMin, division, zMin, zMax, double(rand() % xWidth) / xWidth * 5 + setHeight, area);
			two = new BuildingSubdivision(division, xMax, zMin, zMax, double(rand() % xWidth) / xWidth * 5 + setHeight, area);
		}
	}
}

Cityscape::Cityscape(int xWidth, int zWidth, int area) {
	rootBSD = new BuildingSubdivision(-xWidth / 2, -xWidth / 2 + xWidth, -zWidth / 2, -zWidth / 2 + zWidth, 0.0, 64);
	hasDL = false;
    this->xWidth = xWidth;
    this->zWidth = zWidth;
}

void Cityscape::render() {
	glBegin(GL_QUADS);
    glVertex3d(xWidth/2, 0, zWidth/2);
    glVertex3d(xWidth/2, 0, -zWidth/2);
    glVertex3d(-xWidth/2, 0, -zWidth/2); 
    glVertex3d(-xWidth/2, 0, zWidth/2);   
    glEnd();
    
	renderBuildingSubdivision(rootBSD);
}

void Cityscape::renderBuildingSubdivision(BuildingSubdivision* bsd) {
	if (bsd->leaf) {
      if (bsd->draw) {
			glBegin(GL_QUADS);
			
            //top
			glNormal3d(0, 1, 0);
            glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
			glVertex3d(bsd->xMax, bsd->height, bsd->zMin); 
            glVertex3d(bsd->xMin, bsd->height, bsd->zMin);
            glVertex3d(bsd->xMin, bsd->height, bsd->zMax);

            //back
			glNormal3d(0, 0, -1);
            glVertex3d(bsd->xMax, 0, bsd->zMin);
            glVertex3d(bsd->xMin, 0, bsd->zMin);
            glVertex3d(bsd->xMin, bsd->height, bsd->zMin);
            glVertex3d(bsd->xMax, bsd->height, bsd->zMin);

            //left side
			glNormal3d(-1, 0, 0);
            glVertex3d(bsd->xMin, 0, bsd->zMin);
            glVertex3d(bsd->xMin, 0, bsd->zMax);
            glVertex3d(bsd->xMin, bsd->height, bsd->zMax);
            glVertex3d(bsd->xMin, bsd->height, bsd->zMin);

            //right side
			glNormal3d(1, 0, 0);
            glVertex3d(bsd->xMax, 0, bsd->zMin);
            glVertex3d(bsd->xMax, bsd->height, bsd->zMin);
            glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
            glVertex3d(bsd->xMax, 0, bsd->zMax);

            //front
			glNormal3d(0, 0, 1);
            glVertex3d(bsd->xMax, 0, bsd->zMax);
            glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
            glVertex3d(bsd->xMin, bsd->height, bsd->zMax);
            glVertex3d(bsd->xMin, 0, bsd->zMax);
            
            glEnd();
		}

	}
	else {
		renderBuildingSubdivision(bsd->one);
		renderBuildingSubdivision(bsd->two);
	}
}

void Cityscape::cull(double x, double z) {
	cullHelper(x, z, rootBSD);
}

void Cityscape::cullHelper(double x, double z, BuildingSubdivision* bsd) {
	if (bsd->leaf) {
		bsd->draw = false;
	}
	else {
		BuildingSubdivision* one = bsd->one;
		BuildingSubdivision* two = bsd->two;
		if (one->xMin < x &&
			x < one->xMax &&
			one->zMin < z &&
			z < one->zMax
			) {
			cullHelper(x, z, one);
		}
		else {
			cullHelper(x, z, two);
		}
	}
}
