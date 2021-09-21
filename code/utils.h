#pragma once

#pragma region Include & Using
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <cstring>
#include <cctype>
#include <cwctype>
#include <cassert>

#include <vector>
#include <deque>
#include <map>
#include <string>
#include <list>
#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <iostream>
#include <deque>
#include <array>
#include <iterator>
#include <queue>
#include <set>

#define UNICODE
#define _UNICODE
#define NOMINMAX

#include <Windows.h>
#include <windowsx.h>

#undef UNICODE
#undef _UNICODE
#undef NOMINMAX

using std::shared_ptr;
using std::weak_ptr;
using std::function;
using std::vector;
using std::string;
using std::wstring;
using std::map;
using std::deque;
using std::list;
using std::array;
using std::priority_queue;

using std::next;
using std::max;
using std::min;
using std::move;
using std::to_string;
using std::to_wstring;
using std::remove;
using std::make_shared;
using std::sort;
using std::stable_sort;
#pragma endregion

double constexpr PI = 3.1415926535;
double constexpr EULER = 2.7182818284;

#define fwt(x) fwrite(&(x), sizeof(x), 1, f);
#define frd(x) fread(&(x), sizeof(x), 1, f);
#define fwts(x, n) fwrite(x, sizeof(*(x)), n, f);
#define frds(x, n) fread(x, sizeof(*(x)), n, f);

#define rep(x, a, b) for (int x = (a), _n = int(b); x < _n; ++x)

template<class T> T *&del(T *&x) { if (x) { delete x; x = NULL; } return x; }
template<class T> T *&dels(T *&x) { if (x) { delete[] x; x = NULL; } return x; }

template<class T> bool in_range(T const &v, T const &min, T const &max) { return v >= min && v <= max; }
template<class T> T const &clamp(T const &v, T const &_min, T const &_max) { return min(_max, max(_min, v)); }

inline int drand(int a, int b) {
	static std::random_device rd;
	static std::mt19937 generator(rd());
	std::uniform_int_distribution<int> d(a, b);
	return d(generator);
}
inline double frand(double a = 0, double b = 1) {
	static std::random_device rd;
	static std::mt19937 generator(rd());
	std::uniform_real_distribution<double> d(a, b);
	return d(generator);
}

inline wstring to_wstring2(double d) {
	wchar_t tmp[64];
	swprintf_s(tmp, L"%.2lf", d);
	return tmp;
}
inline wstring to_wstring(string const &s) {
	wchar_t *tmp = new wchar_t[s.size() + 1];
	swprintf_s(tmp, s.size() + 1, L"%S", s.c_str());
	wstring ret = tmp;
	dels(tmp);
	return ret;
}
inline string to_string(wstring const &s) {
	char *tmp = new char[s.size() * 2 + 1];
	sprintf_s(tmp, s.size() * 2 + 1, "%ls", s.c_str());
	string ret = tmp;
	dels(tmp);
	return ret;
}

inline double atof(string const &s) { return atof(s.c_str()); }
inline double wtof(wstring const &s) { return atof(to_string(s)); }

inline FILE *wfopen(wstring const &name, char const *arg) {
	FILE *f = NULL;
	fopen_s(&f, to_string(name).c_str(), arg);
	return f;
}
inline bool file_exist(wstring const &name) {
	FILE *f = wfopen(name, "rb");
	return f ? fclose(f), true : false;
}

inline void set_locale() { setlocale(LC_ALL, "zh-CN"); }
inline void show_console() { ShowWindow(GetConsoleWindow(), SW_SHOW); }
inline void hide_console() { ShowWindow(GetConsoleWindow(), SW_HIDE); }
inline void hide_console_cursor() {
	HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;

	GetConsoleCursorInfo(hdl, &info);
	info.bVisible = false;

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

inline void print_console(wstring const &s, bool newline = false, int width = 80) {
	hide_console_cursor();

	auto hdl = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hdl, &info);
	SetConsoleCursorPosition(hdl, { 0, info.dwCursorPosition.Y });

	wprintf(s.c_str());

	GetConsoleScreenBufferInfo(hdl, &info);
	rep(i, info.dwCursorPosition.X, width) { printf(" "); }

	if (newline) printf("\n");
}