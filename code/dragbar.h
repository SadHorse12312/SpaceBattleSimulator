#pragma once

#include "control.h"

struct Dragbar : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int nw = 12, nh = 30;
	int bw = 0, bh = 3;
	double depth = 1;
	double _min = 0, _max = 0;
	bool upload_only_done = false;

	int nx = 0;
	bool enabled = true;
	bool hovered = false;
	bool dragged = false;
	bool dragged_old = false;

	dcol3 c_invalid = { 100, 100, 100 };
	dcol3 c_dragged = { 250, 50, 50 };
	dcol3 c_hovered = { 150, 250, 250 };
	dcol3 c_normal = { 200, 200, 200 };
	
	Dragbar() = default;
	Dragbar(App *app, dvec2 const &tl, int bw, drect const &vp, double _min, double _max, bool uod = false, int nx = 0) : app(app), tl(tl), bw(bw), vp(vp), _min(_min), _max(_max), upload_only_done(uod), nx(nx) {}
	virtual ~Dragbar() {}

	int max_nx() const { return bw - nw; }
	drect node_rect() const { return { tl + dvec2(nx, 0), nw, nh }; }
	double Val() const { return nx * (_max - _min) / (double)max_nx() + _min; }
	virtual void UploadVal() const {}
	virtual bool CheckVal(double &v) const { return false; }

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return nh; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return inside(app->msp, node_rect()) && inside(app->msp, vp) ? d = depth, true : false;
	}

	void Discard() override {
		hovered = false;
		dragged = false;
		dragged_old = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);
		dragged_old = dragged;

		dragged = enabled && app->msd[0] && (dragged || hovered);
		if (dragged) {
			nx = clamp(app->msp.x - nw / 2 - tl.x, 0, max_nx());
			if (!upload_only_done) {
				UploadVal();
			}
		}
		else {
			if (dragged_old) {
				UploadVal();
			}
			double v;
			if (CheckVal(v)) {
				nx = int(max_nx() * (v - _min) / (_max - _min));
			}
		}
	}
	void Render() const override {
		dcol3 const &c = !enabled ? c_invalid : dragged ? c_dragged : hovered ? c_hovered : c_normal;
		draw_rect(app->scr, tl + dvec2(0, (nh - bh) / 2), bw, bh, vp, dcol3(120), 255);
		draw_rect_framed(app->scr, tl + dvec2(nx, 0), nw, nh, vp, c, 255, dcol3(50), 1);
	}
};

struct DbInt : Dragbar {
	int *val = NULL;

	DbInt() = default;
	DbInt(App *app, int *val, dvec2 const &tl, int bw, drect const &vp, double _min, double _max, bool uod = false) : val(val), Dragbar(app, tl, bw, vp, _min, _max, uod) {}

	void UploadVal() const override { *val = (int)Val(); }
	bool CheckVal(double &v) const override {
		v = *val;
		return true;
	}
};
struct DbDouble : Dragbar {
	double *val = NULL;

	DbDouble() = default;
	DbDouble(App *app, double *val, dvec2 const &tl, int bw, drect const &vp, double _min, double _max, bool uod = false) : val(val), Dragbar(app, tl, bw, vp, _min, _max, uod) {}

	void UploadVal() const override { *val = Val(); }
	bool CheckVal(double &v) const override {
		v = *val;
		return true;
	}
};

