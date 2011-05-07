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

Cityscape::Cityscape(int xWidth, int zWidth, int area) {
	rootBSD = new BuildingSubdivision(-xWidth / 2, -xWidth / 2 + xWidth, -zWidth / 2, -zWidth / 2 + zWidth, 0.0, 64);
	hasDL = false;
    this->xWidth = xWidth;
    this->zWidth = zWidth;
}

void Cityscape::render() {
	/*if (!hasDL) {
        DLid = glGenLists(1);
        glNewList(DLid, GL_COMPILE);
		glColor3f(0.8, 0.8, 0.8);

		renderBuildingSubdivision(rootBSD);

        glEndList();
		hasDL = true;
	}
	glCallList(DLid);*/
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

			// Missing setting the normals
            //top
            //cout<< bsd->xMin << ", " << bsd->xMin<<endl;

            double distanceSpacing = 1.5;

            glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);
			glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMin+distanceSpacing); 
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);

            //back
            glVertex3d(bsd->xMax-distanceSpacing, 0, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, 0, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMin+distanceSpacing);

            //left side
            glVertex3d(bsd->xMin+distanceSpacing, 0, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, 0, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMin+distanceSpacing);

            //right side
            glVertex3d(bsd->xMax-distanceSpacing, 0, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMin+distanceSpacing);
            glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMax-distanceSpacing, 0, bsd->zMax-distanceSpacing);

            //front
            glVertex3d(bsd->xMax-distanceSpacing, 0, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMax-distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, bsd->height, bsd->zMax-distanceSpacing);
            glVertex3d(bsd->xMin+distanceSpacing, 0, bsd->zMax-distanceSpacing);
            
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
