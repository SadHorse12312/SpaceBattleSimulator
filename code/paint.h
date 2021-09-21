#pragma once

#include "tile.h"
#include "font.h"

inline drect vp_intersection(drect const &a, drect const &b) {
	int left = max(a.left(), b.left());
	int right = min(a.right(), b.right());
	int top = max(a.top(), b.top());
	int bottom = min(a.bottom(), b.bottom());
	return { { left, top }, right - left, bottom - top };
}

typedef function<void(dcol3 &, BYTE &, dcol3 const &, BYTE)> filter;

inline void cover(dcol3 &bc, BYTE &ba, dcol3 const &fc, BYTE fa) {
	if (fa == 0) {
		return;
	}

	if (fa == 255) {
		bc = fc;
		ba = 255;
		return;
	}

	int a = 255 * (ba + fa) - ba * fa;
	int r = (bc.r * ba * 255 + fc.r * fa * 255 - bc.r * ba * fa) / a;
	int g = (bc.g * ba * 255 + fc.g * fa * 255 - bc.g * ba * fa) / a;
	int b = (bc.b * ba * 255 + fc.b * fa * 255 - bc.b * ba * fa) / a;
	a /= 255;

	bc = dcol3(r, g, b);
	ba = a;
}

inline filter flt_replace(dcol3 const &c, BYTE a, dcol3 const &c_origin = { 0, 255, 0 }, BYTE a_origin = 255) {
	return [c, a, c_origin, a_origin](dcol3 &c_out, BYTE &a_out, dcol3 const &c_in, BYTE a_in) {
		if (c_in == c_origin && a_in == a_origin) {
			c_out = c;
			a_out = a;
		}
		else {
			c_out = c_in;
			a_out = a_in;
		}
	};
}
inline void flt_trivial(dcol3 &c_out, BYTE &a_out, dcol3 const &c, BYTE a) {
	c_out = c;
	a_out = a;
}

inline void draw_copy(tile &dest, tile const &src) {
	memcpy(dest.cols.data(), src.cols.data(), src.n() * sizeof(dcol3));
	memcpy(dest.as.data(), src.as.data(), src.n());
}

inline void draw_tile_raw(tile &dest, dvec2 const &tl, drect const &vp_dest, tile const &src, drect const &vp_src) {
	int a = max(vp_dest.left(), tl.x), b = min(vp_dest.right(), tl.x + vp_src.w);
	rep(j, max(vp_dest.top(), tl.y), min(vp_dest.bottom(), tl.y + vp_src.h)) {
		int d_id = j * dest.w + a;
		int s_id = (j - tl.y + vp_src.top()) * src.w + a - tl.x + vp_src.left();

		memcpy(dest.cols.data() + d_id, src.cols.data() + s_id, sizeof(dcol3) * (size_t(b) - a));
		memset(dest.as.data() + d_id, 255, (size_t)b - a);
	}
}
inline void draw_tile(tile &dest, dvec2 const &tl, drect const &vp_dest, tile const &src, drect const &vp_src, filter f = flt_trivial) {
	int a = max(vp_dest.left(), tl.x);
	rep(j, max(vp_dest.top(), tl.y), min(vp_dest.bottom(), tl.y + vp_src.h)) {
		int d_id = j * dest.w + a;
		int s_id = (j - tl.y + vp_src.top()) * src.w + a - tl.x + vp_src.left();
		rep(i, a, min(vp_dest.right(), tl.x + vp_src.w)) {
			dcol3 sc;
			BYTE sa;
			f(sc, sa, src.cols[s_id], src.as[s_id]);
			cover(dest.cols[d_id], dest.as[d_id], sc, sa);

			d_id++;
			s_id++;
		}
	}
}
inline void draw_tile_scale(tile &dest, dvec2 const &tl, int w, int h, drect const &vp_dest, tile const &src, drect const &vp_src, filter f = flt_trivial) {
	rep(i, max(vp_dest.left(), tl.x), min(vp_dest.right(), tl.x + w))
		rep(j, max(vp_dest.top(), tl.y), min(vp_dest.bottom(), tl.y + h)) {
		dvec2 dp = { i , j };
		dvec2 sp = (dp - tl) * dvec2(vp_src.w, vp_src.h) / dvec2(w, h) + vp_src.topleft;

		int d_id = dp.y * dest.w + dp.x;
		int s_id = sp.y * src.w + sp.x;

		dcol3 sc;
		BYTE sa;
		f(sc, sa, src.cols[s_id], src.as[s_id]);
		cover(dest.cols[d_id], dest.as[d_id], sc, sa);
	}
}
inline void draw_tile_transform(tile &dest, vec2 const &ct, double w, double h, mat2 const &tsf, drect const &vp_dest, tile const &src, drect const &vp_src, filter f = flt_trivial) {
	mat2 inv_tsf = tsf.inv();
	dvec2 hwh_tsf = dvec2(tsf.abs() * vec2(w, h)) / 2 + dvec2(3, 3);

	rep(i, max(vp_dest.left(), (int)ct.x - hwh_tsf.x), min(vp_dest.right(), (int)ct.x + hwh_tsf.x))
		rep(j, max(vp_dest.top(), (int)ct.y - hwh_tsf.y), min(vp_dest.bottom(), (int)ct.y + hwh_tsf.y)) {
		dvec2 dp = { i, j };
		dvec2 sp = dvec2((inv_tsf * ((vec2)dp - ct) / vec2(w, h) + vec2(0.5)) * vec2(vp_src.w, vp_src.h)) + vp_src.topleft;
		if (inside(sp, vp_src)) {
			int d_id = dp.y * dest.w + dp.x;
			int s_id = sp.y * src.w + sp.x;

			dcol3 sc;
			BYTE sa;
			f(sc, sa, src.cols[s_id], src.as[s_id]);
			cover(dest.cols[d_id], dest.as[d_id], sc, sa);
		}
	}
}

