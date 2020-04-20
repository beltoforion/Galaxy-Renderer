#ifndef _VECTOR_H
#define _VECTOR_H

//------------------------------------------------------------------------------
struct Vec2D {
public:
  Vec2D(double x = 0, double y = 0);
  double x;
  double y;
};

//------------------------------------------------------------------------------
struct Vec3D {
public:
  Vec3D(double x = 0, double y = 0, double z = 0);
  double x;
  double y;
  double z;
};

#endif //_VECTOR_H
