#include "TrackGenerator.h"

#include <algorithm>

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
* Returns 0 if the line segments are far enough apart.
* Returns 1 if the first segment is on top.
* Returns -1 if the second segment is on top.
*/
vec3 checkSegments(pair<vec3, vec3> seg1, pair<vec3, vec3> seg2) {
  if (seg1.first == seg2.second ||
      seg1.second == seg2.first) {
      return vec3(0);
  }

  vec3 P0 = seg1.first;
  vec3 Q0 = seg2.first;
  vec3 u = seg1.second - P0;
  vec3 v = seg2.second - Q0;
  vec3 w0 = P0 - Q0;
  double a = u * u;
  double b = u * v;
  double c = v * v;
  double d = u * w0;
  double e = v * w0;
  
  double s = (b*e - c*d) / (a*c - b*b);
  double t = (a*e - b*d) / (a*c - b*b);

  if (0.0 <= s && s <= 1.0 &&
      0.0 <= t && t <= 1.0) {
    vec3 dist = (P0 + s*u) - (Q0 + t*v);
    if (dist.length() < MIN_DISTANCE) {
      return (MIN_DISTANCE - dist.length()) * dist.normalize();
    }
  }
  return vec3(0);
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
* A method that tests if a candidate point is okay.
*/
bool TrackTreeNode::allGood(vec2 newXZ) {
  for (unsigned int i = 0; i < neighbors.size(); i++) {
    if (angleBetween(xz, neighbors[i]->xz, newXZ) < M_PI / 3) {
      return false;
    }
  }
  return true;
}

/*
* A method that returns the node that follows the given one
*/
TrackTreeNode* TrackTreeNode::getNextNode(TrackTreeNode* current) {
  vector<TrackTreeNode*>::iterator it = find(neighbors.begin(), neighbors.end(), current);
  it++;
  if (it == neighbors.end()) {
    it = neighbors.begin();
  }
  return *it;
}

/*
* The constructor for the TrackTreeNode
*/
TrackTreeNode::TrackTreeNode(double x, double z, TrackTreeNode* root, int current_depth) {
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




TrackGenerator::TrackGenerator() {
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
  
  vector<vec3> controlPts;

  do {
    TrackTreeNode* next = current->getNextNode(previous);

    if (previous == next) { // happens if current node only has one segment
      vec2 direction = current->xz - previous->xz;
      direction.normalize();
      direction *= current->radius;
      vec2 offset(-direction[1], direction[0]);

      vec2 temp = current->xz + direction + offset;
      vec3 v(temp[0], RAND * 20 + 10, temp[1]);
      controlPts.push_back(v);
      temp = current->xz + direction - offset;
      v = vec3(temp[0], RAND * 20 + 10, temp[1]);
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

      // First, randomly assign heights
      vec3 v(newDir[0], RAND * 20 + 10, newDir[1]);
      controlPts.push_back(v);
    }

    // Advance
    previous = current;
    current = next;
  } while (current != current_start || previous != previous_start);
  
  bool majorChange = true;

  while (majorChange) {
    majorChange = false;

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
      double threshold = 20.0;
      int size = controlPts.size();
      for (int i = 0; i < size; i++) {
        vec3 current = controlPts[i];
        vec3 previous = controlPts[(i + size - 1) % size];
        vec3 next = controlPts[(i + 1) % size];

        vec3 distvec1 = current - previous;
        vec3 distvec2 = next - current;
        distvec1[1] = 0.0;
        distvec2[1] = 0.0;
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
        majorChange |= changed;
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
        majorChange |= changed;
      }
      controlPts = tempControlPts;
    }

    /*
    * Phase 3 -
    * Add height information and then adjust to make sure that
    * the track will not overlap too closely.
    */

    // TODO - compare segment distances to make sure that they aren't overalapping too closely
    changed = true;
    while (changed) {
      changed = false;

      for (unsigned int i = 0; i < controlPts.size() - 1; i++) {
        for (unsigned int j = i + 1; j < controlPts.size(); j++) {
          unsigned int jn = (j+1)%controlPts.size();
          pair<vec3, vec3> seg1(controlPts[i], controlPts[i+1]);
          pair<vec3, vec3> seg2(controlPts[j], controlPts[jn]);
          vec3 check = checkSegments(seg1, seg2);
          if (check.length() > 0.01) {
            changed = true;
            controlPts[i] += check / 2;
            controlPts[i+1] += check / 2;
            controlPts[j] -= check / 2;
            controlPts[jn] -= check / 2;
            controlPts[i][1] = max(controlPts[i][1], 4.0);
            controlPts[i+1][1] = max(controlPts[i+1][1], 4.0);
            controlPts[j][1] = max(controlPts[j][1], 4.0);
            controlPts[jn][1] = max(controlPts[jn][1], 4.0);
          }
        }
      }
      
      majorChange |= changed;
    }
  }

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

  /*
  * Add banking to make better turns, and add to pathPts
  */
  for (unsigned int i = 0; i < controlPts.size(); i++) {
    vec2 current = vec2(controlPts[i], VY);
    vec2 prev = vec2(controlPts[(i - 1 + controlPts.size()) % controlPts.size()], VY);
    vec2 next = vec2(controlPts[(i + 1) % controlPts.size()], VY);

    double angle = angleBetween(current, prev, next);
    vec2 dir = current - prev;
    vec2 turn = next - current;
    
    vec3 dir2(dir[0], 0, dir[1]);
    vec3 turn2(turn[0], 0, turn[1]);

    double bankAmount = 45 * pow(0.999, angle * dir.length() * turn.length() / 100);

    if ((dir2 ^ turn2)[1] > 0) {
      bankAmount *= -1;
    }

    pathPts.push_back(PathPoint(controlPts[i], bankAmount));
  }
}
