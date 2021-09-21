#include "paint.h"

struct fill_info {
	dvec2 p;
	int direction = 0;
};

void sub_fill(tile &dest, vector<fill_info> &stack, drect const &vp, dcol3 const &c_before, BYTE a_before, dcol3 const &c_after, BYTE a_after) {
	fill_info &f = stack.back();
	dvec2 p = f.p;

	switch (f.direction) {
	case 0: p += { 1, 0 }; break;
	case 1: p += { 0, 1 }; break;
	case 2: p += { -1, 0 }; break;
	case 3: p += { 0, -1 }; break;
	}

	int id = p.y * dest.w + p.x;
	if (inside(p, vp) && dest.cols[id] == c_before && dest.as[id] == a_before) {
		dest.cols[id] = c_after;
		dest.as[id] = a_after;
		stack.push_back({ p, 0 });
	}
	else {
		++f.direction;
		if (f.direction == 4) stack.pop_back();
	}
}

void draw_fill(tile &dest, dvec2 const &p, drect const &vp, dcol3 const &c, BYTE a) {
	if (!inside(p, vp)) return;

	int id = p.y * dest.w + p.x;
	dcol3 c_before = dest.cols[id];
	BYTE a_before = dest.as[id];
	if (c_before == c && a_before == a) return;

	dest.cols[id] = c;
	dest.as[id] = a;

	vector<fill_info> stack = { { p, 0 } };
	while (!stack.empty()) sub_fill(dest, stack, vp, c_before, a_before, c, a);
}
