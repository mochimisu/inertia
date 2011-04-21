// vim: ts=2:sw=2:softtabstop=2

#include "algebra3.h"
#include "Vehicle.h"

// use the sweep to get F vector
// time is the sweep time, not real time  
vec3 f(Sweep * sweep, double time) {
	return sweep->sampleForward(time);
}

// use the sweep to get U vector
// time is sweep time, not real time
vec3 u(Sweep * sweep, double time) {
	return sweep->sampleUp(time);
}

// use the sweep to get R vector
// time is sweep time, not real time
vec3 r(Sweep * sweep, double time) {
	return f(sweep, time) ^ u(sweep, time);
}

// use F and R to recompute U
// time is sweep time, not real time
vec3 uPrime(Sweep * sweep, double time) {
	return r(sweep, time) ^ f(sweep, time);
}

// gets the next sweep time based on the current sweep time
double getTime(vec3 lastPos, double h, double gravity, Sweep * sweep, double lastTime) {
	double time = lastTime;
	double curVelocity = sqrt(gravity*(h - lastPos[VY]));
	double tempDist = 0;
	vec3 tempPos = lastPos;
	//cout << endl<< curVelocity << endl;
	do {
		time += 0.0000025;
		time = time > 1?time-1:time;
		tempDist += (sweep->sample(time).point - tempPos).length();

		tempPos = sweep->sample(time).point;
		//cout << time << endl;
	} while (tempDist < curVelocity);
	//cout << tempDist << endl;
	lastPos = sweep->sample(time).point;
	lastTime = time;
	//vec3 tempPos = sweep->sample(time).point;
	return lastTime;
}

class Vehicle;

Vehicle::Vehicle(Sweep * sweep, mat4 startLocation, vec3 startDirection) {
  this->sweep = sweep;
  this->location = location;
  this->direction = direction;

}

void Vehicle::setAccelerate(bool isAccelerating) {
  this->isAccelerating = isAccelerating?1:0;
}

void Vehicle::draw() {

  //glutSoldiTeapot's faces are backwards
  glFrontFace(GL_CW);
  glutSolidTeapot(100);
  glFrontFace(GL_CCW);
}

void Vehicle::setLocation(mat4 location) {
  this->location = location;
}

void Vehicle::setTime(double newTime) {
  double delta = newTime - lastTime;
  vec3 velocityScaled = this->velocity * delta;
  this->location = mat4(vec4(1,0,0,velocityScaled[0]), vec4(0,1,0,velocityScaled[1]), vec4(0,0,1,velocityScaled[2]), vec4(0,0,0,1)) * this->location;
  this->velocity = delta * this->getAcceleration() + this->velocity;
}

void Vehicle::turnLeft() {
  cerr << "Left turns not implemented" << endl;
}

void Vehicle::turnRight() {
  cerr << "Right turns not implemented" << endl;
}

vec3 Vehicle::getAcceleration() {
  if (this->isAccelerating) {
    // TODO: add friction everywhere, actually
    return vec3(1,0,0) * exp(-velocity.length());
  } else {
    return vec3(0,0,0); // TODO: add some resistance here, or some other friction
  }
}

vec3 Vehicle::getPerspectiveLocation() {
  // need 6 values: location.xyz and center.xyz.
  double time; // TODO: set time
  time = 0;
  return this->sweep->sample(time).point;
}

vec3 Vehicle::getPerspectiveCenter() {
  double time; // TODO: set time
  time = 0;
  vec3 location = this->getPerspectiveLocation();
  vec3 fVec = f(this->sweep, time);
  fVec = this->sweep->sampleForward(time, 0.05);
  vec3 uVec = u(this->sweep, time);
  return location + fVec;
}

vec3 Vehicle::uVec() {
  double time;
  time = 0;
  return u(this->sweep, time);
}