inline void draw_eclipse(tile &dest, dvec2 const &ct, int aa, int bb, drect const &vp, dcol3 const &c, BYTE a) {
	rep(i, max(vp.left(), ct.x - (int)sqrt(aa) - 2), min(vp.right(), ct.x + (int)sqrt(aa) + 2))
		rep(j, max(vp.top(), ct.y - (int)sqrt(bb) - 2), min(vp.bottom(), ct.y + (int)sqrt(bb) + 2)) {
		dvec2 v = dvec2(i, j) - ct;
		if ((long long)v.x * v.x * bb + (long long)v.y * v.y * aa < (long long)aa * bb) {
			int id = j * dest.w + i;
			cover(dest.cols[id], dest.as[id], c, a);
		}
	}
}
inline void draw_eclipse_replace(tile &dest, dvec2 const &ct, int aa, int bb, drect const &vp, dcol3 const &c, BYTE a) {
	rep(i, max(vp.left(), ct.x - (int)sqrt(aa) - 2), min(vp.right(), ct.x + (int)sqrt(aa)) + 2)
		rep(j, max(vp.top(), ct.y - (int)sqrt(bb) - 2), min(vp.bottom(), ct.y + (int)sqrt(bb)) + 2) {
		dvec2 v = dvec2(i, j) - ct;
		if ((long long)v.x * v.x * bb + (long long)v.y * v.y * aa < (long long)aa * bb) {
			int id = j * dest.w + i;
			dest.cols[id] = c;
			dest.as[id] = a;
		}
	}
}

inline void draw_rect(tile &dest, dvec2 const &tl, int w, int h, drect const &vp, dcol3 const &c, BYTE a) {
	rep(i, max(vp.left(), tl.x), min(vp.right(), tl.x + w))
		rep(j, max(vp.top(), tl.y), min(vp.bottom(), tl.y + h)) {
		int id = j * dest.w + i;
		cover(dest.cols[id], dest.as[id], c, a);
	}
}
inline void draw_rect_replace(tile &dest, dvec2 const &tl, int w, int h, drect const &vp, dcol3 const &c, BYTE a) {
	rep(i, max(vp.left(), tl.x), min(vp.right(), tl.x + w))
		rep(j, max(vp.top(), tl.y), min(vp.bottom(), tl.y + h)) {
		int id = j * dest.w + i;
		dest.cols[id] = c;
		dest.as[id] = a;
	}
}
inline void draw_rect_transform(tile &dest, vec2 const &ct, double w, double h, mat2 const &tsf, drect const &vp, dcol3 const &c, BYTE a) {
	mat2 inv_tsf = tsf.inv();
	dvec2 hwh_tsf = dvec2(tsf.abs() * vec2(w, h)) + dvec2(3, 3);

	rep(i, max(vp.left(), (int)ct.x - hwh_tsf.x), min(vp.right(), (int)ct.x + hwh_tsf.x))
		rep(j, max(vp.top(), (int)ct.y - hwh_tsf.y), min(vp.bottom(), (int)ct.y + hwh_tsf.y)) {
		vec2 v = inv_tsf * (vec2(i, j) - ct);
		if (in_range(v.x, -0.5 * w, 0.5 * w) && in_range(v.y, -0.5 * h, 0.5 * h)) {
			int id = j * dest.w + i;
			cover(dest.cols[id], dest.as[id], c, a);
		}
	}
}

