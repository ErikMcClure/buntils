// Copyright (c)2023 Erik McClure
// For conditions of distribution and use, see copyright notice in "buntils.h"

#ifndef __BUN_GEOMETRY_H__
#define __BUN_GEOMETRY_H__

#include "Collision.h"
#include "Array.h"

namespace bun {
  // Represents an N-dimensional sphere
  template<class T, int N>
  struct NSphere
  {
    template<typename U>
    inline constexpr NSphere(const NSphere<U, N>& copy) : c(copy.c), r(copy.r) { }
    inline constexpr explicit NSphere(const Vector<T, N>& center, T radius) : c(center), r(radius) { }
    inline constexpr explicit NSphere(const T(&e)[N], T radius) : c(e), r(radius) { }
    inline constexpr explicit NSphere(const std::array<T, N>& e, T radius) : c(e), r(radius) { }
    inline constexpr NSphere() {}
    inline T Diameter() const { return r * 2; }
    inline T Radius() const { return r; }
    inline bool ContainsPoint(const Vector<T, N>& p) const { return SphereContainsPoint<T, N>(p, c, r); }
    inline bool ContainsSphere(const NSphere<T, N>& s) const { return SphereContainsSphere<T, N>(c, r, s.c, s.r); }
    inline bool ContainsSphere(const Vector<T, N>& C, T R) const { return SphereContainsSphere<T, N>(c, r, C, R); }
    inline bool SphereCollide(const Vector<T, N>& C, T R) const { return SphereSphereCollide<T, N>(c, r, C, R); }
    inline bool SphereCollide(const NSphere<T, N>& s) const { return SphereSphereCollide<T, N>(c, r, s.c, s.r); }

