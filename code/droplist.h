#pragma once

#include "control.h"

struct Droplist : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int w = 0, h = 30;
	double depth = 1, depth_list = 1.1;
	int h_txt = 20;
	vector<wstring> items;
	int picked = 0;

	int text_margin = 10;

	RenderableCasual rc_list;
	int item_hovered = 0;
	bool enabled = true;
	bool hovered = false;
	bool open = false;

	dcol3 c_invalid = { 100, 100, 100 };
	dcol3 c_open = { 250, 50, 50 };
	dcol3 c_hovered = { 150, 250, 250 };
	dcol3 c_normal = { 200, 200, 200 };

	Droplist() = default;
	Droplist(App *app, vector<wstring> const &items, dvec2 const &tl, int w, drect const &vp, int picked = 0) : app(app), items(items), tl(tl), w(w), vp(vp), picked(picked) {}
	~Droplist() = default;

	drect rect() const {
		return open ? drect(tl, w, ((int)items.size() + 1) * h) : drect(tl, w, h);
	}
	virtual void CheckVal() {}
	virtual void UploadVal() const {}

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return h; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return inside(app->msp, rect()) && inside(app->msp, vp) ? d = depth, true : false;
	}

	void disable() {
		open = false;
		enabled = false;
	}

	void Discard() override {
		hovered = false;
		open = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);

		if (open) {
			if (hovered) {
				item_hovered = (app->msp.y - tl.y) / h - 1;
				if (app->msd[0] && !app->msd_old[0]) {
					if (item_hovered != -1) {
						picked = item_hovered;
					}
					open = false;
				}
			}
			else if (app->msd[0] && !app->msd_old[0]) {
				open = false;
			}

			rc_list = RenderableCasual([this]() {
				draw_rect_framed(app->scr, tl + dvec2(0, h), w, (int)items.size() * h, vp, c_normal, 255, dcol3(50), 1);
				if (item_hovered != -1) {
					draw_rect_framed(app->scr, tl + dvec2(0, h * (item_hovered + 1)), w, h, vp, c_hovered, 255, dcol3(50), 1);
				}
				rep(i, 0, items.size()) {
					draw_string(app->scr, app->ft, items[i], tl + dvec2(0, h * (i + 1)) + dvec2(text_margin, (h - h_txt) / 2), h_txt, vp);
				}
				}, depth_list);
			app->Renderables.push_back(&rc_list);
		}
		else if (enabled && hovered && app->msd[0] && !app->msd_old[0]) {
			open = true;
			item_hovered = -1;
		}
	}
	void Render() const override {
		dcol3 const &c = !enabled ? c_invalid : open ? c_open : hovered ? c_hovered : c_normal;
		draw_rect_framed(app->scr, tl, w, h, vp, c, 255, dcol3(50), 2);
		draw_string(app->scr, app->ft, items[picked], tl + dvec2(text_margin, (h - h_txt) / 2), h_txt, vp);
		draw_eclipse(app->scr, tl + dvec2(w - 15, h / 2), 25, 25, vp, dcol3(50), 255);
	}
};

struct DpInt : Droplist {
	int *val = NULL;

	DpInt() = default;
	DpInt(App *app, int *val, vector<wstring> const &items, dvec2 const &tl, int w, drect const &vp) : val(val), Droplist(app, items, tl, w, vp) {}

	void CheckVal() override { picked = *val; }
	void UploadVal() const override { *val = picked; }
};