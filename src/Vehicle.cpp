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
  this->acceleration = this->sweep->sampleForward(0,0.01);
  this->up = this->sweep->sampleUp(0);
  this->up.normalize();

  this->velocityScalar = 0;
  this->accelerationScalar = 0;
  this->tbn = this->sweep->tbnBasis(0);
  this->turnCurrentValue = 0.0;
  this->airBrake = 0.0;
  this->lap = 1;
  this->energy = 100.0;

  this->bestLapTime = -1;
  this->lapStartTime = 0;

  this->turnTargetValue = 0;
  this->turnCurrentValue = 0;
}

void Vehicle::draw(GeometryShader * shade) {
	shade->toggleDisplacement();
	glCullFace(GL_FRONT);
  mesh->draw(*shade);
  glCullFace(GL_BACK);
  shade->toggleDisplacement();
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
  
  //turn
  //Linear term because TURNING_INERTIA loses effect at small amounts and you "drift" for a while. It approaches asymptotic zero, but slower than a linear term. Without a linear term, you drift for a long time, especially at smaller amount values. tl;dr: keep it.
  turnCurrentValue = turnTargetValue - pow(TURNING_INERTIA, amount) * (turnTargetValue - turnCurrentValue) * 0.98;
  quat qRot = quat::axisAngle(up, turnCurrentValue*amount);
  acceleration = qRot.rotate(acceleration);

  //update the local state variables
  tbn = this->sweep->tbnBasis(this->pos[0]);  

  vec3 tbnVelocityDir = tbn.inverse() * velocity;
  tbnVelocityDir.normalize();
  vec3 tbnVelocity = tbnVelocityDir * velocityScalar;

  vec3 tbnAccelerationDir = tbn.inverse() * acceleration;
  tbnAccelerationDir.normalize();
  vec3 tbnAcceleration = tbnAccelerationDir * accelerationScalar;

  //approximate normal force by projecting velocity onto tb plane and set that as velocity
  vec3 tbVelocityDir = vec3(tbnVelocityDir[0], 0, tbnVelocityDir[2]);
  vec3 tbVelocity = vec3(tbnVelocity[0], 0, tbnVelocity[2]);

  vec3 tbAccelerationDir = vec3(tbnAccelerationDir[0], 0, tbnAccelerationDir[2]);
  vec3 tbAcceleration = vec3(tbnAcceleration[0], 0, tbnAcceleration[2]);

  //make acceleration's normal axis the same as tb's projection
  acceleration = tbn * tbAccelerationDir;

  vec3 newTbVelocity = 0.99*tbVelocity + tbAcceleration + (-(0.00002+airBrake) * velocityScalar * velocityScalar * tbVelocity) ;


  //lateral movement
  float bLateralDisp = pos[2] + newTbVelocity[2]*amount;
  pos[2] = bLateralDisp;

  if(bLateralDisp > 1.25) {
    pos[2] = 1.25;
    energy -= newTbVelocity[2] / 2.0;
    newTbVelocity[0] *= 0.8;
    newTbVelocity[2] = 0;
  }
  if(bLateralDisp < -1.25) {
    pos[2] = -1.25;
    energy -= -newTbVelocity[2] / 2.0;
    newTbVelocity[0] *= 0.8;
    newTbVelocity[2] = 0;
  }

  velocityScalar = newTbVelocity.length();
  if(velocityScalar > 0) {

  vec3 newTbVelocityDir = newTbVelocity;
  newTbVelocityDir.normalize();

  velocity = tbn * newTbVelocityDir;

  //make sure acceleration's normal component lines up with tangent of track
  vec3 tbnAlignedAcceleration = tbn.inverse() * acceleration;
  tbnAlignedAcceleration[2] = 0;

  }

  float distance = amount *  newTbVelocity[0];

  double tempDist = 0;
  vec3 tempPos = sweep->sample(pos[0]).point;
  //step [todo: better method]
  float tempTime = pos[0];
  if(distance > 0.00001) {
    do {
      tempTime += 0.0000025;
      //tempTime = tempTime > 1 ? tempTime-1:tempTime;
      tempDist += (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist < distance);
    if(pos[0] >= lap) {
      ++lap;
      int newTime = glutGet(GLUT_ELAPSED_TIME);
      int curLapTime = newTime - lapStartTime;
      if(curLapTime < bestLapTime || bestLapTime == -1) {
        bestLapTime = curLapTime;
      }
      lapStartTime = newTime;
    }
  } else if(distance < -0.00001) {
    do {
      tempTime -= 0.0000025;
      //tempTime = tempTime <0  ? tempTime+1:tempTime;
      tempDist -= (sweep->sample(tempTime).point - tempPos).length();

      tempPos = sweep->sample(tempTime).point;
    } while (tempDist > distance);
  }

  pos[0] = tempTime;

  

  //find sweep location @ new time
  vec3 sweepLocNew = this->sweep->sample(this->pos[0]).point + tbn*vec3(0,0,pos[2]) + tbn*vec3(0,0.5,0);
  
  //now reconstruct the worldPos
  worldPos = sweepLocNew;

  up = this->sweep->sampleUp(pos[0]);
  up.normalize();

}

vec3 Vehicle::cameraPos() {
  //return sweep->sample(pos[0]-0.002).point + up;
  return worldPos + tbn*vec3(-1,0.5,0);
}

vec3 Vehicle::cameraLookAt() {
  //mat3 tbn = this->sweep->tbnBasis(this->pos[0]);
  //vec3 tbnVelocity = tbn * velocity;
  //return sweep->sample(pos[0]+0.02).point;
  return worldPos + tbn*vec3(8,0,0);
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
    accelerationScalar = 0.1;
}

void Vehicle::setAccel(float acl) {
  accelerationScalar = acl;
}

void Vehicle::turnLeft(double amt) {
  //acceleration += vec3(0,0,0.1);
  //quat qRot =  quat::axisAngle(up,amt);
  //acceleration = qRot.rotate(acceleration);
  turnTargetValue = amt;
}
void Vehicle::turnRight(double amt) {
  //acceleration += vec3(0,0,-0.1);
  //quat qRot =  quat::axisAngle(up,-amt);
  //acceleration = qRot.rotate(acceleration);
  turnTargetValue = -amt;
}
void Vehicle::turnStraight() {
  acceleration = vec3(1,0,0);
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
  vec3 up = sweep->sampleUp(pos[0]);
  up.normalize();

  vec3 forward = acceleration;
  forward.normalize();

  vec3 side = up ^ forward;
  side.normalize();
  
  vec3 uprime = forward ^ side;
  uprime.normalize();

  return mat4(vec4(forward,0),
	      vec4(uprime,0),
	      vec4(side,0),
	      vec4(0,0,0,1));
}

vec3 Vehicle::lightPos() {
    return worldPos + vec3(0,2,10);
}
