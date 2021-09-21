#pragma once

#include "control.h"

struct Label : Control, Renderable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int w = 0, h = 30;
	dcol3 color = {};
	double depth = 1;
	int h_txt = 20;
	wstring txt = L"Label";
	enum _alignment { LEFT, MIDDLE, RIGHT } alignment = LEFT;

	Label() = default;
	Label(App *app, wstring const &txt, dvec2 const &tl, int w, drect const &vp, _alignment al = LEFT, dcol3 const &color = {}, int h = 30) : app(app), txt(txt), tl(tl), w(w), vp(vp), alignment(al), color(color), h(h) {}
	virtual ~Label() {}

	virtual void CheckVal() {}

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return h; }

	double GetDepth() const override { return depth; }

	void Update() override {
		app->Renderables.push_back(this);
		CheckVal();
	}
	void Render() const override {
		dvec2 tl_txt;
		switch (alignment) {
		case LEFT: tl_txt = tl + dvec2(0, h / 2) - dvec2(0, h_txt / 2); break;
		case MIDDLE: tl_txt = tl + dvec2(w, h) / 2 - dvec2(string_width(app->ft, txt, h_txt), h_txt) / 2; break;
		case RIGHT: tl_txt = tl + dvec2(w, h / 2) - dvec2(string_width(app->ft, txt, h_txt), h_txt / 2); break;
		}
		draw_string(app->scr, app->ft, txt, tl_txt, h_txt, vp, color);
	}
};

struct LbCasual : Label {
	function<void()> f;

	LbCasual() = default;
	LbCasual(App *app, function<void()> f, dvec2 const &tl, int w, drect const &vp, _alignment al = LEFT, dcol3 const &color = {}, int h = 30) : f(f), Label(app, {}, tl, w, vp, al, color, h) {}

	void CheckVal() override { f(); }
};

struct LbRichVar : Control, Renderable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int w = 0;
	int h_txt = 20;
	double depth = 1.6;
	wstring txt = L"";

	LbRichVar() = default;
	LbRichVar(App *app, wstring const &txt, dvec2 const &tl, int w, drect const &vp) : app(app), txt(txt), tl(tl), w(w), vp(vp) {}
	virtual ~LbRichVar() {}

	virtual void CheckVal() {}

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return paragraph_height(app->ft, txt, h_txt, w); }

	double GetDepth() const override { return depth; }

	void Update() override {
		app->Renderables.push_back(this);
		CheckVal();
	}
	void Render() const override {
		draw_paragraph(app->scr, w, app->ft, txt, tl, h_txt, vp);
	}
};