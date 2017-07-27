// Copyright ©2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __BSS_COLLISION_H__
#define __BSS_COLLISION_H__

#include "vector.h"

namespace bss {
  /**************** Sphere ****************/

  template<class T, int N>
  inline bool SphereContainsPoint(const Vector<T, N>& p, const Vector<T, N>& center, T radius)
  {
    return NVectorDistanceSq(p.v, center.v) < radius*radius;
  }

  template<class T, int N>
  inline bool SphereContainsSphere(const Vector<T, N>& containerCenter, T containerRadius, const Vector<T, N>& center, T radius)
  {
    if(containerRadius <= radius)
      return false;
    T r = containerRadius - radius;
    return NVectorDistanceSq(containerCenter.v, center.v) < r*r;
  }

  template<class T, int N>
  inline bool SphereSphereCollide(const Vector<T, N>& c1, T r1, const Vector<T, N>& c2, T r2)
  {
    T r = r1 + r2;
    return NVectorDistanceSq(c1.v, c2.v) < r*r;
  }

  /**************** Circle ****************/

  // Gets the intersection points of a circle at the origin and a circle at (X,Y)
  template<class T>
  inline int CircleRadiusIntersect(T r, T X, T Y, T R, Vector<T, 2> (&out)[2])
  {
    T dsq = (X*X) + (Y*Y);
    T maxr = r + R;
    T minr = fabs(r - R);

    if(dsq > maxr*maxr)
      return 0; // circles are disjoint
    if(dsq < minr*minr)
      return 0; // One circle is inside the other

    T id = ((T)1) / bss::FastSqrt(dsq);
    T a = (r*r - (R*R) + dsq)*(id / 2);
    T h = bss::FastSqrt(r*r - (a*a));

    T x2 = X*a*id;
    T y2 = Y*a*id;

    out[0].x = x2 + h*Y*id;
    out[0].y = y2 - h*X*id;
    out[1].x = x2 - h*Y*id;
    out[1].y = y2 + h*X*id;
    return 2;
  }

  // Gets the intersection points of two circles
  template<class T>
  BSS_FORCEINLINE int CircleCircleIntersect(T X1, T Y1, T R1, T X2, T Y2, T R2, Vector<T, 2>(&out)[2])
  {
    return CircleRadiusIntersect<T>(R1, X2 - X1, Y2 - Y1, R2, out);
  }
  template<class T>
  inline bool RadiusContainsPoint(T r, T X, T Y)
  {
    return (X*X + Y*Y) < r*r;
  }
  template<class T>
  BSS_FORCEINLINE bool CircleContainsPoint(T x, T y, T r, T pX, T pY)
  {
    return RadiusContainsPoint(r, pX - x, pY - y);
  }

  // Returns true if R1 contains R2
  template<class T>
  BSS_FORCEINLINE bool CircleContainsCircle(T X1, T Y1, T R1, T X2, T Y2, T R2)
  {
    if(R1 <= R2)
      return false;
    return RadiusContainsPoint(R1 - R2, X2 - X1, Y2 - Y1);
  }

  template<class T>
  BSS_FORCEINLINE bool CircleCircleCollide(T X1, T Y1, T R1, T X2, T Y2, T R2)
  {
    return RadiusContainsPoint(R1 + R2, X2 - X1, Y2 - Y1);
  }

  template<class T>
  inline void RadiusNearestPoint(T r, T X, T Y, T& outX, T& outY) 
  { 
    T s = r / bss::FastSqrt((X*X) + (Y*Y));
    outX = X*s;
    outY = Y*s;
  }

  template<class T>
  BSS_FORCEINLINE void CircleNearestPoint(T x, T y, T r, T X, T Y, T& outX, T& outY) { RadiusNearestPoint(r, X - x, Y - y, outX, outY); outX += x; outY += y; }

  /**************** Line ****************/