struct Scrollbar : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	int w = 20, h = 0;
	double depth = 1;
	CtrlListY *cl = NULL;

	int ny = 0, nh = 0;
	int x_dragged = 0;
	bool full = false;
	bool hovered = false;
	bool dragged = false;
	bool dragged_old = false;

	Scrollbar() = default;
	Scrollbar(App *app, CtrlListY *cl, dvec2 const &tl, int h) : app(app), cl(cl), tl(tl), h(h) {}

	int min_y() const { return cl->vp.bottom() - cl->GetHeight() - cl->gap; }
	int max_y() const { return cl->vp.top() + cl->gap; }
	int max_ny() const { return h - nh; }
	drect node_rect() const { return { tl + dvec2(0, ny), w, nh }; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return !full && inside(app->msp, node_rect()) ? d = depth, true : false;
	}

	void SetCl(CtrlListY *_cl) {
		hovered = false;
		dragged = false;
		dragged_old = false;
		cl = _cl;
	}
	void CheckVal() {
		ny = (max_y() - cl->y) * max_ny() / (max_y() - min_y());
	}

	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);
		dragged_old = dragged;

		full = cl == NULL || cl->vp.h >= cl->GetHeight();
		if (full) {
			dragged = false;
			dragged_old = false;
			return;
		}

		nh = cl->vp.h * h / (cl->GetHeight() + 2 * cl->gap);
		dragged = app->msd[0] && (dragged || hovered);
		if (dragged) {
			if (!dragged_old) {
				x_dragged = app->msp.y - (tl.y + ny);
			}
			else {
				ny = clamp(app->msp.y - x_dragged - tl.y, 0, max_ny());
				cl->y = max_y() - ny * (max_y() - min_y()) / max_ny();
			}
		}
		else {
			CheckVal();
		}
	}
	void Render() const override {
		draw_rect_framed(app->scr, tl, w, h, app->scr.rect(), dcol3(120), 255, dcol3(50), 1);
		if (!full) {
			dcol3 c_dragged = { 250, 50, 50 };
			dcol3 c_hovered = { 150, 250, 250 };
			dcol3 c_normal = { 200, 200, 200 };
			dcol3 const &c = dragged ? c_dragged : hovered ? c_hovered : c_normal;
			draw_rect_framed(app->scr, tl + dvec2(0, ny), w, nh, app->scr.rect(), c, 255, dcol3(50), 1);
		}	
	}
};

struct PnTmp : Control, Renderable, Hoverable {
	App *app = NULL;
	CtrlListY *cur = NULL;
	double depth = 1.5;
	dvec2 tl;
	int w = 0, h = 0;
	int h_close = 30, w_close = 80;
	Scrollbar *sb;
	int h_txt = 20;

	bool hovered = false;
	bool chosen = false;

	PnTmp() = default;
	PnTmp(App *app, Scrollbar *sb, dvec2 const &tl, int w, int h) : app(app), w(w), h(h), sb(sb), tl(tl) { 
		sb->h = h; 
		sb->tl = dvec2(tl.x + w - sb->w, tl.y);
		sb->depth = 1.6;
	}

	dvec2 close_tl() const { return dvec2(tl.x + w - w_close, tl.y - h_close); }
	drect close_rect() const { return { close_tl(), w_close, h_close }; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return inside(app->msp, close_rect()) || inside(app->msp, { tl, w, h }) ? d = depth, true : false;
	}

	void SetCur(CtrlListY *c) {
		if (cur) cur->Discard();
		cur = c;
		sb->SetCl(c);
	}

	void Update() override {
		chosen = hovered && inside(app->msp, close_rect()) && app->msd[0];
		if (hovered && inside(app->msp, close_rect()) && app->msd_old[0] && !app->msd[0]) {
			hovered = false;
			chosen = false;
			SetCur(NULL);
		}

		if (cur) {
			cur->Update();
			sb->Update();
			app->Renderables.push_back(this);
			app->Hoverables.push_back(this);
		}
	}
	void Render() const override {
		dcol3 c_chosen = { 250, 50, 50 };
		dcol3 c_hovered = { 150, 250, 250 };
		dcol3 c_normal = { 200, 200, 200 };
		dcol3 const &c = chosen ? c_chosen : hovered && inside(app->msp, close_rect()) ? c_hovered : c_normal;

		draw_rect_framed(app->scr, close_tl(), w_close, h_close, app->scr.rect(), c, 255, {}, 1);
		draw_rect_framed(app->scr, tl, w, h, app->scr.rect(), dcol3(140), 255, {}, 1);
		
		wstring s = L"¹Ø±Õ";
		dvec2 tl_s = close_tl() + dvec2(w_close - string_width(app->ft, s, h_txt), h_close - h_txt) / 2;
		draw_string(app->scr, app->ft, s, tl_s, h_txt, app->scr.rect());
	}
};
