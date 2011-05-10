/*
 * The track generator.
 *
 * Three primary phases to track generation:
 * 1. Create a random 2D tree, and then curve a B-spline around
 *    the the tree
 * 2. Merge closeby adjacent control points and prune control
 *    points with sharp angles, repeating until nothing changes
 * 3. Jiggle the heights and then adjust until the overlapping
 *    sections are far enough apart for no intersection
 */

#ifndef TRACK_GENERATOR_H_
#define TRACK_GENERATOR_H_

#include <vector>
#include "algebra3.h"

#define RAND (double(rand()) / double(RAND_MAX))

/*
 * Misc declarations
 */
struct TrackTreeNode;
int newBranches(int current_depth);

/*
 * PathPoint MOVED HERE
 */

// control point for the sweep defining the track
struct PathPoint {
  vec3 point;
  double azimuth;
  double scale;

  PathPoint() {}
  PathPoint(vec3 pt, double az = 0, double s = 1) : point(pt), azimuth(az), scale(s) {}
};

/*
 * TrackTreeComparator is used to make a comparator function to
 * sort the neighbors at each TrackTreeNode. In fancy terms, the
 * struct provides a closure in which to encapsulate the pivot
 * position.
 */
struct TrackTreeComparator {
	vec2 xz;

	TrackTreeComparator(vec2 setXZ) : xz(setXZ) {}

	bool operator() (TrackTreeNode* t1, TrackTreeNode* t2);
};

/*
 * TrackTreeNode is the struct to hold the nodes of the tree
 * that the track will be based around. 
 */
struct TrackTreeNode {
	vec2 xz;
	double radius;
	vector<TrackTreeNode*> neighbors;

	bool allGood(vec2 newXZ);
	TrackTreeNode* getNextNode(TrackTreeNode* current);
	TrackTreeNode(double x, double z, TrackTreeNode* root, int current_depth);

	/*
	void render(TrackTreeNode* drawnFrom) {
		glPushMatrix();
		glTranslated(xz[0], y, xz[1]);
		glColor3f(1, 1, 1);
		glutSolidSphere(0.5, 16, 16);
		glPopMatrix();
		
		for (int i = 0; i < neighbors.size(); i++) {
			if (neighbors[i] == drawnFrom) {
				continue;
			}
			TrackTreeNode* child = neighbors[i];
			glBegin(GL_LINES);
			glColor3f(i % 3 == 0 ? 1 : 0, i % 3 == 1 ? 1 : 0, i % 3 == 2 ? 1 : 0);
			glVertex3d(xz[0], y, xz[1]);
			int j = 0;
			for (j = 0; j < child->neighbors.size(); j++) {
				if (child->neighbors[j] == this) {
					break;
				}
			}
			glColor3f(j % 3 == 0 ? 1 : 0, j % 3 == 1 ? 1 : 0, j % 3 == 2 ? 1 : 0);
			glVertex3d(child->xz[0], child->y, child->xz[1]);
			glEnd();
			child->render(this);
		}
	}*/
};

/*
 * TrackGenerator will perform the track generation and store the
 * generated control points for the B-spline that sweep can use.
 */
class TrackGenerator {
public:
	TrackGenerator();
	vector<PathPoint> getControlPts() { return pathPts; }
	double getXWidth() { return xWidth; }
	double getZWidth() { return zWidth; }

private:
	vector<PathPoint> pathPts;
	double xWidth;
	double zWidth;
};

#endif