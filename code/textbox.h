#pragma once

#include "control.h"

struct Textbox : Control, Renderable, Hoverable {
	App *app = NULL;
	dvec2 tl;
	drect vp;
	int w = 0, h = 30;
	double depth = 1;
	int h_txt = 20;
	bool clear_upon_activated = false;
	bool upload_only_done = false;

	wstring left;
	deque<wchar_t> right;
	int x_str = 0;

	int cursor_margin_x = 10, cursor_margin_y = 5;
	int show_text_margin = 2;
	double timer = 0, cursor_move_interval = 0.1;

	bool enabled = true;
	bool edit = false;
	bool edit_old = false;
	bool hovered = false;
	bool cursor_move = false;

	dcol3 c_invalid = { 100, 100, 100 };
	dcol3 c_edit = { 250, 50, 50 };
	dcol3 c_hovered = { 150, 250, 250 };
	dcol3 c_normal = { 200, 200, 200 };
	
	Textbox() = default;
	Textbox(App *app, dvec2 const &tl, int w, drect const &vp, bool cua = false, bool uod = true, wstring const &txt = {}) : app(app), tl(tl), w(w), vp(vp), clear_upon_activated(cua), upload_only_done(uod), left(txt) {}
	virtual ~Textbox() {}

	int x_cursor() const { return x_str + string_width(app->ft, left, h_txt); }
	int max_x_cursor() const { return w - 2 * cursor_margin_x; }
	drect rect() const { return { tl, w, h }; }
	drect vp_string() const { return vp_intersection(vp, { tl + dvec2(show_text_margin, 0), w - 2 * show_text_margin, h }); }
	wstring Text() const { return left + wstring(right.begin(), right.end()); }
	virtual void UploadVal() const {};
	virtual void CheckVal() {};

	int GetY() const override { return tl.y; }
	void SetY(int y) override { tl.y = y; }
	int GetHeight() const override { return h; }

	double GetDepth() const override { return depth; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override {
		if (d > depth) return false;
		return inside(app->msp, rect()) && inside(app->msp, vp) ? d = depth, true : false;
	}

	void finish_edit() {
		edit = false;
		x_str = 0;
		left += wstring(right.begin(), right.end());
		right.clear();
	}
	void Disable() {
		finish_edit();
		enabled = false;
	}

	void Discard() override {
		finish_edit();

		hovered = false;
		cursor_move = false;
		edit_old = false;
	}
	void Update() override {
		app->Renderables.push_back(this);
		app->Hoverables.push_back(this);
		timer -= app->dt;
		edit_old = edit;
		cursor_move = false;

		if (app->msd[0] && !app->msd_old[0] && enabled) {
			if (hovered) {
				if (!edit && clear_upon_activated) {
					left.clear();
				}
				else {
					left += wstring(right.begin(), right.end());
				}
				right.clear();
				edit = true;

				while (!left.empty() && cursor_margin_x + x_cursor() > app->msp.x - tl.x) {
					right.push_front(left.back());
					left.pop_back();
				}
			}
			else if (edit) {
				finish_edit();
			}
		}
		if (app->msd[2] && !app->msd_old[2] && enabled) {
			if (hovered) {
				left.clear();
				right.clear();
				edit = true;
			}
			else if (edit) {
				finish_edit();
			}
		}

		if (edit) {
			add_chs(left, app->chs, { '\n', '\t', '\b' });
			if (app->kbd[VK_LEFT]) {
				cursor_move = true;
				if (!left.empty() && timer < 0) {
					timer = cursor_move_interval;
					right.push_front(left.back());
					left.pop_back();
				}
			}
			else if (app->kbd[VK_RIGHT]) {
				cursor_move = true;
				if (!right.empty() && timer < 0) {
					timer = cursor_move_interval;
					left.push_back(right.front());
					right.pop_front();
				}
			}
			else if (app->kbd[VK_BACK]) {
				if (!left.empty() && timer < 0) {
					timer = cursor_move_interval;
					left.pop_back();
				}
			}
			else if (app->kbd[VK_DELETE]) {
				if (!right.empty() && timer < 0) {
					timer = cursor_move_interval;
					right.pop_front();
				}
			}
			else if (app->kbd[VK_RETURN]) {
				finish_edit();
			}
		}

		if (edit) {
			if (x_cursor() > max_x_cursor()) {
				x_str -= x_cursor() - max_x_cursor();
			}
			else if (x_cursor() < 0) {
				x_str += -x_cursor();
			}

			if (!upload_only_done) { UploadVal(); }
		}
		else {
			if (edit_old) {
				UploadVal();
			}
			CheckVal();
		}
	}
	void Render() const override {
		dcol3 const &c = !enabled ? c_invalid : edit ? c_edit : hovered ? c_hovered : c_normal;
		draw_rect_framed(app->scr, tl, w, h, vp, c, 255, dcol3(50), 1);
		draw_string(app->scr, app->ft, Text(), tl + dvec2(cursor_margin_x + x_str, (h - h_txt) / 2), h_txt, vp_string());

		if (edit && sin(app->dur * 8) > 0 || cursor_move) {
			draw_rect(app->scr, tl + dvec2(cursor_margin_x + x_cursor(), cursor_margin_y), 2, h - 2 * cursor_margin_y, vp, {}, 255);
		}
	}
};

struct TbRangedInt : Textbox {
	int *val = NULL;
	int _min = 0, _max = 0;

	TbRangedInt() = default;
	TbRangedInt(App *app, int *val, int _min, int _max, dvec2 const &tl, int w, drect const &vp, bool cua = false, bool uod = true) : val(val), _min(_min), _max(_max), Textbox(app, tl, w, vp, cua, uod) {}

	void UploadVal() const override {
		*val = clamp((int)wtof(Text()), _min, _max);
	}
	void CheckVal() override {
		left = to_wstring(*val);
	}
};
struct TbInt : Textbox {
	int *val = NULL;

	TbInt() = default;
	TbInt(App *app, int *val, dvec2 const &tl, int w, drect const &vp, bool cua = false, bool uod = true) : val(val), Textbox(app, tl, w, vp, cua, uod) {}

	void UploadVal() const override {
		*val = (int)wtof(Text());
	}
	void CheckVal() override {
		left = to_wstring(*val);
	}
};
struct TbDouble : Textbox {
	double *val = NULL;

	TbDouble() = default;
	TbDouble(App *app, double *val, dvec2 const &tl, int w, drect const &vp, bool cua = false, bool uod = true) : val(val), Textbox(app, tl, w, vp, cua, uod) {}

	void UploadVal() const override {
		*val = wtof(Text());
	}
	void CheckVal() override {
		left = to_wstring(*val);
	}
};
struct TbText : Textbox {
	wstring *val = NULL;

	TbText() = default;
	TbText(App *app, wstring *val, dvec2 const &tl, int w, drect const &vp, bool cua = false, bool uod = true) : val(val), Textbox(app, tl, w, vp, cua, uod) {}

	void UploadVal() const override {
		*val = Text();
	}
	void CheckVal() override {
		left = *val;
	}
};