    template<class U>
    inline NSphere& operator=(const NSphere<U, N>& other) { c = other.c; r = other.r; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        Vector<T, N> c;
        T r;
      };
      T v[N + 1];
    };
  };

  // A circle as a 2 dimensional sphere
  template<class T>
  struct NSphere<T, 2>
  {
    template<typename U>
    inline constexpr NSphere(const NSphere<U, 2>& copy) : c(copy.c), r(copy.r) { }
    inline constexpr explicit NSphere(const Vector<T, 2>& center, T radius) : c(center), r(radius) { }
    inline constexpr explicit NSphere(T X, T Y, T radius) : c(X, Y), r(radius) { }
    inline constexpr explicit NSphere(const T(&e)[2], T radius) : c(e), r(radius) { }
    inline constexpr explicit NSphere(const std::array<T, 2>& e, T radius) : c(e), r(radius) { }
    inline constexpr NSphere() {}
    inline T Area() const { return r * r * (T)PI; }
    inline T Circumference() const { return r * 2 * (T)PI; }
    inline T Diameter() const { return r * 2; }
    inline T Radius() const { return r; }
    BUN_FORCEINLINE bool ContainsPoint(const Vector<T, 2>& p) const { return RadiusContainsPoint(r, p.x - x, p.y - y); }
    BUN_FORCEINLINE bool ContainsPoint(T X, T Y) const { return RadiusContainsPoint(r, X - x, Y - y); }
    BUN_FORCEINLINE bool ContainsCircle(const NSphere<T, 2>& s) const { return ContainsCircle(s.x, s.y, s.r); }
    BUN_FORCEINLINE bool ContainsCircle(const Vector<T, 2>& C, T R) const { return ContainsCircle(C.x, C.y, R); }
    BUN_FORCEINLINE bool ContainsCircle(T X, T Y, T R) const { if(r <= R) return false; return RadiusContainsPoint(r - R, X - x, Y - y); }
    BUN_FORCEINLINE void NearestPoint(const Vector<T, 2>& p, T& outX, T& outY) const { RadiusNearestPoint(r, p.x - x, p.y - y, outX, outY); }
    BUN_FORCEINLINE void NearestPoint(T X, T Y, T& outX, T& outY) const { RadiusNearestPoint(r, X - x, Y - y, outX, outY); }
    BUN_FORCEINLINE int CircleIntersect(T X, T Y, T R, Vector<T, 2>(&out)[2]) const { return CircleRadiusIntersect<T>(r, X - x, Y - y, R, out); }
    BUN_FORCEINLINE bool CircleCollide(T X, T Y, T R) const { return RadiusContainsPoint(r + R, X - x, Y - y); }
    BUN_FORCEINLINE bool CircleCollide(const Vector<T, 2>& C, T R) const { return RadiusContainsPoint(r + R, C.x - x, C.x - y); }
    BUN_FORCEINLINE bool LineCollide(T X1, T Y1, T X2, T Y2) const { return LineRadiusCollide(X1 - x, Y1 - y, X2 - x, Y2 - y, r); }
    BUN_FORCEINLINE bool RayCollide(T X1, T Y1, T X2, T Y2) const { return RayRadiusCollide(X1 - x, Y1 - y, X2 - x, Y2 - y, r); }
    BUN_FORCEINLINE bool LineSegmentCollide(T X1, T Y1, T X2, T Y2) const { return LineSegmentRadiusCollide(X1 - x, Y1 - y, X2 - x, Y2 - y, r); }
    BUN_FORCEINLINE bool LineIntersect(T X1, T Y1, T X2, T Y2, Vector<T, 2>(&out)[2]) const { return LineRadiusIntersect(X1 - x, Y1 - y, X2 - x, Y2 - y, r, out); }
    BUN_FORCEINLINE bool RayIntersect(T X1, T Y1, T X2, T Y2, Vector<T, 2>(&out)[2]) const { return RayRadiusIntersect(X1 - x, Y1 - y, X2 - x, Y2 - y, r, out); }
    BUN_FORCEINLINE bool LineSegmentIntersect(T X1, T Y1, T X2, T Y2, Vector<T, 2>(&out)[2]) const { return LineSegmentRadiusIntersect(X1 - x, Y1 - y, X2 - x, Y2 - y, r, out); }
    BUN_FORCEINLINE bool RectCollide(T left, T top, T right, T bottom) const { return RectCircleCollide<T>(left, top, right, bottom, x, y, r); }
    BUN_FORCEINLINE bool EllipseCollide(T X, T Y, T A, T B) const { return EllipseCircleCollide(X, Y, A, B, x, y, r); }

    template<class U>
    inline NSphere& operator=(const NSphere<U, 2>& other) { c = other.c; r = other.r; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        union {
          Vector<T, 2> c;
          struct {
            T x;
            T y;
          };
        };
        T r;
      };
      T v[3];
    };
  };

  // Traditional 3 dimensional sphere
  template<class T>
  struct NSphere<T, 3>
  {
    template<typename U>
    inline constexpr NSphere(const NSphere<U, 3>& copy) : c(copy.c), r(copy.r) { }
    inline constexpr explicit NSphere(const Vector<T, 3>& center, T radius) : c(center), r(radius) { }
    inline constexpr explicit NSphere(T X, T Y, T Z, T radius) : c(X, Y, Z), r(radius) { }
    inline constexpr explicit NSphere(const T(&e)[3], T radius) : c(e), r(radius) { }
    inline constexpr explicit NSphere(const std::array<T, 3>& e, T radius) : c(e), r(radius) { }
    inline constexpr NSphere() {}
    inline T Volume() const { return (4 * r * r * r * (T)PI) / 3; }
    inline T SurfaceArea() const { return r * r * 4 * (T)PI; }
    inline T Diameter() const { return r * 2; }
    inline T Radius() const { return r; }
    inline bool ContainsPoint(const Vector<T, 3>& p) const { return SphereContainsPoint<T, 3>(p, c, r); }
    inline bool ContainsSphere(const NSphere<T, 3>& s) const { return SphereContainsSphere<T, 3>(c, r, s.c, s.r); }
    inline bool ContainsSphere(const Vector<T, 3>& C, T R) const { return SphereContainsSphere<T, 3>(c, r, C, R); }
    inline bool SphereCollide(const Vector<T, 3>& C, T R) const { return SphereSphereCollide<T, 3>(c, r, C, R); }
    inline bool SphereCollide(const NSphere<T, 3>& s) const { return SphereSphereCollide<T, 3>(c, r, s.c, s.r); }

    template<class U>
    inline NSphere& operator=(const NSphere<U, 3>& other) { c = other.c; r = other.r; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        union {
          Vector<T, 3> c;
          struct {
            T x;
            T y;
            T z;
          };
        };
        T r;
      };
      T v[4];
    };
  };

  template<class T> using Circle = NSphere<T, 2>;
  template<class T> using Sphere = NSphere<T, 3>;

  // N-dimensional line (or ray, or line segment, depending on how it's used)
  template<class T, int N>
  struct LineN
  {
    using V = Vector<T, N>;
    inline constexpr LineN() {}
    template<class U>
    inline constexpr LineN(const LineN<U, N>& other) : p1(other.p1), p2(other.p2) {}
    inline constexpr LineN(const V& P1, const V& P2) : p1(P1), p2(P2) {}
    BUN_FORCEINLINE T Length() const { return p1.Distance(p2); }
    inline V ParametricPoint(T s) const { return p1 + (p2 - p1)*s; }

    template<class U>
    inline LineN& operator=(const LineN<U, N>& other) { p1 = other.p1; p2 = other.p2; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        V p1;
        V p2;
      };
      T v[N * 2];
    };
  };

  // 2D line
  template<class T>
  struct LineN<T, 2>
  {
    using V = Vector<T, 2>;
    inline constexpr LineN() {}
    template<class U>
    inline constexpr LineN(const LineN<U, 2>& other) : p1(other.p1), p2(other.p2) {}
    inline constexpr LineN(const V& P1, const V& P2) : p1(P1), p2(P2) {}
    inline constexpr LineN(T X1, T Y1, T X2, T Y2) : x1(X1), y1(Y1), x2(X2), y2(Y2) {}
    inline constexpr LineN(const V& P1, T X2, T Y2) : p1(P1), x2(X2), y2(Y2) {}
    BUN_FORCEINLINE T Length() const { return Dist<T>(x1, y1, x2, y2); }
    BUN_FORCEINLINE V ParametricPoint(T s) const { return p1 + (p2 - p1)*s; }
    BUN_FORCEINLINE LineN Rotate(T r, const V& center) const { return Rotate(r, center.x, center.y); }
    BUN_FORCEINLINE LineN Rotate(T r, T x, T y) const { return LineN(p1.Rotate(r, x, y), p2.Rotate(r, x, y)); }

    template<class U>
    inline LineN& operator=(const LineN<U, 2>& other) { p1 = other.p1; p2 = other.p2; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        V p1;
        V p2;
      };
      struct {
        T x1;
        T y1;
        T x2;
        T y2;
      };
      T v[4];
    };
  };

  // 3D line
  template<class T>
  struct LineN<T, 3>
  {
    using V = Vector<T, 3>;
    inline constexpr LineN() {}
    template<class U>
    inline constexpr LineN(const LineN<U, 3>& other) : p1(other.p1), p2(other.p2) {}
    inline constexpr LineN(const V& P1, const V& P2) : p1(P1), p2(P2) {}
    inline constexpr LineN(T X1, T Y1, T Z1, T X2, T Y2, T Z2) : x1(X1), y1(Y1), z1(Z1), x2(X2), y2(Y2), z2(Z2) {}
    inline constexpr LineN(const V& P1, T X2, T Y2, T Z2) : p1(P1), x2(X2), y2(Y2), z2(Z2) {}
    inline T Length() const { return p1.Distance(p2); }
    inline V ParametricPoint(T s) const { return p1 + (p2 - p1)*s; }

    template<class U>
    inline LineN& operator=(const LineN<U, 3>& other) { p1 = other.p1; p2 = other.p2; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        V p1;
        V p2;
      };
      struct {
        T x1;
        T y1;
        T z1;
        T x2;
        T y2;
        T z2;
      };
      T v[6];
    };
  };
  template<class T> using Line = LineN<T, 2>;
  template<class T> using Line3d = LineN<T, 3>;

  // 2D Rectangle
  template<class T>
  struct Rect
  {
    using V = Vector<T, 2>;

    inline constexpr Rect() {} //The following constructors allow for implicit conversion between rect types
    inline constexpr Rect(const Rect& other) : left(other.left), top(other.top), right(other.right), bottom(other.bottom) {}
    template<class U>
    inline constexpr Rect(const Rect<U>& other) : left((T)other.left), top((T)other.top), right((T)other.right), bottom((T)other.bottom) {}
    inline constexpr Rect(T Left, T Top, T Right, T Bottom) : left(Left), top(Top), right(Right), bottom(Bottom) {}
    inline constexpr Rect(T X, T Y, const V& dim) : left(X), top(Y), right(X + dim.x), bottom(Y + dim.y) {}
    inline constexpr Rect(const V& pos, const V& dim) : left(pos.x), top(pos.y), right(pos.x + dim.x), bottom(pos.y + dim.y) {}
    inline explicit Rect(const V& v) : left(v.x), top(v.y), right(v.x), bottom(v.y) {}
    inline explicit Rect(const sseVecT<T>& v) { v.Set(ltrb); }
    inline explicit Rect(const T(&rectarray)[4]) : left(rectarray[0]), top(rectarray[1]), right(rectarray[2]), bottom(rectarray[3]) {}
    inline explicit Rect(const std::array<T, 4>& rectarray) : left(rectarray[0]), top(rectarray[1]), right(rectarray[2]), bottom(rectarray[3]) {}
    inline T Area() const { return (right - left)*(bottom - top); }
    inline Rect Abs() const { return Rect(abs(left), abs(top), abs(right), abs(bottom)); }
    inline Circle<T> BoundingCircle() const { V center = topleft + bottomright; center /= (T)2; return Circle<T>(center, Dist<T>(center.x, center.y, topleft.x, topleft.y)); }
    inline V Center() const { return (topleft + bottomright) / (T)2; }
    BUN_FORCEINLINE V Dim() const { return V(right - left, bottom - top); }
    BUN_FORCEINLINE Rect Inflate(T amount) const { return Rect(left - amount, top - amount, right + amount, bottom + amount); }
    BUN_FORCEINLINE Rect Project(T Z) const { return Rect(*this) / (Z + 1); }
    BUN_FORCEINLINE sseVecT<T> ToSSE() const { return sseVecT<T>(ltrb); }
    // Returns true if left <= right and top <= bottom
    BUN_FORCEINLINE bool IsCanonical() const { return left <= right && top <= bottom; }
    // Returns a rectangle with a proper LTRB description, such that left < right and top < bottom.
    BUN_FORCEINLINE Rect Canonical() const { return Rect(bun_min(left, right), bun_min(top, bottom), bun_max(left, right), bun_max(top, bottom)); }
    // Returns a new rectangle that includes the given point.
    BUN_FORCEINLINE Rect ExpandTo(const V& point) const { return Rect(bun_min(left, point.x), bun_min(top, point.y), bun_max(right, point.x), bun_max(bottom, point.y)); }
    // Returns a rectangle that contains both this one and rect
    BUN_FORCEINLINE Rect Union(const Rect& rect) const { return Rect(bun_min(left, rect.left), bun_min(top, rect.top), bun_max(right, rect.right), bun_max(bottom, rect.bottom)); }
    // Returns a rectangle that contains only the overlapping parts (will be degenerate if there is no overlap)
    BUN_FORCEINLINE Rect Intersection(const Rect& rect) const { if(!RectCollide(rect)) return Rect(left, top, left, top); return Rect(bun_max(left, rect.left), bun_max(top, rect.top), bun_min(right, rect.right), bun_min(bottom, rect.bottom)); }

    BUN_FORCEINLINE bool ContainsPoint(T X, T Y) const { return RectContainsPoint(left, top, right, bottom, X, Y); }
    BUN_FORCEINLINE bool ContainsRect(T Left, T Top, T Right, T Bottom) const { return RectContainsRect(left, top, right, bottom, Left, Top, Right, Bottom); }
    BUN_FORCEINLINE bool ContainsRect(const Rect& r) const { return RectContainsRect(left, top, right, bottom, r.left, r.top, r.right, r.bottom); }
    BUN_FORCEINLINE bool CircleCollide(T X, T Y, T R) const { return RectCircleCollide<T>(left, top, right, bottom, X, Y, R); }
    BUN_FORCEINLINE bool CircleCollide(const V& c, T R) const { return RectCircleCollide<T>(left, top, right, bottom, c.x, c.y, R); }
    BUN_FORCEINLINE bool CircleCollide(const Circle<T>& c) const { return RectCircleCollide<T>(left, top, right, bottom, c.x, c.y, c.r); }
    BUN_FORCEINLINE bool LineCollide(T X1, T Y1, T X2, T Y2) const { return RectLineCollide<T>(left, top, right, bottom, X1, Y1, X2, Y2); }
    BUN_FORCEINLINE bool LineCollide(const V& p1, const V& p2) const { return RectLineCollide<T>(left, top, right, bottom, p1.x, p1.y, p2.x, p2.y); }
    BUN_FORCEINLINE bool LineCollide(const Line<T>& l) const { return RectLineCollide<T>(left, top, right, bottom, l.x1, l.y1, l.x2, l.y2); }
    BUN_FORCEINLINE bool RectCollide(T Left, T Top, T Right, T Bottom) const { return RectRectCollide<T>(left, top, right, bottom, Left, Top, Right, Bottom); }
    BUN_FORCEINLINE bool RectCollide(const Rect& r) const { return RectRectCollide<T>(left, top, right, bottom, r.left, r.top, r.right, r.bottom); }

    inline Rect operator +(const Rect& other) const { return Rect(left + other.left, top + other.top, right + other.right, bottom + other.bottom); }
    inline Rect operator -(const Rect& other) const { return Rect(left - other.left, top - other.top, right - other.right, bottom - other.bottom); }
    inline Rect operator *(const Rect& other) const { return Rect(left * other.left, top * other.top, right * other.right, bottom * other.bottom); }
    inline Rect operator /(const Rect& other) const { return Rect(left / other.left, top / other.top, right / other.right, bottom / other.bottom); }
    inline Rect operator +(const V& other) const { return Rect(left + other.x, top + other.y, right + other.x, bottom + other.y); }
    inline Rect operator -(const V& other) const { return Rect(left - other.x, top - other.y, right - other.x, bottom - other.y); }
    inline Rect operator *(const V& other) const { return Rect(left * other.x, top * other.y, right * other.x, bottom * other.y); }
    inline Rect operator /(const V& other) const { return Rect(left / other.x, top / other.y, right / other.x, bottom / other.y); }
    inline Rect operator +(const T other) const { return Rect(left + other, top + other, right + other, bottom + other); }
    inline Rect operator -(const T other) const { return Rect(left - other, top - other, right - other, bottom - other); }
    inline Rect operator *(const T other) const { return Rect(left * other, top * other, right * other, bottom * other); }
    inline Rect operator /(const T other) const { return Rect(left / other, top / other, right / other, bottom / other); }

    inline Rect& operator +=(const Rect& other) { left += other.left; top += other.top; right += other.right; bottom += other.bottom; return *this; }
    inline Rect& operator -=(const Rect& other) { left -= other.left; top -= other.top; right -= other.right; bottom -= other.bottom; return *this; }
    inline Rect& operator *=(const Rect& other) { left *= other.left; top *= other.top; right *= other.right; bottom *= other.bottom; return *this; }
    inline Rect& operator /=(const Rect& other) { left /= other.left; top /= other.top; right /= other.right; bottom /= other.bottom; return *this; }
    inline Rect& operator +=(const V& other) { left += other.x; top += other.y; right += other.x; bottom += other.y; return *this; }
    inline Rect& operator -=(const V& other) { left -= other.x; top -= other.y; right -= other.x; bottom -= other.y; return *this; }
    inline Rect& operator *=(const V& other) { left *= other.x; top *= other.y; right *= other.x; bottom *= other.y; return *this; }
    inline Rect& operator /=(const V& other) { left /= other.x; top /= other.y; right /= other.x; bottom /= other.y; return *this; }
    inline Rect& operator +=(const T other) { left += other; top += other; right += other; bottom += other; return *this; }
    inline Rect& operator -=(const T other) { left -= other; top -= other; right -= other; bottom -= other; return *this; }
    inline Rect& operator *=(const T other) { left *= other; top *= other; right *= other; bottom *= other; return *this; }
    inline Rect& operator /=(const T other) { left /= other; top /= other; right /= other; bottom /= other; return *this; }
    //inline Rect& operator +=(const Rect& other) { (sseVecT<T>(ltrb)+sseVecT<T>(other.ltrb)).Set(ltrb); return *this; }
    //inline Rect& operator -=(const Rect& other) { (sseVecT<T>(ltrb)-sseVecT<T>(other.ltrb)).Set(ltrb); return *this; }
    //inline Rect& operator *=(const Rect& other) { (sseVecT<T>(ltrb)*sseVecT<T>(other.ltrb)).Set(ltrb); return *this; }
    //inline Rect& operator /=(const Rect& other) { (sseVecT<T>(ltrb)/sseVecT<T>(other.ltrb)).Set(ltrb); return *this; }
    //inline Rect& operator +=(const T other) { (sseVecT<T>(ltrb)+sseVecT<T>(other)).Set(ltrb); return *this; }
    //inline Rect& operator -=(const T other) { (sseVecT<T>(ltrb)-sseVecT<T>(other)).Set(ltrb); return *this; }
    //inline Rect& operator *=(const T other) { (sseVecT<T>(ltrb)*sseVecT<T>(other)).Set(ltrb); return *this; }
    //inline Rect& operator /=(const T other) { (sseVecT<T>(ltrb)/sseVecT<T>(other)).Set(ltrb); return *this; }

    inline Rect operator -() const { return Rect(-left, -top, -right, -bottom); }

    inline bool operator !=(const Rect &other) const { return (left != other.left) || (top != other.top) || (right != other.right) || (bottom != other.bottom); }
    inline bool operator ==(const Rect &other) const { return (left == other.left) && (top == other.top) && (right == other.right) && (bottom == other.bottom); }
    inline bool operator !=(const T other) const { return (left != other) || (top != other) || (right != other) || (bottom != other); }
    inline bool operator ==(const T other) const { return (left == other) && (top == other) && (right == other) && (bottom == other); }
    inline bool operator >(const Rect &other) const { return (left > other.left) || ((left == other.left) && ((top > other.top) || ((top == other.top) && ((right > other.right) || ((right == other.right) && (bottom > other.bottom)))))); }
    inline bool operator <(const Rect &other) const { return (left < other.left) || ((left == other.left) && ((top < other.top) || ((top == other.top) && ((right < other.right) || ((right == other.right) && (bottom < other.bottom)))))); }
    BUN_FORCEINLINE bool operator >=(const Rect &other) const { return !operator<(other); }
    BUN_FORCEINLINE bool operator <=(const Rect &other) const { return !operator>(other); }

    inline Rect& operator =(const sseVecT<T>& _right) { _right.Set(ltrb); return *this; }
    inline Rect& operator =(const Rect& _right) { left = _right.left; top = _right.top; right = _right.right;  bottom = _right.bottom; return *this; }
    template<class U>
    inline Rect& operator =(const Rect<U>& _right) { left = (T)_right.left; top = (T)_right.top; right = (T)_right.right; bottom = (T)_right.bottom; return *this; }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(ltrb, id); }

    union {
      struct {
        V topleft;
        V bottomright;
      };
      struct {
        T left;
        T top;
        T right;
        T bottom;
      };
      T ltrb[4];
    };
  };

  template<class T>
  inline void RectExpandTo(Rect<T>& r, const Vector<T, 2>& p)
  {
    if(p.x < r.left) r.left = p.x;
    else if(p.x > r.right) r.right = p.x;
    if(p.y < r.top) r.top = p.y;
    else if(p.y > r.bottom) r.bottom = p.y;
  }

  // 2D Triangle
  template<class T>
  struct Triangle
  {
    using V = Vector<T, 2>;
    inline constexpr Triangle() { }
    template<class U>
    inline constexpr Triangle(const Triangle<U>& other) { for(int i = 0; i < 3; ++i) v[i] = other.v[i]; }
    inline T Area() const { return (v[1] - v[0]).Cross(v[2] - v[0]) / ((T)2); }
    BUN_FORCEINLINE bool ContainsPoint(T X, T Y) const { return TriangleContainsPoint(v, X, Y); }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    V v[3];
  };

  // 2D Ellipse
  template<class T>
  struct Ellipse
  {
    using V = Vector<T, 2>;
    inline constexpr Ellipse() {}
    template<class U>
    inline constexpr Ellipse(const Ellipse<U>& other) : pos(other.pos), axes(other.axes) {}
    template<class U>
    inline explicit constexpr Ellipse(const Circle<U>& other) : pos(other.pos), axes(other.r) {}
    inline constexpr Ellipse(T X, T Y, T A, T B) : pos(X, Y), axes(A, B) {}
    inline constexpr Ellipse(const V& Pos, T A, T B) : pos(Pos), axes(A, B) {}
    inline constexpr Ellipse(const V& Pos, const V& Axes) : pos(Pos), axes(Axes) {}
    inline T Area() const { return (T)(PI*a*b); }
    BUN_FORCEINLINE bool ContainsPoint(T X, T Y) const { return EllipseContainsPoint(x, y, a, b, X, Y); }
    BUN_FORCEINLINE void NearestPoint(T X, T Y, T& outX, T& outY) const { return EllipseNearestPoint(a, b, X - x, Y - y, outX, outY); }
    BUN_FORCEINLINE bool RectCollide(T left, T top, T right, T bottom) const { return EllipseRectCollide(x, y, a, b, left, top, right, bottom); }
    BUN_FORCEINLINE bool RectCollide(const Rect<T>& r) const { return EllipseRectCollide(x, y, a, b, r.left, r.top, r.right, r.bottom); }
    BUN_FORCEINLINE bool CircleCollide(T X, T Y, T R) const { return EllipseCircleCollide(x, y, a, b, X, Y, R); }
    BUN_FORCEINLINE bool CircleCollide(const Circle<T>& c) const { return EllipseCircleCollide(x, y, a, b, c.x, c.y, c.r); }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        V pos;
        V axes;
      };
      struct {
        T x;
        T y;
        T a;
        T b;
      };
      T v[4];
    };
  };

  // 2D Circle Sector
  template<class T>
  struct CircleSector
  {
    using V = Vector<T, 2>;
    template<class U>
    inline constexpr CircleSector(const CircleSector<U>& other) : x((T)other.x), y((T)other.y), inner((T)other.inner), outer((T)other.outer), min((T)other.min), range((T)other.range) {}
    inline constexpr CircleSector(T X, T Y, T Inner, T Outer, T Min, T Range) : x(X), y(Y), inner(Inner), outer(Outer), min(bun_FMod<T>(Min, (T)PI_DOUBLE)), range(Range) {}
    inline constexpr CircleSector(const V& Pos, T Inner, T Outer, T Min, T Range) : x(Pos.x), y(Pos.y), inner(Inner), outer(Outer), min(bun_FMod<T>(Min, (T)PI_DOUBLE)), range(Range) {}
    inline constexpr CircleSector() {}
    inline T Area() const { return (T)((outer*outer - inner*inner)*range*0.5); }
    inline T ArcLengthOuter() const { return range*outer; }
    inline T ArcLengthInner() const { return range*inner; }
    inline bool ContainsPoint(T X, T Y) { return RadiusSectorContainsPoint(X - x, Y - y, inner, outer, min, range); }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { s.EvaluateFixedArray(v, id); }

    union {
      struct {
        union {
          struct {
            V pos;
          };
          struct {
            T x;
            T y;
          };
        };
        T outer;
        T inner;
        T min; // Must be in the range [0,2PI]
        T range; // Must be positive
      };
      T v[6];
    };

    inline bool IntersectPolygon(const V* vertices, size_t num)
    {
      if(num < 1)
        return false;
      if(ContainsPoint(vertices[0].x, vertices[0].y))
        return true;
      if(num < 2)
        return false;
      if(num < 3)
        return _intersectLine(vertices[0].x - x, vertices[0].y - y, vertices[1].x - x, vertices[1].y - y);

      for(size_t i = 1; i < num; ++i)
      {
        if(_intersectLine(vertices[i - 1].x - x, vertices[i - 1].y - y, vertices[i].x - x, vertices[i].y - y))
          return true;
      }
      if(_intersectLine(vertices[num - 1].x - x, vertices[num - 1].y - y, vertices[0].x - x, vertices[0].y - y))
        return true;
      return false;
    }

    inline bool IntersectLine(T X1, T Y1, T X2, T Y2)
    {
      X1 -= x;
      Y1 -= y;
      X2 -= x;
      Y2 -= y;
      if(RadiusSectorContainsPoint(X1, Y1, inner, outer, min, range))
        return true;
      return _intersectLine(X1, Y1, X2, Y2);
    }

    inline bool CircleCollide(T X, T Y, T R)
    {
      X -= x;
      Y -= y;
      T D2 = X*X + Y*Y;
      T tr = R + outer;
      if(D2 > tr*tr) // If the two circles don't intersect, bail out immediately
        return false;

      if(inner > 0) // Check if contained inside inner circle
      {
        T ti = inner - R;
        if(ti > 0 && D2 < (ti*ti))
          return false; // completely inside inner exclusion zone
      }

      if(range >= (T)PI_DOUBLE) // At this point, if this is just a circle, it must intersect
        return true;

      if(RadiusSectorContainsPointAngle(X, Y, min, range))
        return true; // If the center of the circle is inside the segment, automatically return

                     // Otherwise, generate line segments for each angle boundary and see if they intersect the circle
      if(_intersectAngleCircle(X, Y, R, min))
        return true;
      if(_intersectAngleCircle(X, Y, R, min + range))
        return true;

      return false; // If we have no intersections, there is no way we could be inside the circle.
    }

    inline bool CircleSectorCollide(T X, T Y, T Inner, T Outer, T Min, T Range)
    {
      X -= x;
      Y -= y;
      T D2 = X*X + Y*Y;
      T tr = Outer + outer;
      if(D2 > tr*tr) // If the two circles don't intersect, bail out immediately
        return false;
      if(RadiusSectorContainsPoint(X + cos(Min)*Inner, Y - sin(Min)*Inner, inner, outer, min, range))
        return true;
      if(RadiusSectorContainsPoint(-X + cos(min)*inner, -Y - sin(min)*inner, Inner, Outer, Min, Range))
        return true;

      // It is possible for coincidental circles to reach this point if they have a nonzero inner radius.
      if(fSmall(X) && fSmall(Y)) // If this true, both the angle range and radius range must intersect for the segments to intersect
        return inner <= Outer && Inner <= outer && min <= (Min + Range) && Min <= (min + range);

      Vector<T, 2> out[2];
      if(_checkBothAngles(X, Y, CircleRadiusIntersect<T>(outer, X, Y, Outer, out), out, min, range, Min, Range))
        return true;  // outer <-> Outer intersection

      if(inner > 0 && _checkBothAngles(X, Y, CircleRadiusIntersect<T>(inner, X, Y, Outer, out), out, min, range, Min, Range))
        return true; // inner <-> Outer intersection

      if(Inner > 0 && _checkBothAngles(X, Y, CircleRadiusIntersect<T>(Inner, X, Y, outer, out), out, min, range, Min, Range))
        return true; // Inner <-> outer intersection

      if(inner > 0 && Inner > 0 && _checkBothAngles(X, Y, CircleRadiusIntersect<T>(inner, X, Y, Inner, out), out, min, range, Min, Range))
        return true; // inner <-> Inner intersection

      if(Range < (T)PI_DOUBLE || range < (T)PI_DOUBLE)
      {
        if(_rayRayIntersect(X, Y, Inner, Outer, Min, inner, outer, min))
          return true; // Check minarc line segment intersection with other minarc line
        if(_rayRayIntersect(X, Y, Inner, Outer, Min + Range, inner, outer, min))
          return true; // Check minarc line segment intersection with other maxarc line
        if(_rayRayIntersect(X, Y, Inner, Outer, Min, inner, outer, min + range))
          return true; // Check maxarc line segment intersection with other minarc line
        if(_rayRayIntersect(X, Y, Inner, Outer, Min + Range, inner, outer, min + range))
          return true; // Check maxarc line segment intersection with other maxarc line

        if(_intersectRayCircle(X, Y, Outer, inner, outer, min, Min, Range))
          return true; // Check if our minarc ray intersects the other outer radius inside the arc
        if(_intersectRayCircle(X, Y, Outer, inner, outer, min + range, Min, Range))
          return true; // Check if our maxarc ray intersects the other outer radius inside the arc
        if(_intersectRayCircle(-X, -Y, outer, Inner, Outer, Min, min, range))
          return true; // Check if the other minarc ray intersects our outer radius inside the arc
        if(_intersectRayCircle(-X, -Y, outer, Inner, Outer, Min + Range, min, range))
          return true; // Check if the other maxarc ray intersects our outer radius inside the arc

        if(Inner > 0)
        {
          if(_intersectRayCircle(X, Y, Inner, inner, outer, min, Min, Range))
            return true; // Check if our minarc ray intersects the other inner radius inside the arc
          if(_intersectRayCircle(X, Y, Inner, inner, outer, min + range, Min, Range))
            return true; // Check if our maxarc ray intersects the other inner radius inside the arc
        }
        if(inner > 0)
        {
          if(_intersectRayCircle(-X, -Y, inner, Inner, Outer, Min, min, range))
            return true; // Check if the other minarc ray intersects our inner radius inside the arc
          if(_intersectRayCircle(-X, -Y, inner, Inner, Outer, Min + Range, min, range))
            return true; // Check if the other maxarc ray intersects our inner radius inside the arc
        }
      }

      return false; // If we had no valid intersection points, we don't intersect
    }

    // Constructs a line segment for the given circle segment angle and checks if it collides with the given line segment
    BUN_FORCEINLINE static bool _lineRayIntersect(T X1, T Y1, T X2, T Y2, T inner, T outer, T angle)
    {
      return LineSegmentLineSegmentCollide(cos(angle)*inner, -sin(angle)*inner, cos(angle)*outer, -sin(angle)*outer, X1, Y1, X2, Y2);
    }
    BUN_FORCEINLINE static bool _rayRayIntersect(T X, T Y, T Inner, T Outer, T Angle, T inner, T outer, T angle)
    {
      return _lineRayIntersect(X + cos(Angle)*Inner, Y - sin(Angle)*Inner, X + cos(Angle)*Outer, Y - sin(Angle)*Outer, inner, outer, angle);
    }
    BUN_FORCEINLINE static bool _intersectRayCircle(T X, T Y, T R, T inner, T outer, T angle, T Min, T Range)
    {
      Vector<T, 2> out[2];
      T c = cos(angle);
      T s = sin(angle);
      return _checkAngles(LineSegmentRadiusIntersect((c * inner) - X, -(s * inner) - Y, (c * outer) - X, -(s * outer) - Y, R, out), out, Min, Range);
    }

    BUN_FORCEINLINE bool _intersectAngleCircle(T X, T Y, T R, T angle)
    {
      T c = cos(angle);
      T s = sin(angle);
      T x1 = (c * inner) - X;
      T y1 = -(s * inner) - Y;
      if(LineSegmentRadiusCollide(x1, y1, (c * outer) - X, -(s * outer) - Y, R))
        return true;
      return x1*x1 + y1*y1 < R*R; // check for containment
    }
    // Assumes an already centered line and does not check for containment - used in polygon testing
    inline bool _intersectLine(T X1, T Y1, T X2, T Y2)
    {
      Vector<T, 2> out[2]; // Get intersection points of line for the circle and see if they lie inside the arc
      if(_checkAngles(LineSegmentRadiusIntersect(X1, Y1, X2, Y2, outer, out), out, min, range))
        return true;

      if(inner > 0 && _checkAngles(LineSegmentRadiusIntersect(X1, Y1, X2, Y2, inner, out), out, min, range))
        return true;

      // Check line segment intersections with the angle boundaries
      if(_lineRayIntersect(X1, Y1, X2, Y2, inner, outer, min))
        return true;

      if(_lineRayIntersect(X1, Y1, X2, Y2, inner, outer, min + range))
        return true;

      return false;
    }
    static BUN_FORCEINLINE bool _checkBothAngles(T X, T Y, int n, const Vector<T, 2>(&in)[2], T min, T range, T Min, T Range)
    {
      for(int i = 0; i < n; ++i)
        if(RadiusSectorContainsPointAngle(in[i].x, in[i].y, min, range) && RadiusSectorContainsPointAngle(in[i].x - X, in[i].y - Y, Min, Range))
          return true;
      return false;
    }

    static BUN_FORCEINLINE bool _checkAngles(int n, const Vector<T, 2>(&in)[2], T min, T range)
    {
      for(int i = 0; i < n; ++i)
        if(RadiusSectorContainsPointAngle(in[i].x, in[i].y, min, range))
          return true;
      return false;
    }
  };

  template<class T>
  inline Rect<T> PolygonAABB(const Vector<T, 2>* verts, size_t num)
  {
    assert(num > 1); //you can't be a 1D object or this is invalid
    Rect<T> retval(verts[0].x, verts[0].y, verts[0].x, verts[0].y);
    for(size_t i = 1; i < num; ++i)
      RectExpandTo(retval, verts[i]);
    return retval;
  }

  // 2D Polygon
  template<class T>
  struct Polygon
  {
    using V = Vector<T, 2>;

    inline Polygon(const Polygon<T>& copy) : _verts(copy._verts) {}
    inline Polygon(Polygon<T>&& mov) : _verts(std::move(mov._verts)) {}
    inline Polygon() {}
    template<class U>
    inline Polygon(const Polygon<U>& other) : _verts(other._verts.Capacity()) { for(size_t i = 0; i < other._verts.Capacity(); ++i) _verts[i] = other._verts[i]; }
    inline Polygon(const V* vertices, size_t num) : _verts(num) { if(_verts != 0) memcpy(_verts.begin(), vertices, _verts.Capacity() * sizeof(V)); }
    template <uint32_t num>
    inline Polygon(const V(&vertices)[num]) : _verts(num) { if(_verts != 0) memcpy(_verts.begin(), vertices, _verts.Capacity() * sizeof(V)); }
    template<class U>
    inline Polygon(const Vector<U, 2>* vertices, size_t num) : _verts(num) { for(size_t i = 0; i < _verts.Capacity(); ++i) _verts[i] = vertices[i]; }
    template <class U, uint32_t num>
    inline Polygon(const Vector<U, 2>(&vertices)[num]) : _verts(num) { for(size_t i = 0; i < _verts.Capacity(); ++i) _verts[i] = vertices[i]; }
    inline T Area() const { return PolygonArea<T>(_verts.begin(), _verts.Capacity()); }
    inline V Centroid() const { return PolygonCentroid<T>(_verts.begin(), _verts.Capacity()); }
    inline Rect<T> AABB() const { return PolygonAABB<T>(_verts.begin(), _verts.Capacity()); }
    inline bool IsConvex() const { return PolygonIsConvex<T>(_verts.begin(), _verts.Capacity()); }

    inline bool ContainsPoint(const Vector<T, 2>& point) const { return PolygonContainsPoint<T>(point, _verts.begin(), _verts.Capacity()); }
    inline bool ConvexContainsPoint(const Vector<T, 2>& point) const { return ConvexPolygonContainsPoint<T>(point, _verts.begin(), _verts.Capacity()); }
    inline void CenterPolygon()
    {
      V c = Centroid();
      for(size_t i = 0; i < _verts.Capacity(); ++i)
        _verts[i] -= c;
    }

    inline Polygon<T>& operator =(const Polygon<T>& right) { _verts = right._verts; return *this; }
    inline Polygon<T>& operator =(Polygon<T>&& right) { _verts = std::move(right._verts); return *this; }
    template<class U>
    inline Polygon<T>& operator =(const Polygon<U>& right)
    {
      _verts.SetCapacityDiscard(right._verts.Capacity());
      for(uint16_t i = 0; i < _verts.Capacity(); ++i)
        _verts[i] = right._verts[i];
      return *this;
    }
    inline const V& operator[](size_t i) const { return _verts[i]; }
    inline V& operator[](size_t i) { return _verts[i]; }

    inline const V* GetVertices() const { return _verts; }
    inline Array<V>& GetArray() { return _verts; }
    inline const Array<V>& GetArray() const { return _verts; }
    inline size_t GetCount() const { return _verts.Capacity(); }
    inline const V* begin() const { return _verts.begin(); }
    inline const V* end() const { return _verts.end(); }
    inline V* begin() { return _verts.begin(); }
    inline V* end() { return _verts.end(); }
    inline size_t size() const noexcept { return _verts.size(); }
    inline T* data() noexcept { return _verts.data(); }
    inline const T* data() const noexcept { return _verts.data(); }
    inline void SetVertices(const V* vertices, uint16_t num)
    {
      assert(vertices != _verts.begin());
      _verts.SetCapacityDiscard(num);
      if((const V*)_verts.begin())
        memcpy(_verts.begin(), vertices, _verts.Capacity() * sizeof(V));
    }
    template<size_t N>
    inline void SetVertices(const V(&vertices)[N]) { SetVertices(vertices, N); }

    using SerializerArray = T;
    template<typename Engine>
    void Serialize(Serializer<Engine>& s, const char* id) { _verts.Serialize(s, id); }

  protected:
    Array<V> _verts;
  };
}

#endif