// vim: ts=2:sw=2:softtabstop=2

class Vehicle {
  public:
    void setAccelerate(bool is Accelerating);
    void draw(); // handles translation on its own
  private:
    bool isAccelerating;
    mat4 location; // location based on whackness
};
