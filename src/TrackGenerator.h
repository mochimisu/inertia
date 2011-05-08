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
#include <algorithm>
#include "algebra3.h"

#define RAND (double(rand()) / double(RAND_MAX))

/*
 * Misc declarations
 */
struct TrackTreeNode;
int newBranches(int current_depth);

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

	TrackTreeNode* getNextNode(TrackTreeNode* current) {
		vector<TrackTreeNode*>::iterator it = find(neighbors.begin(), neighbors.end(), current);
		it++;
		if (it == neighbors.end()) {
			it = neighbors.begin();
		}
		return *it;
	}

	TrackTreeNode(double x, double z, TrackTreeNode* root, int current_depth) {
		xz = vec2(x, z);
		radius = 15;
		if (root) {
			neighbors.push_back(root);
		}

		int branches = newBranches(current_depth);

		for (int i = 0; i < branches; i++) {
			vec2 newXZ;
			double angle;
			do {
				angle = RAND * 2 * M_PI;
				newXZ = xz + vec2(cos(angle), sin(angle));
			} while (!allGood(newXZ));

			double length = RAND * 40 + 20;
			radius = min(radius, length / 3);
			TrackTreeNode* newNode = new TrackTreeNode(xz[0] + length * cos(angle), xz[1] + length * sin(angle), this, current_depth + 1);
			neighbors.push_back(newNode);
		}
		sort(neighbors.begin(), neighbors.end(), TrackTreeComparator(xz));
	}

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
	TrackGenerator() {
		/*
		 * Phase 1 -
		 * Generate the track tree and place control points that wrap
		 * closely around the tree segments.
		 */

		// Start by generating the tree
		TrackTreeNode* rootTrackTreeNode = new TrackTreeNode(0.0, 0.0, NULL, 0);
	
		// Place the control points
		TrackTreeNode* current_start = rootTrackTreeNode;
		TrackTreeNode* previous_start = rootTrackTreeNode->neighbors[0];

		TrackTreeNode* current = current_start;
		TrackTreeNode* previous = previous_start;

		do {
			TrackTreeNode* next = current->getNextNode(previous);
		
			if (previous == next) { // happens if current node only has one segment
				vec2 direction = current->xz - previous->xz;
				direction.normalize();
				direction *= current->radius;
				vec2 offset(-direction[1], direction[0]);

				vec2 temp = current->xz + direction + offset;
				vec3 v(temp[0], 0.0, temp[1]);
				controlPts.push_back(v);
				temp = current->xz + direction - offset;
				v = vec3(temp[0], 0.0, temp[1]);
				controlPts.push_back(v);
			}
			else { // Otherwise, do the bisector thing
				vec2 direction1 = current->xz - previous->xz;
				direction1.normalize();
				direction1 = vec2(-direction1[1], direction1[0]);
				vec2 direction2 = next->xz - current->xz;
				direction2.normalize();
				direction2 = vec2(-direction2[1], direction2[0]);

				vec2 newDir = direction1 + direction2;

				double angle = acos(direction1 * direction2);

				newDir.normalize();
				newDir *= current->radius / cos(angle / 2);
				newDir += current->xz;
				
				vec3 v(newDir[0], 0.0, newDir[1]);
				controlPts.push_back(v);
			}

			// Advance
			previous = current;
			current = next;
		} while (current != current_start || previous != previous_start);
		
		/*
		 * Phase 2 -
		 * Merge nearby control points and prune control points with angles
		 * that are too acute. Repeat until the track stops changing.
		 */
		
		bool changed = true;
		while (changed) {
			changed = false;
			vector<vec3> tempControlPts;

			// Merge close control points first
			double threshold = 16;
			int size = controlPts.size();
			for (int i = 0; i < size; i++) {
				vec3 current = controlPts[i];
				vec3 previous = controlPts[(i + size - 1) % size];
				vec3 next = controlPts[(i + 1) % size];

				vec3 distvec1 = current - previous;
				vec3 distvec2 = next - current;
				double dist1 = distvec1.length();
				double dist2 = distvec2.length();
				if (dist1 > threshold && dist2 > threshold) {
					tempControlPts.push_back(current);
				}
				else if (dist2 <= threshold) {
					changed = true;
					vec3 newPoint = (current + next) / 2;
					tempControlPts.push_back(newPoint);
				}
			}
			controlPts = tempControlPts;

			// Next, prune sharply angled control points
			tempControlPts.clear();
			size = controlPts.size();
			for (int i = 0; i < size; i++) {
				vec3 current = controlPts[i];
				vec3 previous = controlPts[(i + size - 1) % size];
				vec3 next = controlPts[(i + 1) % size];
				
				vec3 dir1 = current - previous;
				vec3 dir2 = current - next;
				dir1.normalize();
				dir2.normalize();
				if (acos(dir1 * dir2) < M_PI / 4) {
					changed = true;
				}
				else {
					tempControlPts.push_back(current);
				}
			}
			controlPts = tempControlPts;
		}

		/*
		 * Phase 3 -
		 * Add height information and then adjust to make sure that
		 * the track will not overlap too closely.
		 */

		// First, randomly assign heights
		for (unsigned int i = 0; i < controlPts.size(); i++) {
			controlPts[i][1] = RAND * 20 + 5;
		}

		// TODO - compare segment distances to make sure that they aren't overalapping too closely


		/*
		 * Recenter the track
		 */
		vector<vec3>::iterator it = controlPts.begin();
		double xMin = (*it)[0];
		double xMax = (*it)[0];
		double zMin = (*it)[2];
		double zMax = (*it)[2];
		it++;
		while (it != controlPts.end()) {
			xMin = min(xMin, (*it)[0]);
			xMax = max(xMax, (*it)[0]);
			zMin = min(zMin, (*it)[2]);
			zMax = max(zMax, (*it)[2]);
			it++;
		}
		double xMid = (xMax + xMin) / 2;
		double zMid = (zMax + zMin) / 2;
		for (unsigned int i = 0; i < controlPts.size(); i++) {
			controlPts[i][0] -= xMid;
			controlPts[i][2] -= zMid;
		}
		
		xWidth = xMax - xMin;
		zWidth = zMax - zMin;
	}

	vector<vec3> getControlPts() { return controlPts; }
	double getXWidth() { return xWidth; }
	double getZWidth() { return zWidth; }

private:
	vector<vec3> controlPts;
	double xWidth;
	double zWidth;
};

#endif