  // This returns the parametric intersection points for the given line, if they exist, which can then be used for collision, intersection, etc.
  template<class T>
  inline int LineRadiusParametricIntersect(T X1, T Y1, T X2, T Y2, T r, T& out1, T& out2)
  {
    T Dx = X2 - X1;
    T Dy = Y2 - Y1;
    T Dr2 = Dx*Dx + Dy*Dy;
    T b = 2 * (X1*Dx + Y1*Dy);
    T c = (X1*X1 + Y1*Y1) - r*r;

    T d = b*b - 4 * Dr2*c;
    if(d < 0) // No intersection
      return 0;

    d = bss::FastSqrt<T>(d);
    out1 = (-b - d) / (2 * Dr2);
    out2 = (-b + d) / (2 * Dr2);

    return 2;
  }

  template<class T>
  inline int LineRadiusIntersect(T X1, T Y1, T X2, T Y2, T r, Vector<T, 2>(&out)[2])
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    T Dx = X2 - X1;
    T Dy = Y2 - Y1;

    for(int i = 0; i < n; ++i)
    {
      out[i].x = X1 + Dx*t[i];
      out[i].y = Y1 + Dy*t[i];
    }

    return n;
  }

  template<class T>
  inline int LineSegmentRadiusIntersect(T X1, T Y1, T X2, T Y2, T r, Vector<T, 2>(&out)[2])
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    T Dx = X2 - X1;
    T Dy = Y2 - Y1;

    int count = 0;
    for(int i = 0; i < n; ++i)
    {
      if(t[i] >= 0 && t[i] <= 1)
      {
        out[count].x = X1 + Dx*t[i];
        out[count].y = Y1 + Dy*t[i];
        ++count;
      }
    }

    return count;
  }

  template<class T>
  inline int RayRadiusIntersect(T X1, T Y1, T X2, T Y2, T r, Vector<T, 2>(&out)[2])
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    T Dx = X2 - X1;
    T Dy = Y2 - Y1;

    int count = 0;
    for(int i = 0; i < n; ++i)
    {
      if(t[i] >= 0)
      {
        out[count].x = X1 + Dx*t[i];
        out[count].y = Y1 + Dy*t[i];
        ++count;
      }
    }

    return count;
  }

  template<class T>
  inline bool LineRadiusCollide(T X1, T Y1, T X2, T Y2, T r)
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    return n > 0;
  }

  template<class T>
  inline int RayRadiusCollide(T X1, T Y1, T X2, T Y2, T r)
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    if(!n)
      return false;
    return t[0] >= 0 || t[1] >= 0;
  }

  template<class T>
  inline int LineSegmentRadiusCollide(T X1, T Y1, T X2, T Y2, T r)
  {
    T t[2];
    int n = LineRadiusParametricIntersect<T>(X1, Y1, X2, Y2, r, t[0], t[1]);
    if(!n)
      return false;
    return (t[0] >= 0 && t[0] <= 1) || (t[1] >= 0 && t[1] <= 1);
  }

  template<class T>
  inline T LinePointParametricProject(T X1, T Y1, T X2, T Y2, T pX, T pY)
  {
    T tx = X2 - X1;
    T ty = Y2 - Y1;
    return ((pX - X1)*tx + (pY - Y1)*ty) / ((tx*tx) + (ty*ty));
  }
  template<class T>
  inline void LineNearestPoint(T X1, T Y1, T X2, T Y2, T pX, T pY, T& outX, T& outY)
  {
    T tx = X2 - X1;
    T ty = Y2 - Y1;
    T u = LinePointParametricProject<T>(X1, Y1, X2, Y2, pX, pY);
    outX = X1 + u*tx;
    outY = Y1 + u*ty;
  }
  template<class T>
  inline void RayNearestPoint(T X1, T Y1, T X2, T Y2, T pX, T pY, T& outX, T& outY)
  {
    T tx = X2 - X1;
    T ty = Y2 - Y1;
    T u = LinePointParametricProject<T>(X1, Y1, X2, Y2, pX, pY);
    if(u <= 0)
    {
      outX = X1;
      outY = Y1;
    }
    else
    {
      outX = X1 + u*tx;
      outY = Y1 + u*ty;
    }
  }
  template<class T>
  inline void LineSegmentNearestPoint(T X1, T Y1, T X2, T Y2, T pX, T pY, T& outX, T& outY)
  {
    T tx = X2 - X1;
    T ty = Y2 - Y1;
    T u = LinePointParametricProject<T>(X1, Y1, X2, Y2, pX, pY);
    if(u <= 0)
    {
      outX = X1;
      outY = Y1;
    }
    else if(u >= 1)
    {
      outX = X2;
      outY = Y2;
    }
    else
    {
      outX = X1 + u*tx;
      outY = Y1 + u*ty;
    }
  }
  template<typename T>
  inline T LinePointDistanceSqr(T X1, T Y1, T X2, T Y2, T pX, T pY)
  {
    T tx = X2 - X1; 
    T ty = Y2 - Y1; 
    T det = (tx*(Y1 - pY)) - ((X1 - pX)*ty); 
    return (det*det) / ((tx*tx) + (ty*ty));
  }
  template<typename T>
  inline T LinePointDistance(T X1, T Y1, T X2, T Y2, T pX, T pY)
  {
    T tx = X2 - X1;
    T ty = Y2 - Y1;
    T det = (tx*(Y1 - pY)) - ((X1 - pX)*ty);
    return (det*det) / bss::FastSqrt<T>((tx*tx) + (ty*ty));
  }
  template<typename T>
  inline T LineSegmentPointDistanceSqr(T X1, T Y1, T X2, T Y2, T pX, T pY)
  {
    T vx = X1 - pX;
    T vy = Y1 - pY;
    T ux = X2 - X1;
    T uy = Y2 - Y1;
    T length = ux*ux + uy*uy;
    T det = (-vx*ux) + (-vy*uy); //if this is < 0 or > length then its outside the line segment
    if(det<0 || det>length) 
    { 
      ux = X2 - pX; 
      uy = Y2 - pY; 
      vx = vx*vx + vy*vy; 
      ux = ux*ux + uy*uy;
      return (vx<ux) ? vx : ux; 
    }
    det = ux*vy - uy*vx;
    return (det*det) / length;
  }
  template<typename T>
  BSS_FORCEINLINE T LineSegmentPointDistance(T X1, T Y1, T X2, T Y2, T pX, T pY)
  {
    return LineSegmentPointDistanceSqr(X1, Y1, X2, Y2, pX, pY);
  }
  template<typename T>
  inline bool LineSegmentLineSegmentIntersect(T x1, T y1, T x2, T y2, T X1, T Y1, T X2, T Y2, Vector<T, 2>& retpoint)
  {
    Vector<T, 2> p(x1, y1);
    Vector<T, 2> r(x2 - x1, y2 - y1);
    Vector<T, 2> q(X1, Y1);
    Vector<T, 2> s(X2 - X1, Y2 - Y1);
    T rCrossS = r.Cross(s);

    if(rCrossS <= 0 && rCrossS >= -1)
      return false;
    T t = s.Cross(q -= p) / rCrossS;
    T u = r.Cross(q) / rCrossS;
    if(0 <= u && u <= 1 && 0 <= t && t <= 1)
    {
      retpoint = (p += (r *= t));
      return true;
    }
    return false;
  }
  template<typename T>
  inline bool LineLineIntersect(T x1, T y1, T x2, T y2, T X1, T Y1, T X2, T Y2, Vector<T, 2>& retpoint)
  {
    Vector<T, 2> p(x1, y1);
    Vector<T, 2> r(x2 - x1, y2 - y1);
    Vector<T, 2> q(X1, Y1);
    Vector<T, 2> s(X2 - X1, Y2 - Y1);
    T rCrossS = r.Cross(s);

    if(rCrossS <= 0 && rCrossS >= -1)
      return false;
    T t = s.Cross(q -= p) / rCrossS;
    retpoint = (p += (r *= t));
    return true;
  }

  template<typename T>
  inline bool LineSegmentLineSegmentCollide(T x1, T y1, T x2, T y2, T X1, T Y1, T X2, T Y2)
  {
    T da = X2 - X1;
    T db = Y2 - Y1;
    T dx = x2 - x1;
    T dy = y2 - y1;
    T s = (dx*(Y1 - y1) + dy*(x1 - X1)) / (da*dy - db*dx);
    T t = (da*(y1 - Y1) + db*(X1 - x1)) / (db*dx - da*dy);
    return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
  }

  template<typename T>
  inline bool LineSegmentLineVertCollide(T x1, T y1, T x2, T y2, T X, T Y1, T Y2)
  {
    T db = Y2 - Y1;
    T dx = x2 - x1; T dy = y2 - y1;
    dy = (dx*(Y1 - y1) + dy*(x1 - X)) / (-db*dx);
    X = (db*(X - x1)) / (db*dx);
    return (dy >= 0 && dy <= 1 && X >= 0 && X <= 1);
  }

  template<typename T>
  inline bool LineSegmentLineHorzCollide(T x1, T y1, T x2, T y2, T X1, T X2, T Y)
  {
    T da = X2 - X1;
    T dx = x2 - x1; T dy = y2 - y1;
    dx = (dx*(Y - y1) + dy*(x1 - X1)) / (da*dy);
    Y = (da*(y1 - Y)) / (-da*dy);
    return (dx >= 0 && dx <= 1 && Y >= 0 && Y <= 1);
  }

  /**************** Rect ****************/

  template<typename T>
  inline bool RectCircleCollide(T left, T top, T right, T bottom, T X, T Y, T R)
  { //based on a solution from this thread: http://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
    T rwh = (right - left) / 2;
    T rhh = (bottom - top) / 2;
    T cx = abs(X - ((right + left) / 2));
    T cy = abs(Y - ((bottom + top) / 2));

    if(cx > (rwh + R) || cy > (rhh + R)) return false;
    if(cx <= rwh || cy <= rhh) return true;

    cx -= rwh;
    cy -= rhh;
    rwh = (cx*cx) + (cy*cy);
    return rwh <= (R*R);
  }

  template<typename T>
  inline bool RectLineCollide(T left, T top, T right, T bottom, T X1, T Y1, T X2, T Y2)
  {
    T m = X2 - X1;
    if(m == (T)0) //this is a special case
    {
      return !(X1 < left || X2 > right || (Y1 < top && Y2 < top) || (Y1 > bottom && Y2 < bottom));
    }
    m = (Y2 - Y1) / m; //this is a precaution to prevent divide by zero
                       //T m = (Y2-Y1) / (X2-X1);
    T c = Y1 - (m*X1);
    T ti, bi, tp, bp;

    if(m > 0) // if the line is going up from right to left then the top intersect point is on the left
    {
      ti = (m*left + c);
      bi = (m*right + c);
    }
    else // otherwise it's on the right
    {
      ti = (m*right + c);
      bi = (m*left + c);
    }

    if(Y1 < Y2) // work out the top and bottom extents for the triangle
    {
      tp = Y1;
      bp = Y2;
    }
    else
    {
      tp = Y2;
      bp = Y1;
    }

    ti = ti > tp ? ti : tp;
    bi = bi < bp ? bi : bp;

    return (ti < bi) && (!((bi < top) || (ti > bottom)));
  }

  template<typename T>
  BSS_FORCEINLINE bool RectContainsPoint(T left, T top, T right, T bottom, T X, T Y)
  {
    return (X >= left && Y >= top && X < right && Y < bottom);
  }

  template<typename T>
  BSS_FORCEINLINE bool RectContainsRect(T Left, T Top, T Right, T Bottom, T left, T top, T right, T bottom)
  {
    return (left >= Left && top >= Top && right <= Right && bottom <= Bottom);
  }

  template<typename T>
  BSS_FORCEINLINE bool RectRectCollide(T Left, T Top, T Right, T Bottom, T left, T top, T right, T bottom)
  {
    return left <= Right && top <= Bottom && right >= Left && bottom >= Top;
  }

  /**************** Triangle ****************/

  template<typename T> //Uses the standard barycentric method to figure out point inclusion
  inline bool TriangleContainsPoint(Vector<T, 2>(&p)[2], T X, T Y) 
  {
    Vector<T, 2> v0(p[2]);
    Vector<T, 2> v1(p[1]);
    Vector<T, 2> v2(X, Y);
    v0 -= p[0];
    v1 -= p[0];
    v2 -= p[0];

    // Compute dot products
    T dot00 = v0.DotProduct(v0);
    T dot01 = v0.DotProduct(v1);
    v0.x = v0.DotProduct(v2);
    v0.y = v1.DotProduct(v1);
    v1.x = v1.DotProduct(v2);

    // Compute barycentric coordinates
    v1.y = 1 / (dot00 * v0.y - dot01 * dot01);
    v2.x = (v0.y * v0.x - dot01 * v1.x) * v1.y;
    v2.y = (dot00 * v1.x - dot01 * v0.x) * v1.y;

    // Check if point is in triangle
    return (v2.x > 0) && (v2.y > 0) && (v2.x + v2.y < 1);
  }

  /**************** Ellipse ****************/

  template<typename T>
  BSS_FORCEINLINE bool EllipseContainsPoint(T X, T Y, T A, T B, T x, T y) 
  { 
    T tx = X - x; 
    T ty = Y - y; 
    return ((tx*tx) / (A*A)) + ((ty*ty) / (B*B)) <= 1; 
  }

  template<typename T>
  inline void EllipseNearestPoint(T A, T B, T cx, T cy, T& outX, T& outY)
  {
    //f(t) = (A*A - B*B)cos(t)sin(t) - x*A*sin(t) + y*B*cos(t)
    //f'(t) = (A*A - B*B)(cos²(t) - sin²(t)) - x*a*cos(t) - y*B*sin(t)
    //x(t) = [A*cos(t),B*sin(t)]
    T A2 = A*A;
    T B2 = B*B;
    if(((cx*cx) / A2) + ((cy*cy) / B2) <= 1)
    {
      outX = cx;
      outY = cy;
      return;
    }
    T t = atan2(A*cy, B*cx); //Initial guess = arctan(A*y/B*x)
    T AB2 = (A2 - B2); //precalculated values for optimization
    T xA = cx*A;
    T yB = cy*B;
    T f_, f__, tcos, tsin;
    for(int i = 0; i < 4; ++i)
    {
      tcos = cos(t);
      tsin = sin(t);
      f_ = AB2*tcos*tsin - xA*tsin + yB*tcos; //f(t)
      f__ = AB2*(tcos*tcos - tsin*tsin) - xA*tcos - yB*tsin; //f'(t)
      t = t - f_ / f__; //Newton's method: t - f(t)/f'(t)
    }

    outX = A*cos(t);
    outY = B*sin(t);
  }

  template<typename T>
  BSS_FORCEINLINE bool EllipseCircleCollide(T X, T Y, T A, T B, T x, T y, T r)
  {
    T dx = x - X; //Centers the ellipse/circle pair such that the ellipse is at the origin
    T dy = y - Y;
    EllipseNearestPoint(A, B, dx, dy, X, Y);
    return bss::DistSqr(X, Y, dx, dy)<(r*r);
  }

  template<typename T>
  BSS_FORCEINLINE bool EllipseRectCollide(T X, T Y, T A, T B, T left, T top, T right, T bottom)
  {
    T s = A / B;
    return RectCircleCollide(left, top*s, right, bottom*s, X, Y*s, A);
  }

  /**************** Circle Ellipse ****************/

  template<typename T>
  BSS_FORCEINLINE bool RadiusSectorContainsPointRadius(T X, T Y, T inner, T outer)
  {
    T dist = X*X + Y*Y;
    return (dist <= outer*outer) && (dist >= inner*inner);
  }

  template<typename T>
  BSS_FORCEINLINE bool RadiusSectorContainsPointAngle(T X, T Y, T min, T range)
  {
    if(range >= (T)bss::PI_DOUBLE)
      return true;

    range *= 0.5;
    T angle = atan2(-Y, X);
    return bss::AngleDist<T>(angle, min + range) <= range;
  }
  template<typename T>
  BSS_FORCEINLINE bool RadiusSectorContainsPoint(T X, T Y, T inner, T outer, T min, T range)
  {
    return RadiusSectorContainsPointRadius(X, Y, inner, outer) && RadiusSectorContainsPointAngle(X, Y, min, range);
  }
  template<typename T>
  BSS_FORCEINLINE bool CircleSectorContainsPoint(T x, T y, T inner, T outer, T min, T range, T X, T Y)
  {
    return RadiusSectorContainsPoint(X - x, Y - y, inner, outer, min, range);
  }


  /**************** Polygon ****************/

  template<class T>
  inline bool PolygonContainsPoint(const Vector<T, 2>& point, const Vector<T, 2>* verts, size_t num)
  {
    assert(num > 2);
    size_t i, j, c = 0;
    for(i = 0, j = num - 1; i < num; j = i++)
    {
      if(((verts[i].y>point.y) != (verts[j].y>point.y)) &&
        (point.x < (verts[j].x - verts[i].x) * (point.y - verts[i].y) / (verts[j].y - verts[i].y) + verts[i].x))
        c = !c;
    }
    return c != 0;
  }

  // Checks if the given series of points consitutes a convex polygon
  template<class T>
  inline bool PolygonIsConvex(const Vector<T, 2>* p, size_t n)
  {
    assert(n > 2);
    uint8_t flag = 0;
    T z;

    for(size_t i = 0, k = 0, j = n - 1; i<n; j = i++)
    {
      k = ((++k) % n);
      z = ((p[i].x - p[j].x) * (p[k].y - p[i].y)) - ((p[i].y - p[j].y) * (p[k].x - p[i].x));
      flag |= ((z<0) | ((z>0) << 1));
    }
    return flag != 3;
  }

  template<class T>
  inline bool ConvexPolygonContainsPoint(const Vector<T, 2>& point, const Vector<T, 2>* verts, size_t num)
  {
    assert(PolygonIsConvex<T>(verts, num));

    //(y - y0)*(x1 - x0) - (x - x0)*(y1 - y0) where x,y is the point being tested and 0 and 1 are vertices on the polygon, returns negative if on the right, 0 if its on the line, and positive if its on the left
    T last = ((point.y - verts[0].y)*(verts[num - 1].x - verts[0].x)) - ((point.x - verts[0].x)*(verts[num - 1].y - verts[0].y));
    T cur;
    for(size_t i = 1; i< num; ++i)
    {
      cur = ((point.y - verts[i - 1].y)*(verts[i].x - verts[i - 1].x)) - ((point.x - verts[i - 1].x)*(verts[i].y - verts[i - 1].y));
      if(cur<0 && last>0 || cur>0 && last<0) return false;//This is written so that being on the line counts as an intersection.
    }

    return true;
  }

  template<class T>
  inline T PolygonArea(const Vector<T, 2>* verts, size_t num)
  {
    assert(num>2); //you must be at least a triangle or this is invalid
    T area = 0; //Since the polygon is supposed to be non-intersecting, we can use a line integral to calculate the area

    for(size_t i = 0, j = num - 1; i<num; j = i++) //for loop trick for iterating each edge taken from pnpoly
      area += ((verts[j].x*verts[i].y) - (verts[i].x*verts[j].y));

    return area / ((T)2);
  }

  template<class T>
  inline Vector<T, 2> PolygonCentroid(const Vector<T, 2>* verts, size_t num)
  {
    Vector<T, 2> c;

    for(size_t i = 0, j = num - 1; i<num; j = i++) //for loop trick for iterating each edge taken from pnpoly
    {
      c.x += (verts[j].x + verts[i].x)*((verts[j].x*verts[i].y) - (verts[i].x*verts[j].y));
      c.y += (verts[j].y + verts[i].y)*((verts[j].x*verts[i].y) - (verts[i].x*verts[j].y));
    }

    return c / (PolygonArea<T>(verts, num) * 6);
  }
}

#endif