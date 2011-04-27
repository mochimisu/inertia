
#ifndef VEHICLE2_H
#define VEHICLE2_H

#include "sweep.h"
#include "mesh.h"
#include "shaders.h"
#include "algebra3.h"
#include "functions.h"

class Mesh;

class Vehicle2 {
 public:
  Vehicle2(Sweep * sweep);
  void draw(GeometryShader * shade);

  void setAccel(vec3 accel);
  void setVelocity(vec3 vel);

  void setAccelScalar(double mag);
  void setVelocityScalar(double mag);

  void step(double amount);

  vec3 worldSpacePos();
  vec3 getVelocity();
  vec3 getAcceleration();
  vec3 getUp();

  Mesh * mesh;

  vec3 cameraPos();

  void turnLeft();
  void turnRight();
  void turnStraight();
  void toggleAcceleration();

 private:

  vec3 resistanceAccel();
  vec3 getWindResistance();
  void updateWorldPos();

  vec3 accelNorm() { vec3 normAccel = acceleration; normAccel.normalize(); return normAccel; }; //need because accel is not normalized automatically
  vec3 accelNormDirection(); //accel's Y should follow tangent's Y world space

  vec3 worldPos;
  vec3 pos; //TBN coordinates: T is time along sweep, B is lateral, N is distance on normal from sweep
  vec3 velocity; // Velocity and accel are normalized vectors maintaining direction
  vec3 acceleration; // for when either are zero in magnitude
  //(maybe use a quaternion just for kicks?)
  float velocityScalar;
  float accelerationScalar;

  vec3 up; //"cached" up value
  
  Sweep * sweep;


};

#endif
