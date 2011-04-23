// vim: ts=2:sw=2:softtabstop=2

#include "sweep.h"

/* This is a Vehicle class. Operate it by initializing it with a sweep.
 * Then you can set the initial location on the sweep. Lastly from time
 * to time you should setTime on it to inform it of the new time. When 
 * it receives a time, it will automatically compute where it should 
 * have moved to since the last time update.
 */
#ifndef VEHICLE_H
#define VEHICLE_H
class Vehicle {
  public:
    Vehicle(Sweep * sweep, mat4 startLocation, vec3 startDirection);
    void setAccelerate(bool isAccelerating);
    void draw(); // handles translation on its own
    void setLocation(mat4 location);
    void setDirection(mat4 direction);
    void setVelocity(double velocity);
    void setTime(double newTime);
    void setSweepTime(double newSweepTime);
    void turnLeft();
    void turnRight();
    vec3 getPerspectiveLocation();
    vec3 getPerspectiveCenter();
    vec3 getPerspectiveUp();
    mat4 getCurrentLocation();
    inline void setGravity(double gravity) { this->gravity = gravity; }
    inline void setH(double h) { this->h = h; }
    inline double getVelocity() { return this->velocity.length(); }
    inline double getAcceleration2() { return this->getAcceleration().length(); }
    inline mat4 getR() { return this->R; }
    vec3 uVec();
  private:
    mat4 location; // location based on whackness
    mat4 R; // stores FUR matrix for faster retrieval?
    vec3 direction; // just in case?
    vec3 velocity;
    bool isAccelerating;
    Sweep * sweep;
    double lastTime; // the last real time, for velocity calculations
    double lastSweepTime; // the last time on the parametrized sweep
    vec3 getAcceleration();
    double getTime(double distance);
    double h;
    double gravity;
};

#endif
