#include "TrackGenerator.h"

/*
 * General function to determine the angle between two points
 * at a pivot point
 */
double angleBetween(vec2 pivot, vec2 leg1, vec2 leg2) {
	leg1 -= pivot;
	leg2 -= pivot;
	return acos((leg1 * leg2) / (leg1.length() * leg2.length()));
}

/*
 * A randomizer function that determines the number of new
 * branches to form for the track tree
 */
int newBranches(int current_depth) {
	if (current_depth < 2) {
		int i = int(RAND * 3);
		return i + 1;
	}
	else if (current_depth > 4) {
		return 0;
	}
	else {
		return int(RAND * 3);
	}
}

/*
 * The implementation of the TrackTreeComparator, so that the
 * neighbors of a TrackTreeNode can be sorted in clockwise order
 */
bool TrackTreeComparator::operator() (TrackTreeNode* t1, TrackTreeNode* t2) {
	vec2 dueEast(1, 0);
	double a1 = angleBetween(xz, t1->xz, xz + dueEast);
	if (t1->xz[1] > xz[1]) {
		a1 = 2 * M_PI - a1;
	}
	double a2 = angleBetween(xz, t2->xz, xz + dueEast);
	if (t2->xz[1] > xz[1]) {
		a2 = 2 * M_PI - a2;
	}
	return a1 < a2;
}

/*
 * A function that tests if a candidate point is okay.
 */
bool TrackTreeNode::allGood(vec2 newXZ) {
	for (unsigned int i = 0; i < neighbors.size(); i++) {
		if (angleBetween(xz, neighbors[i]->xz, newXZ) < M_PI / 3) {
			return false;
		}
	}
	return true;
}
