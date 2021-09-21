#pragma once

#include "linear.h"

struct dcol3 {
	BYTE b = 0, g = 0, r = 0;

	dcol3() = default;
	dcol3(BYTE r, BYTE g, BYTE b) : r(r), g(g), b(b) {}
	explicit dcol3(BYTE a) : dcol3(a, a, a) {}
};

inline bool operator==(dcol3 const &a, dcol3 const &b) { return memcmp(&a, &b, sizeof(dcol3)) == 0; }
inline bool operator!=(dcol3 const &a, dcol3 const &b) { return !(a == b); }

struct tile {
	int w = 0, h = 0;
	vector<dcol3> cols;
	vector<BYTE> as;

	int n() const { return w * h; }
	drect rect() const { return { {}, w, h }; }

	tile() = default;
	tile(int w, int h) : w(w), h(h), cols(n()), as(n()) {}

	void make_transparent() { memset(as.data(), 0, n()); }
	void fill(dcol3 const &c, BYTE a = 255) {
		rep(i, 0, n()) { cols[i] = c; }
		memset(as.data(), a, n());
	}

	void save(wstring const &nm) const {
		FILE *f = wfopen(nm, "wb");
		fwt(w); fwt(h);
		fwts(&cols[0], n());
		fwts(&as[0], n());
		fclose(f);
	}
	static tile load(wstring const &nm) {
		FILE *f = wfopen(nm, "rb");
		int w = 0, h = 0;
		frd(w); frd(h);
		tile tmp(w, h);
		frds(&tmp.cols[0], tmp.n());
		frds(&tmp.as[0], tmp.n());
		fclose(f);
		return tmp;
	}
	static tile clip(tile const &scr, drect const &vp, int w, int h) {
		tile t(w, h);
		rep(i, 0, w) rep(j, 0, h) {
			int x = vp.left() + vp.w * i / w;
			int y = vp.top() + vp.h * j / h;

			int s_id = y * scr.w + x;
			int d_id = j * w + i;

			t.cols[d_id] = scr.cols[s_id];
			t.as[d_id] = scr.as[s_id];
		}
		return t;
	}
};