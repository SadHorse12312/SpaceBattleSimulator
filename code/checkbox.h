#pragma once

#include "control.h"

struct Checkbox : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	double depth = 1;
	bool checked = false;
	int h = 30, s_box = 20, s_inner_box = 10;

	bool enabled = true;
	bool hovered = false;

	Checkbox() = default;
	Checkbox(App *app, dvec2 const &tl, drect const &vp, int h = 30, bool checked = false) : app(app), tl(tl), vp(vp), h(h), checked(checked) {}

	dvec2 tl_box() const { return tl + dvec2(0, (h - s_box) / 2); }
	dvec2 tl_inner_box() const { return tl + dvec2(s_box, h) / 2 - dvec2(s_inner_box, s_inner_box) / 2; }
	drect rect() const { return { tl_box(), s_box, s_box }; }
	virtual void UploadVal() const {}
	virtual void CheckVal() {}

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return h; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return inside(app->msp, rect()) && inside(app->msp, vp) ? d = depth, true : false;
	}

	void Discard() override {
		hovered = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);

		if (enabled && hovered && app->msd[0] && !app->msd_old[0]) {
			checked = !checked;
			UploadVal();
		}
		else {
			CheckVal();
		}
	}
	void Render() const override {
		dcol3 c_invalid = { 100, 100, 100 };
		dcol3 c_hovered = { 150, 250, 250 };
		dcol3 c_normal = { 200, 200, 200 };
		dcol3 const &c = !enabled ? c_invalid : hovered ? c_hovered : c_normal;
		draw_rect_framed(app->scr, tl_box(), s_box, s_box, vp, c, 255, dcol3(50), 1);

		if (checked) {
			draw_rect_framed(app->scr, tl_inner_box(), s_inner_box, s_inner_box, vp, { 0, 255, 0 }, 255, dcol3(50), 1);
		}
	}
};

struct CbBool : Checkbox {
	bool *val = NULL;

	CbBool() = default;
	CbBool(App *app, bool *val, dvec2 const &tl, drect const &vp, int h = 30) : val(val), Checkbox(app, tl, vp, h) {}

	void UploadVal() const override { *val = checked; }
	void CheckVal() override { checked = *val; }
};