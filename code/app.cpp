#include "app.h"

LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

void App::register_window_class() {
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = wnd_proc;
	wc.hInstance = hinst;
	wc.lpszClassName = class_name.c_str();
	RegisterClassExW(&wc);
}

int created(App *&app, WPARAM wp, LPARAM lp) {
	auto pcs = (CREATESTRUCT *)lp;
	app = (App *)pcs->lpCreateParams;
	return 0;
}
int destroyed(App *app, WPARAM wp, LPARAM lp) {
	PostQuitMessage(0);
	return 0;
}
int mouse_wheel(App *app, WPARAM wp, LPARAM lp) {
	int dt = GET_WHEEL_DELTA_WPARAM(wp);
	app->msw = (double)dt / WHEEL_DELTA;
	return 0;
}
int left_mouse_down(App *app, WPARAM wp, LPARAM lp) {
	app->msd[0] = true;
	return 0;
}
int left_mouse_up(App *app, WPARAM wp, LPARAM lp) {
	app->msd[0] = false;
	return 0;
}
int mid_mouse_down(App *app, WPARAM wp, LPARAM lp) {
	app->msd[1] = true;
	return 0;
}
int mid_mouse_up(App *app, WPARAM wp, LPARAM lp) {
	app->msd[1] = false;
	return 0;
}
int right_mouse_down(App *app, WPARAM wp, LPARAM lp) {
	app->msd[2] = true;
	return 0;
}
int right_mouse_up(App *app, WPARAM wp, LPARAM lp) {
	app->msd[2] = false;
	return 0;
}
int mouse_move(App *app, WPARAM wp, LPARAM lp) {
	return 0;
}
int key_down(App *app, WPARAM wp, LPARAM lp) {
	app->kbd[wp] = true;
	return 0;
}
int key_up(App *app, WPARAM wp, LPARAM lp) {
	app->kbd[wp] = false;
	return 0;
}
int kill_focus(App *app, WPARAM wp, LPARAM lp) {
	app->kbd.fill(false);
	return 0;
}
int char_translated(App *app, WPARAM wp, LPARAM lp) {
	app->chs += (wchar_t)wp;
	return 0;
}
int set_cursor(App *app, WPARAM wp, LPARAM lp) {
	SetCursor(LOWORD(lp) != HTCLIENT || app->use_sys_cursor ? app->sys_cursor : NULL);
	return 0;
}

int moving(App *app, WPARAM wp, LPARAM lp) {
	app->moving = true;
	return 0;
}

LRESULT CALLBACK wnd_proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
	static App *app = NULL;
	switch (msg) {
	case WM_DESTROY: return destroyed(app, wp, lp);
	case WM_CREATE: return created(app, wp, lp);
	case WM_MOUSEWHEEL: return mouse_wheel(app, wp, lp);
	case WM_LBUTTONDOWN: return left_mouse_down(app, wp, lp);
	case WM_LBUTTONUP: return left_mouse_up(app, wp, lp);
	case WM_MBUTTONDOWN: return mid_mouse_down(app, wp, lp);
	case WM_MBUTTONUP: return mid_mouse_up(app, wp, lp);
	case WM_RBUTTONDOWN: return right_mouse_down(app, wp, lp);
	case WM_RBUTTONUP: return right_mouse_up(app, wp, lp);
	case WM_MOUSEMOVE: return mouse_move(app, wp, lp);
	case WM_KEYDOWN: return key_down(app, wp, lp);
	case WM_KEYUP: return key_up(app, wp, lp);
	case WM_KILLFOCUS: return kill_focus(app, wp, lp);
	case WM_CHAR: return char_translated(app, wp, lp);
	case WM_SETCURSOR: return set_cursor(app, wp, lp);
	case WM_MOVING: return moving(app, wp, lp);
	default: return DefWindowProc(wnd, msg, wp, lp);
	}
}