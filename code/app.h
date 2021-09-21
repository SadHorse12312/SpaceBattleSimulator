#pragma once

#include "font.h"
#include "paint.h"

struct Renderable {
	virtual void Render() const {};
	virtual double GetDepth() const { return 0; };
	virtual ~Renderable() {}
};

struct Hoverable {
	virtual void SetHovered(bool h) {};
	virtual bool TestHover(double &d) const { return false; };
	virtual ~Hoverable() {}
};

struct App {
	wstring title;
	int w = 0, h = 0;
	dcol3 bgc = { 20, 40, 80 };
	bool use_sys_cursor = true;
	bool loaded = false;

	map<wstring, shared_ptr<tile>> Resources;
	vector<Hoverable *> Hoverables;
	vector<Renderable *> Renderables;

	wstring class_name = L"class";
	HWND hwnd = NULL;
	HINSTANCE hinst = NULL;
	BITMAPINFO bmi{};

	tile scr;
	font ft;

	double dur = 0, dt = 0;
	double fps = 0, frm_time = 0;
	int frm_cnt = 0, ms_cnt = 0, frm_cnt_max = 10;
	wstring fps_txt;
	std::chrono::high_resolution_clock::time_point tm;

	double msw{};
	wstring chs;
	dvec2 msp, msp_old, msp_raw;
	array<bool, 3> msd{}, msd_old{};
	array<bool, 256> kbd{}, kbd_old{};
	bool moving = true;

	int w_cursor = 50, h_cursor = 50;
	HCURSOR sys_cursor = NULL;
	wstring user_cursor;

	App() = default;
	App(wstring const &tt, int w, int h, dcol3 const &bgc = { 20, 40, 80 }, bool usc = true, wstring const &user_cursor = L"") : title(tt), w(w), h(h), bgc(bgc), use_sys_cursor(usc), scr(w, h), user_cursor(user_cursor) {}

	void add_resource(wstring const &nm) {
		wstring path = L"resources/" + nm + L".tile";
		if (Resources.find(nm) == Resources.end()) {
			Resources[nm] = make_shared<tile>(tile::load(path));
			print_console(L"已加载资源: " + nm + L".", true);
		}
	}

	void register_window_class();
	void create_window() {
		RECT cr{};
		cr.right = w; 
		cr.bottom = h;
		DWORD sty = WS_SYSMENU | WS_CAPTION;
		AdjustWindowRect(&cr, sty, FALSE);

		int rw = cr.right - cr.left, rh = cr.bottom - cr.top;
		hwnd = CreateWindowExW(0, class_name.c_str(), title.c_str(), sty, 0, 0, rw, rh, NULL, NULL, hinst, this);
	}
	void init_bmi() {
		BITMAPINFOHEADER hd{};
		hd.biSize = sizeof(hd);
		hd.biWidth = w;
		hd.biHeight = -h;
		hd.biPlanes = 1;
		hd.biBitCount = 24;
		hd.biCompression = BI_RGB;
		bmi.bmiHeader = hd;
	}

	void calc_fps() {
		auto now = std::chrono::high_resolution_clock::now();
		auto dif = now - tm;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dif).count();

		tm = now;
		frm_cnt++;
		ms_cnt += (int)ms;
		dur += dt = moving ? 0 : ms / 1000.0;

		if (frm_cnt == frm_cnt_max) {
			frm_time = (double)ms_cnt / frm_cnt;
			fps = frm_time == 0 ? 0 : 1000.0 / frm_time;
			frm_cnt = ms_cnt = 0;
		}

		static wchar_t tmp[64];
		swprintf_s(tmp, L"帧时间: %.2lf ms, FPS: %.2lf.", frm_time, fps);
		fps_txt = tmp;
	}
	void update_msp() {
		POINT tile{};
		GetCursorPos(&tile);
		ScreenToClient(hwnd, &tile);

		msp_old = msp;
		msp_raw = { tile.x, tile.y };
		msp.x = clamp(msp_raw.x, 0, w - 1);
		msp.y = clamp(msp_raw.y, 0, h - 1);
		if (msp != msp_raw) msd.fill(false);
	}
	void update_hover() {
		double d = 0;
		Hoverable *cur = NULL;
		for (auto h : Hoverables) {
			if (h->TestHover(d)) {
				if (cur) cur->SetHovered(false);
				cur = h;
			}
			else h->SetHovered(false);
		}
		if (cur) cur->SetHovered(true);
		Hoverables.clear();
	}
	void present_scr() {
		HDC hdc = GetDC(hwnd);
		SetDIBitsToDevice(hdc, 0, 0, w, h, 0, 0, 0, h, scr.cols.data(), &bmi, DIB_RGB_COLORS);
		ReleaseDC(hwnd, hdc);
	}
	void render() {
		scr.fill(bgc);
		stable_sort(Renderables.begin(), Renderables.end(), [](Renderable *a, Renderable *b) { return a->GetDepth() < b->GetDepth(); });
		for (auto r : Renderables) r->Render();
		Renderables.clear();
	}
	void show_cursor() {
		if (!use_sys_cursor) {
			tile &cursor = *Resources[user_cursor];
			draw_tile_scale(scr, msp - dvec2(w_cursor, h_cursor) / 2, w_cursor, h_cursor, scr.rect(), cursor, cursor.rect());
		}
	}
	void prep_next_frm() {
		rep(i, 0, 3) msd_old[i] = msd[i];
		rep(i, 0, 256) kbd_old[i] = kbd[i];
		msw = 0;
		chs.clear();
		moving = false;
	}

	virtual void Load() {
		if (loaded) return;

		loaded = true;
		set_locale();

		print_console(L"正在初始化窗口...");
		hinst = GetModuleHandle(NULL);
		register_window_class();
		create_window();
		init_bmi();
		sys_cursor = LoadCursorW(NULL, IDC_ARROW);
		print_console(L"窗口初始化完成.", true);

		ft.load();
		if (!use_sys_cursor) { add_resource(user_cursor); }
	}
	virtual void Update() {}
	void Run(bool show_console = false, bool wait_enter = true) {
		if (wait_enter) {
			print_console(L"按[ENTER]键开始运行程序.", true);
			std::cin.ignore(INT_MAX, '\n');
		}
		if (!show_console) hide_console();

		ShowWindow(hwnd, SW_SHOW);
		tm = std::chrono::high_resolution_clock::now();
		MSG msg;
		while (true) {
			if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) break;
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else {
				update_msp();
				update_hover();
				calc_fps();

				Update();

				render();
				show_cursor();
				present_scr();
				prep_next_frm();
			}
		}
	}
};

struct RenderableCasual : Renderable {
	function<void()> f;
	double d = 0;

	RenderableCasual() = default;
	RenderableCasual(function<void()> f, double d = 0) : f(f), d(d) {}

	double GetDepth() const override { return d; }
	void Render() const override { f(); }
};

struct HoverableCasual : Hoverable {
	function<bool()> f;
	double depth = 0;
	bool hovered = false;

	HoverableCasual() = default;
	HoverableCasual(function<bool()> f, double d = 0) : f(f), depth(d) {}

	void SetHovered(bool h) override { hovered = h; };
	bool TestHover(double &d) const { return f()? d = depth, true : false; };
};