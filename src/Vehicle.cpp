// vim: ts=2:sw=2:softtabstop=2

#include "algebra3.h"
#include "functions.h"
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

// gets the next SWEEP time based on the current SWEEP time
// unlike the one form my as7, does NOT update sweep time on its own.
double Vehicle::getTime() {
  vec3 lastPos = vec3((location.transpose())[3], VW);
  vec3 lastPos = this->sweep->sample(lastSweepTime).point;
  double time = lastSweepTime;
	//double time = lastTime;
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
  this->location = startLocation;
  this->direction = startDirection;
  this->velocity = startDirection; // assume velocity starts off that way
  this->lastTime = 0;
  this->lastSweepTime = 0;
}

void Vehicle::setAccelerate(bool isAccelerating) {
  this->isAccelerating = isAccelerating?1:0;
}

void Vehicle::draw() {

  //cout << "Begin error dumping" << endl;
  //cout << glGetError() << endl;
  //glPushMatrix();
  //cout << glGetError() << endl;
  //applyMat4(this->location.transpose());

  //glutSolidTeapot's faces are backwards
  glFrontFace(GL_CW);
  //cout << glGetError() << endl;
  //glTranslatef(100000,0,0);
  //cout << glGetError() << endl;
  glutSolidTeapot(5);


  glFrontFace(GL_CCW);
  //glPopMatrix();
}

void Vehicle::setLocation(mat4 location) {
  this->location = location;
}

void Vehicle::setTime(double newTime) {
  cerr << "warning: whackness" << endl;
  double delta = newTime - lastTime;
  vec3 velocityScaled = this->velocity * delta;
  this->location = mat4(vec4(1,0,0,velocityScaled[0]), vec4(0,1,0,velocityScaled[1]), vec4(0,0,1,velocityScaled[2]), vec4(0,0,0,1)) * this->location;
  this->velocity = delta * this->getAcceleration() + this->velocity;
  this->lastTime = newTime;
}

void Vehicle::setSweepTime(double newSweepTime) {
  cerr << "warning: whackness" << endl;
  double delta = newSweepTime - lastSweepTime;
  vec3 velocityScaled = this->velocity * delta;
  vec3 temp = this->sweep->sample(newSweepTime).point;
  this->location = mat4(vec4(1,0,0,temp[0]), vec4(0,1,0,temp[1]), vec4(0,0,1,temp[2]), vec4(0,0,0,1));

  //double time = getTime(vec3((this->location).transpose()[2], VW), 10, 10, sweep, lastSweepTime);
  double time = newSweepTime;
  vec3 fVec = f(this->sweep, time-.05);
	vec3 rVec = r(this->sweep, time-.05);
	vec3 uPrimeVec = uPrime(this->sweep, time-.05);
	PathPoint loc = this->sweep->sample(time-.05);
	fVec.normalize();
	uPrimeVec.normalize();
	rVec.normalize();
	mat4 R(vec4(fVec, 0), vec4(uPrimeVec, 0), vec4(rVec, 0), vec4(0,0,0,1));
  //this->location = R;
  //this->location = mat4(vec4(1,0,0,velocityScaled[0]), vec4(0,1,0,velocityScaled[1]), vec4(0,0,1,velocityScaled[2]), vec4(0,0,0,1)) * this->location;
  this->velocity = delta * this->getAcceleration() + this->velocity;
  this->lastSweepTime = newSweepTime;
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


mat4 Vehicle::getCurrentLocation() {
  return this->location;
}

void Vehicle::setVelocity(double velocity) {
  this->velocity = velocity * this->direction;
}