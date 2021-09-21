#pragma once

#include "app.h"

struct MsgManager : Renderable {
	struct msg {
		wstring txt;
		double time_left = 0;
		dcol3 c = {};

		msg() = default;
		msg(wstring const &txt, double time_left, dcol3 const &c) : txt(txt), time_left(time_left), c(c) {}
	};
	App *app = NULL;
	dvec2 bottom_left;
	double depth = 1;
	int w = 0;
	int h_txt = 20;
	int max_msg_cnt = 5;
	double max_time_left = 2;
	double move_time = 0, max_move_time = 0.5;
	bool paused = false;
	deque<msg> msgs;

	MsgManager() = default;
	MsgManager(App *app, dvec2 const &bl, int w, int max_msg_cnt = 5) : app(app), bottom_left(bl), w(w), max_msg_cnt(max_msg_cnt) {}

	int h_msg() const { return h_txt * max_msg_cnt; }
	drect vp() const { return { bottom_left - dvec2(0, h_msg()), w, h_msg() }; }

	double GetDepth() const override { return depth; }

	void AddMsg(wstring const &txt, dcol3 const &c = {}) {
		msgs.push_back(msg(txt, max_time_left, c));
	}
	void Update() {
		app->Renderables.push_back(this);
		if (!paused) {
			for (auto &m : msgs) {
				m.time_left -= app->dt;
			}
		}

		if (!msgs.empty() && msgs.front().time_left < 0) {
			move_time += app->dt;
			if (move_time > max_move_time) {
				move_time = 0;
				msgs.pop_front();
			}
		}
	}
	void Render() const override {
		rep(i, 0, min(max_msg_cnt + 1, (int)msgs.size())) {
			draw_string(app->scr, app->ft, msgs[i].txt, bottom_left - dvec2(0, h_txt * (i + 1)) + dvec2(0, int(h_txt * move_time / max_move_time)), h_txt, vp(), msgs[i].c);
		}
	}
};