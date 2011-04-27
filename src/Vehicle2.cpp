#include "Vehicle.h"

Vehicle::Vehicle(Sweep * sw, Mesh * ms) {
  this->sweep = sw;
  this->mesh = ms;

  //todo: initial velocity and accel
}

void Vehicle::draw(GeometryShader * shade) {
  mesh->draw(*shade);
}

void Vehicle::setAccel(vec3 accel) {
  this->acceleration = accel;
}

void Vehicle::setAccel(vec3 vel) {
  this->velocity = vel;
}

void Vehicle::setAccelScalar(double mag) {
  this->acceleration.normalize();
  this->acceleration *= mag;
}

void Vehicle::setVelocityScalar(double mag) {
  this->velocity.normalize();
  this->veloicty *= mag;
}

void Vehicle::step(double amount) {
  mat4 tbn = sweep->tbnBasis(pos[0]
}

vec3 Vehicle::resistanceAccel() {
  return this->acceleration - this->getWindResistance();
}

vec3 Vehicle::getWindResistance() {
  //Drag Force = -1/2 * density * Area * Drag Coeff ~[0.25-0.45] * (velocity dot velocity) * velocity/magvelocity
  //simplified: F =  - SOME COEFF * velocity

  //density of air @ 20C = 1.204 kg/m^3
  //drag coeff of car ~0.25->0.45 [2010 impreza WRX: 0.36]
  //frontal area of car 0.717 m^2 [1993 impreza]

  return -(0.15538824) * vel;
}
