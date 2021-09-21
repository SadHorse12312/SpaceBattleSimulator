#pragma once

#include "utils.h"

#pragma region Discrete
inline int mod(int a, int b) { return (a % b + b) % b; }

struct dvec2 {
	int x = 0, y = 0;

	dvec2() = default;
	dvec2(int x, int y) : x(x), y(y) {}

	dvec2 const &operator+() const { return *this; }
	dvec2 operator-() const { return { -x, -y }; }
	dvec2 &operator+=(dvec2 const &a) { x += a.x; y += a.y; return *this; }
	dvec2 &operator-=(dvec2 const &a) { return *this += -a; }
	dvec2 &operator*=(dvec2 const &a) { x *= a.x; y *= a.y; return *this; }
	dvec2 &operator/=(dvec2 const &a) { x /= a.x; y /= a.y; return *this; }
	dvec2 &operator*=(int a) { x *= a; y *= a; return *this; }
	dvec2 &operator/=(int a) { x /= a; y /= a; return *this; }

	int lensqr() const { return x * x + y * y; }
};

inline dvec2 operator+(dvec2 a, dvec2 const &b) { return a += b; }
inline dvec2 operator-(dvec2 a, dvec2 const &b) { return a -= b; }
inline dvec2 operator*(dvec2 a, dvec2 const &b) { return a *= b; }
inline dvec2 operator/(dvec2 a, dvec2 const &b) { return a /= b; }
inline dvec2 operator*(dvec2 a, int b) { return a *= b; }
inline dvec2 operator/(dvec2 a, int b) { return a /= b; }
inline dvec2 operator*(int a, dvec2 const &b) { return b * a; }

inline bool operator==(dvec2 const &a, dvec2 const &b) { return memcmp(&a, &b, sizeof(dvec2)) == 0; }
inline bool operator!=(dvec2 const &a, dvec2 const &b) { return !(a == b); }

struct drect {
	dvec2 topleft;
	int w = 0, h = 0;

	drect() = default;
	drect(dvec2 const &tl, int w, int h) : topleft(tl), w(w), h(h) {}
	drect(int w, int h) : drect({}, w, h) {}

	int left() const { return topleft.x; }
	int top() const { return topleft.y; }
	int right() const { return left() + w; }
	int bottom() const { return top() + h; }
};

inline bool inside(dvec2 const &v, drect const &r) {
	return in_range(v.x, r.left(), r.right() - 1) && in_range(v.y, r.top(), r.bottom() - 1);
}
#pragma endregion

struct vec2 {
	double x = 0, y = 0;

	vec2() = default;
	vec2(double x, double y) : x(x), y(y) {}
	explicit vec2(double a) : vec2(a, a) {}

	vec2 const &operator+() const { return *this; }
	vec2 operator-() const { return { -x, -y }; }
	vec2 &operator+=(vec2 const &a) { x += a.x; y += a.y; return *this; }
	vec2 &operator-=(vec2 const &a) { return *this += -a; }
	vec2 &operator*=(vec2 const &a) { x *= a.x; y *= a.y; return *this; }
	vec2 &operator/=(vec2 const &a) { x /= a.x; y /= a.y; return *this; }
	vec2 &operator*=(double a) { x *= a; y *= a; return *this; }
	vec2 &operator/=(double a) { return *this *= 1 / a; }
	explicit operator dvec2() const { return dvec2(int(x), int(y)); }
	explicit vec2(dvec2 const &v) : x(v.x), y(v.y) {}

	double lensqr() const { return x * x + y * y; }
};

inline double dot(vec2 const &a, vec2 const &b) { return a.x * b.x + a.y * b.y; }

inline vec2 operator+(vec2 a, vec2 const &b) { return a += b; }
inline vec2 operator-(vec2 a, vec2 const &b) { return a -= b; }
inline vec2 operator*(vec2 a, vec2 const &b) { return a *= b; }
inline vec2 operator/(vec2 a, vec2 const &b) { return a /= b; }
inline vec2 operator*(vec2 a, double b) { return a *= b; }
inline vec2 operator/(vec2 a, double b) { return a /= b; }
inline vec2 operator*(double a, vec2 const &b) { return b * a; }

inline bool operator==(vec2 const &a, vec2 const &b) { return memcmp(&a, &b, sizeof(vec2)) == 0; }
inline bool operator!=(vec2 const &a, vec2 const &b) { return !(a == b); }

struct rect {
	vec2 topleft;
	double w = 0, h = 0;

	rect() = default;
	rect(vec2 const &tl, double w, double h) : topleft(tl), w(w), h(h) {}
	rect(double w, double h) : rect({}, w, h) {}

	double left() const { return topleft.x; }
	double top() const { return topleft.y; }
	double right() const { return left() + w; }
	double bottom() const { return top() + h; }
};

inline bool inside(vec2 const &v, rect const &r) {
	return in_range(v.x, r.left(), r.right()) && in_range(v.y, r.top(), r.bottom());
}

struct mat2;
inline mat2 operator/(mat2 a, double b);

struct mat2 {
	double a00 = 0, a01 = 0, a10 = 0, a11 = 0;

	mat2() = default;
	mat2(double a00, double a01, double a10, double a11) : a00(a00), a01(a01), a10(a10), a11(a11) {}

	static mat2 rot(double rad) {
		double c = cos(rad), s = sin(rad);
		return { c, -s, s, c };
	}
	static mat2 id() { return { 1, 0, 0, 1 }; }

	mat2 const &operator+() const { return *this; }
	mat2 operator-() const { return { -a00, -a01, -a10, -a11 }; }
	mat2 &operator+=(mat2 const &a) { a00 += a.a00; a01 += a.a01; a10 += a.a10; a11 += a.a11; return *this; }
	mat2 &operator-=(mat2 const &a) { return *this += -a; }
	mat2 &operator*=(double a) { a00 *= a; a01 *= a; a10 *= a; a11 *= a; return *this; }
	mat2 &operator/=(double a) { return *this *= 1 / a; }

	double det() const { return a00 * a11 - a01 * a10; }
	mat2 tsp() const { return { a00, a10, a01, a11 }; }
	mat2 inv() const { return mat2(a11, -a01, -a10, a00) / det(); }
	mat2 abs() const { return mat2(::abs(a00), ::abs(a01), ::abs(a10), ::abs(a11)); }
};

inline mat2 operator+(mat2 a, mat2 const &b) { return a += b; }
inline mat2 operator-(mat2 a, mat2 const &b) { return a -= b; }
inline mat2 operator*(mat2 a, double b) { return a *= b; }
inline mat2 operator/(mat2 a, double b) { return a /= b; }
inline mat2 operator*(double a, mat2 const &b) { return b * a; }
inline vec2 operator*(mat2 const &a, vec2 const &b) { return { a.a00 * b.x + a.a01 * b.y, a.a10 * b.x + a.a11 * b.y }; }
inline mat2 operator*(mat2 const &a, mat2 const &b) {
	return { a.a00 * b.a00 + a.a01 * b.a10, a.a00 * b.a01 + a.a01 * b.a11, a.a10 * b.a00 + a.a11 * b.a10, a.a10 * b.a01 + a.a11 * b.a11 };
}