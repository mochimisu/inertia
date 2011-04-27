#include "Vehicle2.h"

Vehicle2::Vehicle2(Sweep * sw) {
  this->sweep = sw;
  this->mesh = new Mesh();

  //todo: initial velocity and accel
  //initial pos
  this->worldPos = this->sweep->sample(0).point;
  this->velocity = this->sweep->sampleForward(0,0.01);
  this->velocity.normalize();
  this->pos = vec3(0,0,0);
}

void Vehicle2::draw(GeometryShader * shade) {
  mesh->draw(*shade);
}

void Vehicle2::setAccel(vec3 accel) {
  this->acceleration = accel;
}

void Vehicle2::setVelocity(vec3 vel) {
  this->velocity = vel;
}

void Vehicle2::setAccelScalar(double mag) {
  this->acceleration.normalize();
  this->acceleration *= mag;
}

void Vehicle2::setVelocityScalar(double mag) {
  this->velocity.normalize();
  this->velocity *= mag;
}

void Vehicle2::step(double amount) {
  //mat4 tbn = this->sweep->tbnBasis(this->pos[0], worldPos);
  mat3 tbn = this->sweep->tbnBasis(this->pos[0]);
  //bring velocity (world coord) into tangent space
  cout << "v " << this->velocity << endl;
  vec3 tbnVelocity = tbn.transpose().inverse() * velocity;
  cout << "tbnv " << tbnVelocity << endl;
  //t coordinate of transformed velocity is what we we step using
  double distance = amount * tbnVelocity[0];

  cout << "dist: " << distance << endl;

  double tempDist = 0;
  vec3 tempPos = sweep->sample(pos[0]).point;
  //step [todo: better method]
  float tempTime = pos[0];
  if(distance > 0) {
    do {
      tempTime += 0.0000025;
      tempTime = tempTime > 1 ? tempTime-1:tempTime;
      tempDist += (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist < distance);
  } else {
    do {
      tempTime -= 0.0000025;
      tempTime = tempTime <0  ? tempTime+1:tempTime;
      tempDist -= (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist > distance);
  }
  
  cout << "time: " << this->pos[0];
  this->pos[0] = tempTime;
  cout << " -> " << this->pos[0] << endl;

  //find sweep location @ new time
  vec3 sweepLocNew = this->sweep->sample(this->pos[0]).point;

  //apply lateral movement
  //calculate tbn space of lateral displacement
  this->pos[1] = (amount * tbnVelocity)[1];// + this->pos[1];
  vec3 lateralPos = vec3(0,this->pos[1],0);
  //transform tbn space lateral displacement into world space
  vec3 lateralDispWorld = tbn * lateralPos;
  cout << "ldispworld " << lateralDispWorld << endl;

  //apply vertical movement
  //calcualte tbn space of vertical displacement
  //ACTUALLy lets keep it at z=1 for right now
  vec3 verticalPos = vec3(0,0,1);
  vec3 verticalDispWorld = tbn * verticalPos;
  cout << "vdispworld " << verticalDispWorld << endl;

  //now reconstruct the worldPos
  worldPos = sweepLocNew + lateralDispWorld + verticalDispWorld;

  //temp velocity
  velocity = sweep->sampleForward(pos[0],0.01);
  velocity.normalize();


}

vec3 Vehicle2::cameraPos() {
  return worldPos - (5 * velocity);
}

vec3 Vehicle2::worldSpacePos() {
  return worldPos;
}

vec3 Vehicle2::resistanceAccel() {
  return this->acceleration - this->getWindResistance();
}

vec3 Vehicle2::getWindResistance() {
  //Drag Force = -1/2 * density * Area * Drag Coeff ~[0.25-0.45] * (velocity dot velocity) * velocity/magvelocity
  //simplified: F =  - SOME COEFF * velocity

  //density of air @ 20C = 1.204 kg/m^3
  //drag coeff of car ~0.25->0.45 [2010 impreza WRX: 0.36]
  //frontal area of car 0.717 m^2 [1993 impreza]

  return -(0.15538824) * this->velocity;
}

 void Vehicle2::updateWorldPos() {

}

//temp for debug
vec3 Vehicle2::getVelocity() {
  return velocity;
}
