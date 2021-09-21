#pragma once

#include "control.h"

struct Info : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	wstring info;
	double depth = 1, depth_info = 1.2;
	drect vp;
	int h = 30, r = 10, h_mark = 18;
	int w_info, h_txt = 20, gap_info = 10;

	RenderableCasual rc_info;
	dvec2 show_tl;
	double time_to_show = 0, max_time_to_show = 0.3;
	bool enabled = true;
	bool hovered = false;
	bool show = false;
	bool show_old = false;

	Info() = default;
	Info(App *app, wstring info, dvec2 const &tl, int w_info, drect const &vp, int h = 30) : app(app), info(info), tl(tl), w_info(w_info), vp(vp), h(h) {}
	virtual ~Info() {}

	dvec2 ct() const { return tl + dvec2(r, h / 2); }
	virtual void CheckVal() {}

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return h; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return (app->msp - ct()).lensqr() < r * r ? d = depth, true : false;
	}

	void Discard() override {
		hovered = false;
		show = false;
		show_old = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);
		show_old = show;
		time_to_show -= app->dt;

		show = enabled && hovered;
		if (show) {
			if (!show_old) {
				show_tl = app->msp + dvec2(0, r);
				time_to_show = max_time_to_show;
			}			

			if (time_to_show < 0) {
				CheckVal();

				int h_info = paragraph_height(app->ft, info, h_txt, w_info);
				dvec2 tl_info = dvec2(min(show_tl.x, app->scr.w - w_info - 2 * gap_info), min(show_tl.y, app->scr.h - h_info - 2 * gap_info));
				rc_info = RenderableCasual([this, h_info, tl_info]() {
					draw_rect_framed(app->scr, tl_info, w_info + 2 * gap_info, h_info + 2 * gap_info, app->scr.rect(), { 200, 200, 120 }, 255, dcol3(50), 1);
					draw_paragraph(app->scr, w_info, app->ft, info, tl_info + dvec2(gap_info, gap_info), h_txt, app->scr.rect());
					}, depth_info);
				app->Renderables.push_back(&rc_info);
			}
		}
	}
	void Render() const override {
		dcol3 c_invalid = { 100, 100, 100 };
		dcol3 c_hovered = { 150, 250, 250 };
		dcol3 c_normal = { 200, 200, 200 };
		dcol3 const &c = !enabled ? c_invalid : hovered ? c_hovered : c_normal;
		draw_eclipse_framed(app->scr, (vec2)ct(), r, r, vp, c, 255, dcol3(50), 2);

		tile const &mark = app->ft[L'?'];
		int w = mark.w * h_mark / mark.h;
		draw_tile_scale(app->scr, ct() - dvec2(w, h_mark) / 2 - dvec2(1, 0), w, h_mark, vp, mark, mark.rect(), flt_replace({}, 255));
	}
};

struct IfCasual : Info {
	function<void()> f;

	IfCasual() = default;
	IfCasual(App *app, function<void()> f, dvec2 const &tl, int w_info, drect const &vp, int h = 30) : f(f), Info(app, L"", tl, w_info, vp, h) {}

	void CheckVal() override { f(); }
};