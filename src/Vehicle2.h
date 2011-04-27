
#ifndef VEHICLE2_H
#define VEHICLE2_H

#include "sweep.h"
#include "mesh.h"
#include "shaders.h"
#include "algebra3.h"
#include "functions.h"

class Vehicle2 {
 public:
  Vehicle(Sweep * sweep, Mesh * mesh);
  void draw(GeometryShader * shade);

  void setAccel(vec3 accel);
  void setVelocity(vec3 vel);

  void setAccelScalar(double mag);
  void setVelocityScalar(double mag);

  void step(double amoutn);

  vec3 worldSpacePos();

 private:

  vec3 resistanceAccel();
  vec3 getWindResistance();

  vec3 pos; //TBN coordinates: T is time along sweep, B is lateral, N is distance on normal from sweep
  vec3 velocity;
  vec3 acceleration; //WITHOUT wind resistance
  
  Sweep * sweep;
  Mesh * mesh;


}

#endif
