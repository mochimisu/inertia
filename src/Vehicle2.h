
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
  Mesh * mesh;

  vec3 cameraPos();

 private:

  vec3 resistanceAccel();
  vec3 getWindResistance();
  void updateWorldPos();

  vec3 worldPos;
  vec3 pos; //TBN coordinates: T is time along sweep, B is lateral, N is distance on normal from sweep
  vec3 velocity;
  vec3 acceleration; //WITHOUT wind resistance
  
  Sweep * sweep;


};

#endif
