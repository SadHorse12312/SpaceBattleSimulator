#pragma once

#include "app.h"

struct Control {
	virtual void Update() {}
	virtual void Load() {}
	virtual void Discard() {}

	virtual int GetY() const { return 0; }
	virtual void SetY(int y) {}
	virtual int GetHeight() const { return 0; }

	virtual ~Control() {}
};

struct Panel : Control {
	App *app = NULL;
	Control *cur = NULL;

	Panel() = default;
	Panel(App *app, Control *c = NULL) : app(app), cur(c) {}

	void SetCur(Control *c) {
		if (cur) cur ->Discard();
		cur = c;
	}

	void Update() override {
		if (cur) {
			cur->Update();
		}
	}
};

struct CtrlGroup : Control {
	App *app = NULL;
	int y = 0, h = 0;
	vector<Control *> cs;

	CtrlGroup() = default;
	CtrlGroup(App *app, vector<Control *> const &cs, int y = 0, int h = 30) : app(app), cs(cs), y(y), h(h) {}

	int GetY() const override { return y; }
	void SetY(int _y) override { 
		for (auto c : cs) {
			c->SetY(c->GetY() + _y - y);
		}
		y = _y;
	}
	int GetHeight() const override { return h; }

	void Discard() override { for (auto c : cs) c->Discard(); }
	void Update() override { for (auto c : cs) c->Update(); }
};

struct CtrlListY : Control {
	App *app = NULL;
	drect vp;
	int y = 0;
	int gap = 10;
	vector <Control *> cs;

	CtrlListY() = default;
	CtrlListY(App *app, vector<Control *> const &cs, int y, drect const &vp) : app(app), cs(cs), y(y), vp(vp) {}

	int GetY() const override { return y; }
	void SetY(int _y) override { y = _y; }
	int GetHeight() const override {
		if (cs.empty()) return 0;

		int h = 0;
		for (auto c : cs) {
			h += c->GetHeight();
		}
		return h + ((int)cs.size() - 1) * gap;
	}

	void Discard() override { for (auto c : cs) c->Discard(); }
	void Update() override {
		int cur_y = y;
		for (auto c : cs) {
			c->SetY(cur_y);
			c->Update();
			cur_y += c->GetHeight() + gap;
		}
	}
};