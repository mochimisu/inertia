#include "Vehicle.h"

Vehicle::Vehicle(Sweep * sw) {
  this->sweep = sw;
  this->mesh = new Mesh();

  //todo: initial velocity and accel
  //initial pos
  this->worldPos = this->sweep->sample(0).point;
  this->velocity = this->sweep->sampleForward(0,0.01);
  this->velocity.normalize();
  this->pos = vec3(0,0,0);
  this->acceleration = vec3(1,0,0);
  this->up = this->sweep->sampleUp(0);

  this->velocityScalar = 0;
  this->accelerationScalar = 0;
}

void Vehicle::draw(GeometryShader * shade) {
  mesh->draw(*shade);
}

void Vehicle::setAccel(vec3 accel) {
  this->acceleration = accel;
}

void Vehicle::setVelocity(vec3 vel) {
  this->velocity = vel;
}

void Vehicle::setAccelScalar(double mag) {
  this->acceleration.normalize();
  this->acceleration *= mag;
}

void Vehicle::setVelocityScalar(double mag) {
  this->velocity.normalize();
  this->velocity *= mag;
}

void Vehicle::step(double amount) {
  mat3 tbn = this->sweep->tbnBasis(this->pos[0]);

  vec3 tbnVelocityDir = tbn.inverse() * velocity;
  tbnVelocityDir.normalize();

  
  vec3 tbnVelocity = tbnVelocityDir * velocityScalar;

  //approximate normal force by projecting velocity onto tb plane and set that as velocity
  vec3 tbVelocityDir = vec3(tbnVelocityDir[0], tbnVelocityDir[1], 0);
  vec3 tbVelocity = vec3(tbnVelocity[0], tbnVelocity[1], 0);



  //acceleration is relative to velocity and basis from velocityxup
  vec3 vsnAccelDir = acceleration;
  vsnAccelDir.normalize();
  vec3 tbWorldVelocityDir = tbn * tbVelocityDir;
  tbWorldVelocityDir.normalize();
  vec3 curUp = sweep->sampleUp(pos[0],0.01);
  mat3 vsn = mat3(tbWorldVelocityDir,
		  up,
		  up ^ tbWorldVelocityDir).transpose();
  vec3 worldAccelDir = vsn * vsnAccelDir;
  vec3 tbnAccelDir = tbn.inverse() * worldAccelDir;
  vec3 tbnAcceleration = tbnAccelDir * accelerationScalar;

  tbnAccelDir = vec3(1,0,0);
  tbnAcceleration = accelerationScalar * vec3(1,0,0);

  vec3 tbAcceleration = accelerationScalar * vec3(tbnAccelDir[0], tbnAccelDir[1], 0);

  vec3 newTbVelocity = tbVelocity + tbAcceleration + (-0.000002 * velocityScalar * velocityScalar * tbVelocity) ;

  velocityScalar = newTbVelocity.length();
  if(velocityScalar > 0) {

  vec3 newTbVelocityDir = newTbVelocity;
  newTbVelocityDir.normalize();

  velocity = tbn * newTbVelocityDir;

  }

  float distance = amount *  newTbVelocity[0];

  double tempDist = 0;
  vec3 tempPos = sweep->sample(pos[0]).point;
  //step [todo: better method]
  float tempTime = pos[0];
  if(distance > 0.00001) {
    do {
      tempTime += 0.0000025;
      tempTime = tempTime > 1 ? tempTime-1:tempTime;
      tempDist += (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist < distance);
  } else if(distance < -0.00001) {
    do {
      tempTime -= 0.0000025;
      tempTime = tempTime <0  ? tempTime+1:tempTime;
      tempDist -= (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist > distance);
  }
  this->pos[0] = tempTime;

  //find sweep location @ new time
  vec3 sweepLocNew = this->sweep->sample(this->pos[0]).point + tbn*vec3(0,0.5,0);
  
  //now reconstruct the worldPos
  worldPos = sweepLocNew;

  up = this->sweep->sampleUp(pos[0]);

}

vec3 Vehicle::cameraPos() {
  return worldPos - 2*velocity + up;
}

vec3 Vehicle::cameraLookAt() {
  return worldPos + 5*velocity;
}

vec3 Vehicle::worldSpacePos() {
  return worldPos;
}

vec3 Vehicle::getUp() {
  return up;
}


void Vehicle::toggleAcceleration() {
  if(accelerationScalar > 0.0001)
    accelerationScalar = 0.0;
  else
    accelerationScalar = 1.0;
}

void Vehicle::setAccel(float acl) {
  accelerationScalar = acl;
}

void Vehicle::turnLeft() {
  acceleration += vec3(0,0,0.1);
}
void Vehicle::turnRight() {
  acceleration += vec3(0,0,-0.1);
}
void Vehicle::turnStraight() {
  acceleration = vec3(1,0,0);
}

vec3 Vehicle::getWindResistance() {
  //Drag Force = -1/2 * density * Area * Drag Coeff ~[0.25-0.45] * (velocity dot velocity) * velocity/magvelocity
  //simplified: F =  - SOME COEFF * velocity^2

  //density of air @ 20C = 1.204 kg/m^3
  //drag coeff of car ~0.25->0.45 [2010 impreza WRX: 0.36]
  //frontal area of car 0.717 m^2 [1993 impreza]


  //and just dividing by a few thousand to get a higher terminal velocity
  return -(0.0015538824) * velocityScalar * velocityScalar * velocity;
}

 void Vehicle::updateWorldPos() {

}

//temp for debug
vec3 Vehicle::getVelocity() {
  return velocity;
}

vec3 Vehicle::getAcceleration() {
  return accelNorm();
}

mat4 Vehicle::orientationBasis() {
  mat3 tbn = this->sweep->tbnBasis(this->pos[0]);

  
  vec3 up = sweep->sampleUp(pos[0]);
  up.normalize();

  vec3 tbnVelocityDir = tbn.inverse() * velocity;
  vec3 tbVelocityDir = vec3(tbnVelocityDir[0], tbnVelocityDir[1], 0);
  vec3 tbWorldVelocityDir = tbn * tbVelocityDir;
  tbWorldVelocityDir.normalize();
  mat3 vsn = mat3(tbWorldVelocityDir,
		  up,
		  up ^ tbWorldVelocityDir).transpose();


  vec3 vsnAccelDir = acceleration;
  vsnAccelDir.normalize();
  vec3 worldAccelDir = vsn * vsnAccelDir;
  

  vec3 side = up ^ worldAccelDir;

  side.normalize();
  vec3 uprime = worldAccelDir ^ side;
  uprime.normalize();



  return mat4(vec4(worldAccelDir,0),
	      vec4(uprime,0),
	      vec4(side,0),
	      vec4(0,0,0,1));
}
