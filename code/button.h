#pragma once

#include "control.h"

struct Button : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int w = 0, h = 30;
	double depth = 1;
	int h_txt = 20;
	wstring txt = L"°´Å¥";

	bool enabled = true;
	bool chosen = false;
	bool hovered = false;

	dcol3 c_invalid = { 100, 100, 100 };
	dcol3 c_chosen = { 250, 50, 50 };
	dcol3 c_hovered = { 150, 250, 250 };
	dcol3 c_normal = { 200, 200, 200 };

	Button() = default;
	Button(App *app, wstring const &txt, dvec2 const &tl, int w, drect const &vp) : app(app), txt(txt), tl(tl), w(w), vp(vp) {}
	virtual ~Button() {}

	drect rect() const { return { tl, w, h }; }
	virtual void OnClick() {}

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
		chosen = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);

		chosen = enabled && hovered && app->msd[0];
		if (enabled && hovered && app->msd_old[0] && !app->msd[0]) OnClick();
	}
	void Render() const override {
		dcol3 const &c = !enabled ? c_invalid : chosen ? c_chosen : hovered ? c_hovered : c_normal;
		draw_rect_framed(app->scr, tl, w, h, vp, c, 255, dcol3(50), 2);

		dvec2 tl_txt = tl + dvec2(w, h) / 2 - dvec2(string_width(app->ft, txt, h_txt), h_txt) / 2;
		draw_string(app->scr, app->ft, txt, tl_txt, h_txt, vp);
	}
};

struct BtCasual : Button {
	function<void()> f;

	BtCasual() = default;
	BtCasual(App *app, function<void()> f, wstring const &txt, dvec2 const &tl, int w, drect const &vp) : f(f), Button(app, txt, tl, w, vp) {}

	void OnClick() override { f(); }
};