inline void draw_segment(tile &dest, dvec2 const &a, dvec2 const &b, int rr, drect const &vp, dcol3 const &fc, BYTE fa) {
	int r = (int)sqrt(rr) + 2;
	int ab = max(1, (a - b).lensqr());

	rep(i, max(vp.left(), min(a.x, b.x) - r), min(vp.right(), max(a.x, b.x) + r))
		rep(j, max(vp.top(), min(a.y, b.y) - r), min(vp.bottom(), max(a.y, b.y) + r)) {
		dvec2 p = { i , j };
		int ap = (p - a).lensqr(), bp = (p - b).lensqr();
		long long dif = (long long)ap - (long long)bp;
		int dd = ab + bp < ap ? bp : ab + ap < bp ? ap : int((ap + bp) / 2 - ab / 4 - dif * dif / (4LL * ab));
		if (dd < rr) {
			int id = j * dest.w + i;
			cover(dest.cols[id], dest.as[id], fc, fa);
		}
	}
}
inline void draw_segment_replace(tile &dest, dvec2 const &a, dvec2 const &b, int rr, drect const &vp, dcol3 const &fc, BYTE fa) {
	int r = (int)sqrt(rr) + 2;
	int ab = max(1, (a - b).lensqr());

	rep(i, max(vp.left(), min(a.x, b.x) - r), min(vp.right(), max(a.x, b.x) + r))
		rep(j, max(vp.top(), min(a.y, b.y) - r), min(vp.bottom(), max(a.y, b.y) + r)) {
		dvec2 p = { i , j };
		int ap = (p - a).lensqr(), bp = (p - b).lensqr();
		long long dif = (long long)ap - (long long)bp;
		int dd = ab + bp < ap ? bp : ab + ap < bp ? ap : int((ap + bp) / 2 - ab / 4 - dif * dif / (4LL * ab));
		if (dd < rr) {
			int id = j * dest.w + i;
			dest.cols[id] = fc;
			dest.as[id] = fa;
		}
	}
}

inline void draw_linestrip_replace(tile &dest, function<dvec2(int)> f, int n, int rr, drect const &vp, dcol3 const c, BYTE a) {
	if (n == 0) return;

	dvec2 va = f(0), vb;
	rep(i, 1, n) {
		vb = f(i);
		draw_segment_replace(dest, va, vb, rr, vp, c, a);
		va = vb;
	}
}
inline void draw_linestrip_replace(tile &dest, vector<dvec2> const &vs, int rr, drect const &vp, dcol3 const c, BYTE a) {
	draw_linestrip_replace(dest, [vs](int i) { return vs[i]; }, (int)vs.size(), rr, vp, c, a);
}
inline void draw_curve_replace(tile &dest, function<vec2(double)> f, double ta, double tb, int n, int rr, drect const &vp, dcol3 const &c, BYTE a) {
	draw_linestrip_replace(dest, [f, ta, tb, n](int i) { return (dvec2)f(ta + (tb - ta) * i / n); }, n + 1, rr, vp, c, a);
}

inline void draw_string(tile &dest, font const &ft, wstring const &s, dvec2 const &tl, int h, drect const &vp, dcol3 const &c = { 0, 0, 0 }, BYTE a = 255, int gap = 0) {
	filter f = flt_replace(c, a);
	dvec2 cur = tl;

	for (auto c : s) {
		tile const &x = ft[c];
		int w = h * x.w / x.h;
		draw_tile_scale(dest, cur, w, h, vp, x, x.rect(), f);
		cur.x += w + gap;
	}
}

inline void draw_paragraph(tile &dest, int w, font const &ft, wstring const &s, dvec2 const &tl, int h, drect const &vp, dcol3 const &color = { 0, 0, 0 }, BYTE a = 255, int x_gap = 0, int y_gap = 0) {
	dvec2 cur = tl;
	wstring cur_s;
	int cur_w = 0;

	int i = 0;
	while (i < s.size() && s[i] == L'\n') {
		cur.y += h + y_gap;
		++i;
	}
	if (i < s.size()) {
		cur_w = x_gap + ft[s[i]].w * h / ft[s[i]].h;
		cur_s = { s[i] };
	}
	else {
		return;
	}
	++i;

	while (i < s.size()) {
		wchar_t c = s[i];
		if (c == L'\n') {
			draw_string(dest, ft, cur_s, cur, h, vp, color, a, x_gap);
			while (i < s.size() && s[i] == L'\n') {
				cur.y += h + y_gap;
				++i;
			}
			if (i < s.size()) {
				cur_w = x_gap + ft[s[i]].w * h / ft[s[i]].h;
				cur_s = { s[i] };
			}
			else {
				return;
			}
		}
		else {
			int dw = x_gap + ft[c].w * h / ft[c].h;
			cur_w += dw;
			if (cur_w > w) {
				draw_string(dest, ft, cur_s, cur, h, vp, color, a, x_gap);
				cur_s.clear();
				cur.y += h + y_gap;
				cur_w = -x_gap + dw;
			}
			cur_s += c;
		}

		++i;
	}
	draw_string(dest, ft, cur_s, cur, h, vp, color, a, x_gap);
}

