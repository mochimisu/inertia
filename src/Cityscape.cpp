#include "Cityscape.h"

bool loadTexture(string filename, GLuint &texture);
bool loadHeightAndNormalMaps(string filename, GLuint &heightmap, GLuint &normalmap, double zScale);

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

  // Normalize buildings
  setHeights(rootBSD, 20.0 / getAverageHeight(rootBSD));

  this->xWidth = xWidth;
  this->zWidth = zWidth;

  loadTexture("building.png", this->myTexture);
  loadHeightAndNormalMaps("building_height.png", this->myHeightTexture, this->myNormalTexture, 0.1);
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

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, myTexture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, myHeightTexture);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, myNormalTexture);
      glBegin(GL_QUADS);

      //top
      glNormal3d(0, 1, 0);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMin); 
      glVertex3d(bsd->xMin, bsd->height, bsd->zMin);
      glVertex3d(bsd->xMin, bsd->height, bsd->zMax);

      //back
      glNormal3d(0, 0, -1);
      glTexCoord2f(1,bsd->height);
      glVertex3d(bsd->xMax, 0, bsd->zMin);
      glTexCoord2f(0,bsd->height);
      glVertex3d(bsd->xMin, 0, bsd->zMin);
      glTexCoord2f(0, 0);
      glVertex3d(bsd->xMin, bsd->height, bsd->zMin);
      glTexCoord2f(1, 0);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMin);

      //left side
      glNormal3d(-1, 0, 0);
      glTexCoord2f(0,bsd->height);
      glVertex3d(bsd->xMin, 0, bsd->zMin);
      glTexCoord2f(1,bsd->height);
      glVertex3d(bsd->xMin, 0, bsd->zMax);
      glTexCoord2f(1,0);
      glVertex3d(bsd->xMin, bsd->height, bsd->zMax);
      glTexCoord2f(0,0);
      glVertex3d(bsd->xMin, bsd->height, bsd->zMin);

      //right side
      glNormal3d(1, 0, 0);
      glTexCoord2f(0,bsd->height);
      glVertex3d(bsd->xMax, 0, bsd->zMin);
      glTexCoord2f(0,0);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMin);
      glTexCoord2f(1,0);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
      glTexCoord2f(1,bsd->height);
      glVertex3d(bsd->xMax, 0, bsd->zMax);

      //front
      glNormal3d(0, 0, 1);
      glTexCoord2f(0,bsd->height);
      glVertex3d(bsd->xMax, 0, bsd->zMax);
      glTexCoord2f(0,0);
      glVertex3d(bsd->xMax, bsd->height, bsd->zMax);
      glTexCoord2f(1,0);
      glVertex3d(bsd->xMin, bsd->height, bsd->zMax);
      glTexCoord2f(1,bsd->height);
      glVertex3d(bsd->xMin, 0, bsd->zMax);

      glEnd();
    }

  }
  else {
    renderBuildingSubdivision(bsd->one);
    renderBuildingSubdivision(bsd->two);
  }
}

double Cityscape::getAverageHeight(BuildingSubdivision* bsd) {
  if (bsd->leaf) {
    return bsd->height;
  }
  double areaTotal = (bsd->xMax - bsd->xMin) * (bsd->zMax - bsd->zMin);
  BuildingSubdivision* one = bsd->one;
  BuildingSubdivision* two = bsd->two;
  double oneTotal = (one->xMax - one->xMin) * (one->zMax - one->zMin) * getAverageHeight(one);
  double twoTotal = (two->xMax - two->xMin) * (two->zMax - two->zMin) * getAverageHeight(two);
  return (oneTotal + twoTotal) / areaTotal;
}

void Cityscape::setHeights(BuildingSubdivision* bsd, double scale) {
  if (bsd->leaf) {
    bsd->height = min(scale * bsd->height, 55.0);
  }
  else {
    setHeights(bsd->one, scale);
    setHeights(bsd->two, scale);
  }
}

void Cityscape::carve(vector<vec3> carveAway) {
  for (unsigned int i = 0; i < carveAway.size(); i++) {
    carveHelper(carveAway[i], rootBSD);
  }
}

void Cityscape::carveHelper(vec3 point, BuildingSubdivision* bsd) {
  double xPt = point[0];
  double zPt = point[2];
    
  double xBuilding = xPt;
  double zBuilding = zPt;
  if (xPt < bsd->xMin) {
    xBuilding = bsd->xMin;
  }
  else if (xPt > bsd->xMax) {
    xBuilding = bsd->xMax;
  }
  if (zPt < bsd->zMin) {
    zBuilding = bsd->zMin;
  }
  else if (zPt > bsd->zMax) {
    zBuilding = bsd->zMax;
  }
    
  xPt -= xBuilding;
  zPt -= zBuilding;

  if (xPt * xPt + zPt * zPt < CARVE_RADIUS * CARVE_RADIUS) {
    if (bsd->leaf) {
      if (bsd->height > point[1] - CARVE_RADIUS) {
        bsd->height = fmod(double(rand()), point[1] - CARVE_RADIUS);
      }
      return;
    }
    else {
      carveHelper(point, bsd->one);
      carveHelper(point, bsd->two);
    }
  }
}