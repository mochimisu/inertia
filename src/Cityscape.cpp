#include "Cityscape.h"

Cityscape::Cityscape(int width, int height, int area) {
	rootBSD = new BuildingSubdivision(-width / 2, -width / 2 + width, -height / 2, -height / 2 + height, 0.0, 64);
	hasDL = false;
}

void Cityscape::render() {
	if (!hasDL) {
        DLid = glGenLists(1);
        glNewList(DLid, GL_COMPILE);
		glColor3f(0.8, 0.8, 0.8);

		renderBuildingSubdivision(rootBSD);

        glEndList();
		hasDL = true;
	}
	glCallList(DLid);
}

void Cityscape::renderBuildingSubdivision(BuildingSubdivision* bsd) {
	if (bsd->leaf) {
		/*glBegin(GL_QUADS);

		// Missing setting the normals

		glVertex3d(bsd->right, bsd->top, bsd->height);
		glVertex3d(bsd->left, bsd->top, bsd->height);
		glVertex3d(bsd->left, bsd->bottom, bsd->height);
		glVertex3d(bsd->right, bsd->bottom, bsd->height);
		
		glVertex3d(bsd->left, bsd->top, bsd->height);
		glVertex3d(bsd->right, bsd->top, bsd->height);
		glVertex3d(bsd->right, bsd->top, 0);
		glVertex3d(bsd->left, bsd->top, 0);
		
		glVertex3d(bsd->right, bsd->bottom, bsd->height);
		glVertex3d(bsd->right, bsd->top, bsd->height);
		glVertex3d(bsd->right, bsd->top, 0);
		glVertex3d(bsd->right, bsd->bottom, 0);
		
		glVertex3d(bsd->left, bsd->bottom, bsd->height);
		glVertex3d(bsd->right, bsd->bottom, bsd->height);
		glVertex3d(bsd->right, bsd->bottom, 0);
		glVertex3d(bsd->left, bsd->bottom, 0);
		
		glVertex3d(bsd->left, bsd->top, bsd->height);
		glVertex3d(bsd->left, bsd->bottom, bsd->height);
		glVertex3d(bsd->left, bsd->bottom, 0);
		glVertex3d(bsd->left, bsd->top, 0);

		glEnd();
		*/
		if (bsd->draw) {
			double xWidth = bsd->xMax - bsd->xMin;
			double zWidth = bsd->zMax - bsd->zMin;

			glPushMatrix();
			glTranslated(bsd->xMin + xWidth / 2, bsd->height / 2, bsd->zMin + zWidth / 2);
			glScaled(xWidth, bsd->height, zWidth);
			glutSolidCube(1.0);
			glPopMatrix();
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