void draw_fill(tile &dest, dvec2 const &p, drect const &vp, dcol3 const &c, BYTE a);

#pragma region Specialized
inline void draw_rect_frame_transform(tile &dest, vec2 const &ct, double w, double h, mat2 const &tsf, drect const &vp, dcol3 const &c, int rr, int n = 40) {
	vec2 p0 = ct + tsf * vec2(w, h) / 2, p1 = ct + tsf * vec2(-w, h) / 2, p2 = ct + tsf * vec2(-w, -h) / 2, p3 = ct + tsf * vec2(w, -h) / 2;
	draw_curve_replace(dest, [p0, p1](double t) { return t * p0 + (1 - t) * p1; }, 0, 1, n, rr, vp, c, 255);
	draw_curve_replace(dest, [p1, p2](double t) { return t * p1 + (1 - t) * p2; }, 0, 1, n, rr, vp, c, 255);
	draw_curve_replace(dest, [p2, p3](double t) { return t * p2 + (1 - t) * p3; }, 0, 1, n, rr, vp, c, 255);
	draw_curve_replace(dest, [p3, p0](double t) { return t * p3 + (1 - t) * p0; }, 0, 1, n, rr, vp, c, 255);
}

inline void draw_rect_framed(tile &dest, dvec2 const &tl, int w, int h, drect const &vp, dcol3 const &c, BYTE a, dcol3 const &fc, int rr) {
	dvec2 p0 = tl, p1 = tl + dvec2(w, 0), p2 = tl + dvec2(w, h), p3 = tl + dvec2(0, h);
	draw_rect(dest, tl, w, h, vp, c, a);
	draw_linestrip_replace(dest, { p0, p1, p2, p3, p0 }, rr, vp, fc, 255);
}

inline void draw_eclipse_frame(tile &dest, vec2 const &ct, double a, double b, drect const &vp, dcol3 const &c, int rr, int n = 40) {
	draw_curve_replace(dest, [ct, a, b](double t) { return ct + vec2(cos(t), sin(t)) * vec2(a, b); }, 0, 2 * PI, n, rr, vp, c, 255);
}

inline void draw_eclipse_framed(tile &dest, vec2 const &ct, double a, double b, drect const &vp, dcol3 const &c, BYTE alpha, dcol3 const &fc, int rr, int n = 40) {
	draw_eclipse(dest, (dvec2)ct, int(a * a), int(b * b), vp, c, alpha);
	draw_eclipse_frame(dest, ct, a, b, vp, fc, rr, n);
}

inline void draw_rect_framed_transform(tile &dest, vec2 const &ct, double w, double h, mat2 const &tsf, drect const &vp, dcol3 const &c, BYTE a, dcol3 const &fc, int rr) {
	dvec2 p0 = dvec2(ct + tsf * vec2(w, h) / 2), p1 = dvec2(ct + tsf * vec2(-w, h) / 2), p2 = dvec2(ct + tsf * vec2(-w, -h) / 2), p3 = dvec2(ct + tsf * vec2(w, -h) / 2);
	draw_rect_transform(dest, ct, w, h, tsf, vp, c, a);
	draw_linestrip_replace(dest, { p0, p1, p2, p3, p0 }, rr, vp, fc, 255);
}

inline void draw_grid(tile &dest, vec2 const &ct, double s, int nx, int ny, int rr, mat2 const &tsf, drect const &vp, dcol3 const &c) {
	double w = nx * s, h = ny * s;
	rep(i, 0, nx + 1) {
		vec2 a = -vec2(w, h) / 2 + vec2(i, 0) * s;
		a = ct + tsf * a;
		vec2 b = -vec2(w, h) / 2 + vec2(i, ny) * s;
		b = ct + tsf * b;
		draw_curve_replace(dest, [a, b](double t) { return t * a + (1 - t) * b; }, 0, 1, 60, rr, vp, c, 255);
	}
	rep(i, 0, ny + 1) {
		vec2 a = -vec2(w, h) / 2 + vec2(0, i) * s;
		a = ct + tsf * a;
		vec2 b = -vec2(w, h) / 2 + vec2(nx, i) * s;
		b = ct + tsf * b;
		draw_curve_replace(dest, [a, b](double t) { return t * a + (1 - t) * b; }, 0, 1, 60, rr, vp, c, 255);
	}
}
#pragma endregion
