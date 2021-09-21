#include "app.h"
#include "button.h"
#include "dragbar.h"
#include "textbox.h"
#include "label.h"
#include "droplist.h"
#include "checkbox.h"
#include "info.h"
#include "msg_manager.h"

int constexpr DEADMODE_ALIVE = 0;
int constexpr DEADMODE_ALMOST_DEAD = 1;
int constexpr DEADMODE_DEAD = 2;

enum build_plan { PLAN_SHIPYARD, PLAN_TURRENT, PLAN_GENERATOR };

double constexpr depth_shipyard = 0.2;
double constexpr depth_turrent = 0.2;
double constexpr depth_ship = 0.3;
double constexpr depth_bullet = 0.1;
double constexpr depth_cmd_center = 0.2;
double constexpr depth_generator = 0.2;

double constexpr small_angle = 0.01;

double constexpr ship_target_rr = 120.0 * 120;
double constexpr ship_close_rr = 80.0 * 80;
double constexpr ship_fire_r = 140;

double constexpr turrent_target_rr = 150.0 * 150;
double constexpr turrent_fire_r = 200;

struct SpaceBattle;

struct Object {
	virtual void Update() {}

	virtual int Faction() const { return 0; }
	virtual int DeadMode() const { return DEADMODE_ALIVE; }
	virtual vec2 Pos() const { return {}; }
	virtual vec2 Velocity() const { return {}; }
	virtual double GetHealth() const { return 0; }
	virtual void SetHealth(double h) {}

	virtual ~Object() {}
};

struct Shipyard : Renderable, Hoverable, Object {
	SpaceBattle *sb = NULL;
	int faction = 0;
	dvec2 pos_grid;
	int dead_mode = DEADMODE_ALIVE;

	bool highlight = false;
	bool highlight_target = false;
	bool hovered = false;
	double health = 8;
	double max_next_ship = 10;
	double next_ship = max_next_ship;
	

	Shipyard() = default;
	Shipyard(SpaceBattle *sb, int f, dvec2 const &pg) : sb(sb), faction(f), pos_grid(pg) {}

	double GetDepth() const override { return highlight || highlight_target ? depth_shipyard + 0.05 : depth_shipyard; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override;
	double GetHealth() const override { return health; }
	void SetHealth(double h) override { health = h; }

	void Update() override;
	void Render() const override;
};

struct Turrent : Renderable, Hoverable, Object {
	SpaceBattle *sb = NULL;
	int faction = 0;
	dvec2 pos_grid;
	int dead_mode = DEADMODE_ALIVE;
	Object *target = NULL;

	bool highlight = false;
	bool highlight_target = false;
	bool hovered = false;
	double health = 4;
	double next_fire = 0;
	double angle = 0;

	Turrent() = default;
	Turrent(SpaceBattle *sb, int f, dvec2 const &pg) : sb(sb), faction(f), pos_grid(pg) {}

	double GetDepth() const override { return highlight || highlight_target ? depth_turrent + 0.05 : depth_turrent; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override;
	double GetHealth() const override { return health; }
	void SetHealth(double h) override { health = h; }

	void Update() override;
	void Render() const override;
};

struct Ship : Renderable, Hoverable, Object {
	SpaceBattle *sb = NULL;
	int faction = 0;
	vec2 pos;
	vec2 v;
	int dead_mode = DEADMODE_ALIVE;
	Object *target = NULL;

	bool highlight = false;
	bool highlight_target = false;
	bool hovered = false;
	double health = 2;
	double next_fire = 0;
	double angle = 0;

	Ship() = default;
	Ship(SpaceBattle *sb, int f, vec2 const &p, vec2 const &v = vec2()) : sb(sb), faction(f), pos(p), v(v) {}

	double GetDepth() const override { return highlight || highlight_target ? depth_ship + 0.05 : depth_ship; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override { return pos; };
	vec2 Velocity() const override { return v; }
	double GetHealth() const override { return health; }
	void SetHealth(double h) override { health = h; }

	void Update() override;
	void Render() const override;
};

struct Bullet : Renderable, Hoverable, Object {
	SpaceBattle *sb = NULL;
	int faction = 0;
	vec2 pos;
	vec2 v;
	int dead_mode = DEADMODE_ALIVE;
	
	bool highlight = false;
	bool hovered = false;
	double time_left = 0.6;

	Bullet() = default;
	Bullet(SpaceBattle *sb, int f, vec2 const &p, vec2 const &v) : sb(sb), faction(f), pos(p), v(v) {}

	double GetDepth() const override { return highlight ? depth_bullet + 0.05 : depth_bullet; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override { return pos; };
	vec2 Velocity() const override { return v; }

	void Update() override;
	void Render() const override;
};

struct Generator : Object, Renderable, Hoverable {
	SpaceBattle *sb = NULL;
	int faction = 0;
	dvec2 pos_grid;
	int dead_mode = DEADMODE_ALIVE;

	bool highlight = false;
	bool highlight_target = false;
	bool hovered = false;
	double health = 10;
	
	Generator() = default;
	Generator(SpaceBattle *sb, int f, dvec2 const &pg) : sb(sb), faction(f), pos_grid(pg) {}

	double GetDepth() const override { return highlight || highlight_target ? depth_generator + 0.05 : depth_generator; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override;
	double GetHealth() const override { return health; }
	void SetHealth(double h) override {
		health = h;
	}

	void Update() override;
	void Render() const override;
};

struct CmdCenter : Object, Renderable, Hoverable {
	SpaceBattle *sb = NULL;
	int faction = 0;
	dvec2 pos_grid;
	int dead_mode = DEADMODE_ALIVE;

	bool highlight = false;
	bool highlight_target = false;
	bool hovered = false;
	double health = 40;
	build_plan plan = PLAN_SHIPYARD;


	CmdCenter() = default;
	CmdCenter(SpaceBattle *sb, int f, dvec2 const &pg) : sb(sb), faction(f), pos_grid(pg), plan((build_plan)drand(0, 2)) {}

	double GetDepth() const override { return highlight || highlight_target ? depth_cmd_center + 0.05 : depth_cmd_center; }
	void SetHovered(bool h) override { hovered = h; }
	bool TestHover(double &d) const override;
	int Faction() const override { return faction; }
	int DeadMode() const override { return dead_mode; }
	vec2 Pos() const override;
	double GetHealth() const override { return health; }
	void SetHealth(double h) override { health = h; }

	void Update() override;
	void Render() const override;
};

struct SpaceBattle : App {
	SpaceBattle() : App(L"太空战斗模拟器", 1440, 720, { 160, 160, 140 }) {}

	enum _mode { OBSERVE_IDLE, OBSERVE_SHIPYARD, OBSERVE_TURRENT, OBSERVE_SHIP, OBSERVE_BULLET, OBSERVE_GENERATOR, OBSERVE_CMD_CENTER, CREATE_SHIPYARD, CREATE_TURRENT, CREATE_CMD_CENTER, CREATE_SHIP, CREATE_GENERATOR } mode = OBSERVE_IDLE;
	wstring mode_txt(_mode m) const {
		switch (m) {
		case OBSERVE_IDLE: return L"观察未知";
		case OBSERVE_SHIPYARD: return L"观察船坞";
		case OBSERVE_TURRENT: return L"观察炮塔";
		case OBSERVE_SHIP: return L"观察飞船";
		case OBSERVE_BULLET: return L"观察子弹";
		case OBSERVE_CMD_CENTER: return L"观察指挥中心";
		case OBSERVE_GENERATOR: return L"观察反应堆";
		case CREATE_SHIPYARD: return L"创建船坞";
		case CREATE_TURRENT: return L"创建炮塔";
		case CREATE_SHIP: return L"创建飞船";
		case CREATE_CMD_CENTER: return L"创建指挥中心";
		case CREATE_GENERATOR: return L"创建反应堆";
		}
	}

	wstring plan_txt(build_plan m) const {
		switch (m) {
		case PLAN_SHIPYARD: return L"船坞";
		case PLAN_TURRENT: return L"炮塔";
		case PLAN_GENERATOR: return L"反应堆";
		}
	}

	double plan_energy(build_plan m) const {
		switch (m) {
		case PLAN_SHIPYARD: return 50;
		case PLAN_TURRENT: return 10;
		case PLAN_GENERATOR: return 30;
		}
	}

	Shipyard *shipyard_observed = NULL;
	Turrent *turrent_observed = NULL;
	Ship *ship_observed = NULL;
	Bullet *bullet_observed = NULL;
	CmdCenter *cmd_center_observed = NULL;
	Generator *generator_observed = NULL;
	Object *target_observed = NULL;

	vector<shared_ptr<Object>> new_damageables;
	vector<shared_ptr<Object>> damageables;
	vector<shared_ptr<Object>> new_others;
	vector<shared_ptr<Object>> others;

	int gap = 10;
	int x_film = 160;
	int x_setting = 1080;
	int h_info = 40;
	int h_txt = 20;
	int w_menu_bt() const { return x_film - 2 * gap; }
	int y_info_txt() const { return h - h_info + h_txt / 2; }
	int x_setting_ctrl() const { return x_setting + gap; }
	drect vp_setting() const { return { { x_setting_ctrl(), 0 }, w - x_setting - 20, h - h_info }; }
	drect vp_film() const { return { { x_film, 0 }, x_setting - x_film, h - h_info }; }
	dvec2 ct_film() const { return dvec2(x_film, 0) + dvec2(x_setting - x_film, h - h_info) / 2; }
	tile film_bgr = tile(vp_film().w, vp_film().h);

	int h_build_s = 4;
	int build_s() const { return 2 * h_build_s + 1; }
	double angle = 0;
	double ini_eye_dist = 2.5;
	double eye_dist = ini_eye_dist;
	double min_eye_dist = 0.1, max_eye_dist = 10;
	mat2 tsf, inv_tsf;
	vec2 cam_pos;

	int n_faction = 6;
	vector<dcol3> faction_colors = {
		{ 255, 0, 0 },
		{ 0, 128, 255 },
		{ 255, 255, 0 },
		{ 255, 0, 255 },
		{ 200, 200, 200 },
		{ 0, 255, 0 },
	};
	vector<wstring> faction_names = {
		L"银河公社",
		L"监护委员会",
		L"星海集团",
		L"第二帝国",
		L"一般联盟",
		L"超空间僵尸",
	};
	vector<wstring> faction_descriptions = {
		L"银河公社的描述: 没有简介.",
		L"监护委员会的描述: 监护委员会致力于捍卫星系内的传统与秩序.",
		L"星海集团的描述: 恒星际企业, 前身是一家互联网公司.",
		L"第二帝国的描述: 在第一帝国覆灭后, 人们建立了第二帝国.",
		L"一般联盟的描述: 一个政治实体, 在各个方面都表现的非常一般.",
		L"超空间僵尸的描述: 在经典僵尸的基础上增加了超空间功能.",
	};
	vector<double> faction_energy = vector<double>(n_faction);
	vector<int> faction_alive = vector<int>(n_faction);
	vector<int> faction_alive_old = vector<int>(n_faction);

	int nx_grid = 60, ny_grid = 40;
	double s_grid = 40;
	vector<int> grid = vector<int>((size_t)nx_grid * ny_grid);

	vec2 tl_grid() const { return -vec2(nx_grid, ny_grid) * s_grid / 2; }
	vec2 pos_grid(dvec2 const &v) const {
		return tl_grid() + (vec2(v) + vec2(0.5)) * s_grid;
	}
	bool grid_chosen(dvec2 &v) const {
		vec2 p = inv_tsf * vec2(msp - ct_film()) + cam_pos - tl_grid();
		p /= s_grid;
		v = dvec2((int)floor(p.x), (int)floor(p.y));
		return inside(msp, vp_film()) && inside(v, { nx_grid, ny_grid });
	}

	vec2 send(vec2 const &p) const {
		return tsf * (p - cam_pos) + (vec2)ct_film();
	}
	vec2 send_back(vec2 const &p) const {
		return inv_tsf * (p - (vec2)ct_film()) + cam_pos;
	}

	RenderableCasual rc_grid = RenderableCasual([this]() {
		draw_grid(scr, (vec2)ct_film() + tsf * -cam_pos, s_grid, nx_grid, ny_grid, 4, tsf, vp_film(), dcol3(100));
		}, -0.9);

	HoverableCasual hc_film = HoverableCasual([this]() { return inside(msp, vp_film()); }, -1);

	MsgManager msg = MsgManager(this, { x_film, h - h_info }, x_setting - x_film, 10);

	bool paused = false;
	bool follow_ship = false;
	bool show_grid = false;
	bool follow_ship_angle = false;

	wstring about_txt = 
		L"用 [方向键], [WASD] 或者按住鼠标中键来移动摄像机的位置. 用 [QE] 或者按住鼠标右键来旋转画面. 用 \"启动 / 暂停\" 按钮或者 [空格键] 来启动或暂停程序. 用鼠标滚轮来放缩画面. 在观察模式下用鼠标左键点击物体可以查看它的详细信息."
		L"\n\n这个程序只用了 CPU 渲染画面, 所以绘制图形对帧率的影响是很大的, 最好不要在视野较远的时候打开 \"显示网格\" 的选项."
		L"\n\n我写这个程序主要是为了复刻一个 Kongregate 上的一款 Flash 小游戏, 它的名字叫 Obliterate Everything 2, 在 4399 里面它被叫做 \"太空防卫战 2 正式版\". Obliterate Everything 3 很早就在 Kongregate 上架了, 现在它的第 4 部正在开发当中. 虽然 Flash 游戏早就过时了, 而且这个游戏的内容也不是非常丰富, 但我觉得它还是有那么一点意思. \n\n程序是用 C++ 编写的, 用到了 C++ 标准库和微软的 Windows API 库, 其中并没有参考 (我也没有去找) Obliterate Everything 2 的源代码. 在未来我准备增添飞船和炮塔的种类, 增加新的武器和伤害效果. 在图形上我准备用手绘的贴图, 而非现在使用的简单几何图形来渲染船只和结构. 也许未来我还会把它发布到 itch.io 上. (2021/09/21)"

		
		L"\n\n下面的内容是用来测试滚动条用的: \n\nI'd look right up at night.\nAnd all I'd see was darkness.\nNow I see the stars alright.\nI wanna reach right up and grab one for you.\nWhen the lights went down in your house.\nYeah that made me happy.\nThe sweat I make for you.\nYeah..\nI think you know where that comes from.";
	//RenderableCasual rc_about = RenderableCasual([this]() {
	//	draw_rect_framed(scr, dvec2((w - 600) / 2 - 10, 90), 620, 500, scr.rect(), { 160, 160, 160 }, 255, dcol3(50), 1);
	//	draw_paragraph(scr, 600, ft, about_txt, dvec2((w - 600) / 2, 100), h_txt, scr.rect());
	//	}, 1.5);

	void replay() {
		damageables.clear();
		new_damageables.clear();
		others.clear();
		new_others.clear();
		mode = OBSERVE_IDLE;
		pn_setting.SetCur(NULL);
		sc_setting.SetCl(NULL);
		cam_pos = {};
		rep(i, 0, grid.size()) grid[i] = 0;
		for (auto &e : faction_energy) e = 0;
		eye_dist = ini_eye_dist;
		angle = 0;

		rep(f, 0, n_faction) {
			int i = drand(h_build_s, nx_grid - h_build_s - 1);
			int j = drand(h_build_s, ny_grid - h_build_s - 1);
			new_damageables.push_back(make_shared<CmdCenter>(this, f, dvec2{ i, j }));
			int id = j * nx_grid + i;
			grid[id] = 1;
		}
	}
	void pause() {
		if (paused) {
			msg.AddMsg(L"你启动了游戏.", dcol3(255));
			paused = false;
		}
		else {
			msg.AddMsg(L"你暂停了游戏.", dcol3(255));
			paused = true;
		}
	}

	int w_float = 600;
	int h_float = 400;
	dvec2 tl_float() const { return dvec2((w - w_float) / 2, (h - h_float) / 2); }
	int x_float_control() const { return tl_float().x + gap; }
	drect vp_float() const { return { tl_float(), w_float, h_float }; }

	
#pragma region UI
	Scrollbar sb_float = Scrollbar(this, NULL, {}, 0);
	PnTmp pn_float = PnTmp(this, &sb_float, tl_float(), w_float, h_float);
	LbRichVar lb_about = LbRichVar(this, about_txt, { x_float_control(), 0 }, w_float - 2 * gap - sb_float.w, vp_float());
	CtrlListY cl_about = CtrlListY(this, { &lb_about }, tl_float().y + gap, vp_float());


	BtCasual bt_start = BtCasual(this, [this]() { 
		pause();
		}, L"", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_edit_to_menu = BtCasual(this, [this]() {
		pn_menu.SetCur(&cl_menu);
		}, L"回到主菜单", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_edit = BtCasual(this, [this]() {
		pn_menu.SetCur(&cl_edit);
		}, L"编辑场景", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_show_grid = BtCasual(this, [this]() { show_grid = !show_grid; }, L"", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_create_shipyard = BtCasual(this, [this]() { mode = CREATE_SHIPYARD; pn_setting.SetCur(&cl_create_shipyard); sc_setting.SetCl(&cl_create_shipyard); }, L"创建船坞", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_create_turrent = BtCasual(this, [this]() { mode = CREATE_TURRENT; pn_setting.SetCur(&cl_create_turrent); sc_setting.SetCl(&cl_create_turrent); }, L"创建炮塔", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_create_cmd_center = BtCasual(this, [this]() { mode = CREATE_CMD_CENTER; pn_setting.SetCur(&cl_create_cmd_center); sc_setting.SetCl(&cl_create_cmd_center); }, L"创建指挥中心", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_create_generator = BtCasual(this, [this]() { mode = CREATE_GENERATOR; pn_setting.SetCur(&cl_create_generator); sc_setting.SetCl(&cl_create_generator); }, L"创建反应堆", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_create_ship = BtCasual(this, [this]() { mode = CREATE_SHIP; pn_setting.SetCur(&cl_create_ship); sc_setting.SetCl(&cl_create_ship); }, L"创建飞船", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_observe = BtCasual(this, [this]() { mode = OBSERVE_IDLE; pn_setting.SetCur(NULL); sc_setting.SetCl(NULL); }, L"观察", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_clear = BtCasual(this, [this]() { 
		damageables.clear(); 
		new_damageables.clear();
		others.clear();
		new_others.clear();
		mode = OBSERVE_IDLE;
		pn_setting.SetCur(NULL);
		sc_setting.SetCl(NULL);
		cam_pos = {};
		rep(i, 0, grid.size()) grid[i] = 0;
		for (auto &e : faction_energy) e = 0;
		eye_dist = ini_eye_dist;
		angle = 0;
		msg.AddMsg(L"你清空了场景.", dcol3(255));
		}, L"清除", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_test = BtCasual(this, [this]() {
		replay();
		msg.AddMsg(L"你重新开始了游戏.", dcol3(255));
		}, L"重新开始", { gap, 0 }, w_menu_bt(), scr.rect());
	BtCasual bt_about = BtCasual(this, [this]() {
		pn_float.SetCur(&cl_about);
		}, L"关于", { gap, 0 }, w_menu_bt(), scr.rect());
	CtrlListY cl_edit = CtrlListY(this, { &bt_edit_to_menu, &bt_create_shipyard, &bt_create_turrent, &bt_create_cmd_center, &bt_create_generator, &bt_create_ship, &bt_clear }, gap, scr.rect());
	CtrlListY cl_menu = CtrlListY(this, { &bt_start, &bt_show_grid, &bt_edit, &bt_observe, &bt_test, &bt_about }, gap, scr.rect());
	Panel pn_menu = Panel(this, &cl_menu);

	Label lb_create_shipyard = Label(this, L"设置: 创建船坞", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_create_turrent = Label(this, L"设置: 创建炮塔", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_create_ship = Label(this, L"设置: 创建飞船", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_create_cmd_center = Label(this, L"设置: 创建指挥中心", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_create_generator = Label(this, L"设置: 创建反应堆", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_regime = Label(this, L"势力: ", { x_setting_ctrl(), 0 }, 0, vp_setting());
	Droplist dl_regime = Droplist(this, faction_names, { x_setting_ctrl() + 60, 0 }, 160, vp_setting());
	IfCasual if_regime = IfCasual(this, [this]() { if_regime.info = faction_descriptions[dl_regime.picked]; }, { x_setting_ctrl() + 240, 0 }, 300, vp_setting());
	CtrlGroup cg_regime = CtrlGroup(this, { &lb_regime, &dl_regime, &if_regime });
	CtrlListY cl_create_shipyard = CtrlListY(this, { &lb_create_shipyard, &cg_regime }, gap, vp_setting());
	CtrlListY cl_create_turrent = CtrlListY(this, { &lb_create_turrent, &cg_regime }, gap, vp_setting());
	CtrlListY cl_create_ship = CtrlListY(this, { &lb_create_ship, &cg_regime }, gap, vp_setting());
	CtrlListY cl_create_cmd_center = CtrlListY(this, { &lb_create_cmd_center, &cg_regime }, gap, vp_setting());
	CtrlListY cl_create_generator = CtrlListY(this, { &lb_create_generator, &cg_regime }, gap, vp_setting());
	Label lb_observe_shipyard = Label(this, L"观察: 船坞", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_shipyard_regime = LbCasual(this, [this]() { 
		lb_shipyard_regime.txt = L"势力: " + faction_names[shipyard_observed->faction]; 
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_shipyard_regime = IfCasual(this, [this]() { if_shipyard_regime.info = faction_descriptions[shipyard_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_shipyard_regime = CtrlGroup(this, { &lb_shipyard_regime, &if_shipyard_regime }, 0, 20);
	LbCasual lb_shipyard_pos = LbCasual(this, [this]() {
		lb_shipyard_pos.txt = L"格子: (" + to_wstring(shipyard_observed->pos_grid.x) + L", " + to_wstring(shipyard_observed->pos_grid.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_shipyard_health = LbCasual(this, [this]() {
		lb_shipyard_health.txt = L"结构: " + to_wstring2(shipyard_observed->health);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_shipyard_next_ship = LbCasual(this, [this]() {
		lb_shipyard_next_ship.txt = L"剩余造船时间: " + to_wstring2(shipyard_observed->next_ship);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CtrlListY cl_observe_shipyard = CtrlListY(this, { 
		&lb_observe_shipyard, &cg_shipyard_regime, &lb_shipyard_pos, &lb_shipyard_health, &lb_shipyard_next_ship
		}, gap, vp_setting());
	Label lb_observe_ship = Label(this, L"观察: 飞船", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_ship_regime = LbCasual(this, [this]() {
		lb_ship_regime.txt = L"势力: " + faction_names[ship_observed->faction];
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_ship_regime = IfCasual(this, [this]() { if_ship_regime.info = faction_descriptions[ship_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_ship_regime = CtrlGroup(this, { &lb_ship_regime, &if_ship_regime }, 0, 20);
	LbCasual lb_ship_pos = LbCasual(this, [this]() {
		lb_ship_pos.txt = L"位置: (" + to_wstring2(ship_observed->pos.x) + L", " + to_wstring2(ship_observed->pos.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_ship_v = LbCasual(this, [this]() {
		lb_ship_v.txt = L"速度: (" + to_wstring2(ship_observed->v.x) + L", " + to_wstring2(ship_observed->v.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_ship_angle = LbCasual(this, [this]() {
		lb_ship_angle.txt = L"角度: " + to_wstring2(ship_observed->angle);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_ship_health = LbCasual(this, [this]() {
		lb_ship_health.txt = L"结构: " + to_wstring2(ship_observed->health);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_ship_next_fire = LbCasual(this, [this]() {
		lb_ship_next_fire.txt = L"剩余开火时间: " + to_wstring2(ship_observed->next_fire);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	Label lb_follow_ship = Label(this, L"跟随船只: ", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CbBool cb_follow_ship = CbBool(this, &follow_ship, { x_setting_ctrl() + 100, 0 }, vp_setting(), 20);
	CtrlGroup cg_follow_ship = CtrlGroup(this, { &lb_follow_ship, &cb_follow_ship }, 0, 20);
	Label lb_follow_ship_angle = Label(this, L"保持船只角度: ", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CbBool cb_follow_ship_angle = CbBool(this, &follow_ship_angle, { x_setting_ctrl() + 140, 0 }, vp_setting(), 20);
	CtrlGroup cg_follow_ship_angle = CtrlGroup(this, { &lb_follow_ship_angle, &cb_follow_ship_angle }, 0, 20);
	CtrlListY cl_observe_ship = CtrlListY(this, {
		&lb_observe_ship, &cg_follow_ship, &cg_follow_ship_angle, &cg_ship_regime, &lb_ship_pos, &lb_ship_v, &lb_ship_angle, &lb_ship_health, &lb_ship_next_fire
		}, gap, vp_setting());
	Label lb_observe_turrent = Label(this, L"观察: 炮塔", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_turrent_regime = LbCasual(this, [this]() {
		lb_turrent_regime.txt = L"势力: " + faction_names[turrent_observed->faction];
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_turrent_regime = IfCasual(this, [this]() { if_turrent_regime.info = faction_descriptions[turrent_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_turrent_regime = CtrlGroup(this, { &lb_turrent_regime, &if_turrent_regime }, 0, 20);
	LbCasual lb_turrent_pos = LbCasual(this, [this]() {
		lb_turrent_pos.txt = L"格子: (" + to_wstring(turrent_observed->pos_grid.x) + L", " + to_wstring(turrent_observed->pos_grid.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_turrent_angle = LbCasual(this, [this]() {
		lb_turrent_angle.txt = L"角度: " + to_wstring2(turrent_observed->angle);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_turrent_health = LbCasual(this, [this]() {
		lb_turrent_health.txt = L"结构: " + to_wstring2(turrent_observed->health);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_turrent_next_fire = LbCasual(this, [this]() {
		lb_turrent_next_fire.txt = L"剩余开火时间: " + to_wstring2(turrent_observed->next_fire);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CtrlListY cl_observe_turrent = CtrlListY(this, {
		&lb_observe_turrent, &cg_turrent_regime, &lb_turrent_pos, &lb_turrent_angle, &lb_turrent_health, &lb_turrent_next_fire
		}, gap, vp_setting());
	Label lb_observe_bullet = Label(this, L"观察: 子弹", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_bullet_regime = LbCasual(this, [this]() {
		lb_bullet_regime.txt = L"势力: " + faction_names[bullet_observed->faction];
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_bullet_regime = IfCasual(this, [this]() { if_bullet_regime.info = faction_descriptions[bullet_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_bullet_regime = CtrlGroup(this, { &lb_bullet_regime, &if_bullet_regime }, 0, 20);
	LbCasual lb_bullet_pos = LbCasual(this, [this]() {
		lb_bullet_pos.txt = L"位置: (" + to_wstring2(bullet_observed->pos.x) + L", " + to_wstring2(bullet_observed->pos.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_bullet_v = LbCasual(this, [this]() {
		lb_bullet_v.txt = L"速度: (" + to_wstring2(bullet_observed->v.x) + L", " + to_wstring2(bullet_observed->v.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_bullet_time_left = LbCasual(this, [this]() {
		lb_bullet_time_left.txt = L"剩余存活时间: " + to_wstring2(bullet_observed->time_left);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CtrlListY cl_observe_bullet = CtrlListY(this, {
		&lb_observe_bullet, &cg_bullet_regime, &lb_bullet_pos, &lb_bullet_v, &lb_bullet_time_left
		}, gap, vp_setting());
	Label lb_observe_cmd_center = Label(this, L"观察: 指挥中心", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_cmd_center_regime = LbCasual(this, [this]() {
		lb_cmd_center_regime.txt = L"势力: " + faction_names[cmd_center_observed->faction];
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_cmd_center_regime = IfCasual(this, [this]() { if_cmd_center_regime.info = faction_descriptions[cmd_center_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_cmd_center_regime = CtrlGroup(this, { &lb_cmd_center_regime, &if_cmd_center_regime }, 0, 20);
	LbCasual lb_cmd_center_pos = LbCasual(this, [this]() {
		lb_cmd_center_pos.txt = L"格子: (" + to_wstring(cmd_center_observed->pos_grid.x) + L", " + to_wstring(cmd_center_observed->pos_grid.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_cmd_center_health = LbCasual(this, [this]() {
		lb_cmd_center_health.txt = L"结构: " + to_wstring2(cmd_center_observed->health);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_cmd_center_energy = LbCasual(this, [this]() {
		lb_cmd_center_energy.txt = L"能量: " + to_wstring2(faction_energy[cmd_center_observed->faction]);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_cmd_center_plan = LbCasual(this, [this]() {
		lb_cmd_center_plan.txt = L"计划建造: " + plan_txt(cmd_center_observed->plan);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CtrlListY cl_observe_cmd_center = CtrlListY(this, {
		&lb_observe_cmd_center, &cg_cmd_center_regime, &lb_cmd_center_pos, &lb_cmd_center_health, &lb_cmd_center_energy, &lb_cmd_center_plan
		}, gap, vp_setting());
	Label lb_observe_generator = Label(this, L"观察: 反应堆", { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_generator_regime = LbCasual(this, [this]() {
		lb_generator_regime.txt = L"势力: " + faction_names[generator_observed->faction];
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	IfCasual if_generator_regime = IfCasual(this, [this]() { if_generator_regime.info = faction_descriptions[generator_observed->faction]; }, { x_setting_ctrl() + 180, 0 }, 300, vp_setting(), 20);
	CtrlGroup cg_generator_regime = CtrlGroup(this, { &lb_generator_regime, &if_generator_regime }, 0, 20);
	LbCasual lb_generator_pos = LbCasual(this, [this]() {
		lb_generator_pos.txt = L"格子: (" + to_wstring(generator_observed->pos_grid.x) + L", " + to_wstring(generator_observed->pos_grid.y) + L")";
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	LbCasual lb_generator_health = LbCasual(this, [this]() {
		lb_generator_health.txt = L"结构: " + to_wstring2(generator_observed->health);
		}, { x_setting_ctrl(), 0 }, 0, vp_setting(), Label::LEFT, {}, 20);
	CtrlListY cl_observe_generator = CtrlListY(this, {
		&lb_observe_generator, &cg_generator_regime, &lb_generator_pos, &lb_generator_health, 
		}, gap, vp_setting());


	Scrollbar sc_setting = Scrollbar(this, NULL, { w - 20, 0 }, h - h_info);
	Panel pn_setting = Panel(this, NULL);

	RenderableCasual rc_background = RenderableCasual([this]() {
		draw_tile_raw(scr, { x_film, 0 }, scr.rect(), film_bgr, film_bgr.rect());
		draw_rect(scr, { 0, h - h_info }, w, h_info, scr.rect(), { 80, 80, 160 }, 255);
		draw_string(scr, ft, fps_txt, { gap, y_info_txt() }, h_txt, scr.rect());
		draw_string(scr, ft, L"模式: " + mode_txt(mode), { 360, y_info_txt() }, h_txt, scr.rect());
		}, -1);
#pragma endregion

	void Load() override {
		App::Load();
		rep(i, 0, film_bgr.n()) {
			film_bgr.cols[i] = { 20, 40, 80 };
			film_bgr.as[i] = 255;
		}
		replay();
	}

	void mouse_rotate_and_scale() {
		if (!inside(msp, vp_film()) || !inside(msp_old, vp_film())) return;

		eye_dist = clamp(eye_dist - msw * 0.1, min_eye_dist, max_eye_dist);
		vec2 v_ms = vec2(msp - ct_film());
		vec2 v_ms_old = vec2(msp_old - ct_film());
		if (v_ms != vec2() && v_ms_old != vec2() && msd[2]) {
			angle += atan2(v_ms.y, v_ms.x) - atan2(v_ms_old.y, v_ms_old.x);
		}
		if (msd[1]) {
			cam_pos -= inv_tsf * (v_ms - v_ms_old);
		}
	}

	void Update() override {
		pn_menu.Update();
		pn_setting.Update();
		sc_setting.Update();
		Renderables.push_back(&rc_background);
		Hoverables.push_back(&hc_film);
		pn_float.Update();

		if (hc_film.hovered && msd[0] && !msd_old[0]) {
			switch (mode) {
			case SpaceBattle::OBSERVE_IDLE:
			case SpaceBattle::OBSERVE_SHIPYARD:
			case SpaceBattle::OBSERVE_TURRENT:
			case SpaceBattle::OBSERVE_SHIP:
			case SpaceBattle::OBSERVE_BULLET:
			case SpaceBattle::OBSERVE_CMD_CENTER:
			case SpaceBattle::OBSERVE_GENERATOR:
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
		}

		if(show_grid)
			Renderables.push_back(&rc_grid);

		bt_start.txt = paused ? L"启动" : L"暂停";
		bt_show_grid.txt = show_grid ? L"隐藏网格" : L"显示网格";

		msg.paused = paused;
		msg.Update();

		mouse_rotate_and_scale();
		tsf = mat2::rot(angle) / eye_dist;
		inv_tsf = tsf.inv();

		double v_move = 360;
		double v_angle = 1;
		if (kbd[VK_UP] || kbd['W']) {
			cam_pos += inv_tsf * vec2(0, -1) * v_move * dt;
		}
		if (kbd[VK_DOWN] || kbd['S']) {
			cam_pos += inv_tsf * vec2(0, 1) * v_move * dt;
		}
		if (kbd[VK_LEFT] || kbd['A']) {
			cam_pos += inv_tsf * vec2(-1, 0) * v_move * dt;
		}
		if (kbd[VK_RIGHT] || kbd['D']) {
			cam_pos += inv_tsf * vec2(1, 0) * v_move * dt;
		}
		if (kbd['Q']) {
			angle -= v_angle * dt;
		}
		if (kbd['E']) {
			angle += v_angle * dt;
		}
		if (kbd[VK_SPACE] && !kbd_old[VK_SPACE]) {
			pause();
		}

		for (auto d : new_damageables) {
			damageables.push_back(d);
		}
		new_damageables.clear();
		for (auto d : damageables) d->Update();
		damageables.erase(remove_if(damageables.begin(), damageables.end(), [](shared_ptr<Object> a) { return a->DeadMode() == 2; }), damageables.end());

		for (auto o : new_others) {
			others.push_back(o);
		}
		new_others.clear();
		for (auto o : others) o->Update();
		others.erase(remove_if(others.begin(), others.end(), [](shared_ptr<Object> a) { return a->DeadMode() == 2; }), others.end());

		bool faction_lost = false;
		faction_alive_old = faction_alive;
		rep(i, 0, n_faction) faction_alive[i] = 0;
		for (auto d : damageables) faction_alive[d->Faction()]++;

		rep(i, 0, n_faction) {
			if (faction_alive[i] == 0 && faction_alive_old[i] != 0) {
				msg.AddMsg(faction_names[i] + L"被击败了.", faction_colors[i]);
				faction_lost = true;
			}
		}
		if (faction_lost) {
			int tmp = 0;
			int f;
			rep(i, 0, n_faction) {
				if (faction_alive[i]) {
					tmp++;
					f = i;
				}
			}
			if (tmp == 1) {
				msg.AddMsg(faction_names[f] + L"赢得了战斗!", faction_colors[f]);
			}
			else if (tmp == 0) {
				msg.AddMsg(L"所有势力都失败了.", dcol3(255));
			}
		}

		target_observed = NULL;

		switch (mode) {
		case CREATE_SHIPYARD: 
		{
			dvec2 v;
			if (msd[0] && grid_chosen(v)) {
				int id = v.y * nx_grid + v.x;
				if (grid[id] == 0) {
					grid[id] = 1;
					damageables.push_back(make_shared<Shipyard>(this, dl_regime.picked, v));

					// int f = dl_regime.picked;
					// msg.AddMsg(L"你创建了一个属于" + faction_names[f] + L"的船坞.", faction_colors[f]);
				}
			}
		}
			break;
		case CREATE_TURRENT:
		{
			dvec2 v;
			if (msd[0] && grid_chosen(v)) {
				int id = v.y * nx_grid + v.x;
				if (grid[id] == 0) {
					grid[id] = 1;
					damageables.push_back(make_shared<Turrent>(this, dl_regime.picked, v));

					// int f = dl_regime.picked;
					// msg.AddMsg(L"你创建了一个属于" + faction_names[f] + L"的炮塔.", faction_colors[f]);
				}
			}
		}
		break;
		case CREATE_CMD_CENTER:
		{
			dvec2 v;
			if (msd[0] && grid_chosen(v)) {
				int id = v.y * nx_grid + v.x;
				if (grid[id] == 0) {
					grid[id] = 1;
					damageables.push_back(make_shared<CmdCenter>(this, dl_regime.picked, v));

					int f = dl_regime.picked;
					msg.AddMsg(L"你创建了一个属于" + faction_names[f] + L"的指挥中心.", faction_colors[f]);
				}
			}
		}
		break;
		case CREATE_GENERATOR:
		{
			dvec2 v;
			if (msd[0] && grid_chosen(v)) {
				int id = v.y * nx_grid + v.x;
				if (grid[id] == 0) {
					grid[id] = 1;
					damageables.push_back(make_shared<Generator>(this, dl_regime.picked, v));
				}
			}
		}
		break;
		case CREATE_SHIP:
		{
			if (msd[0] && !msd_old[0] && inside(msp, vp_film())) {
				damageables.push_back(make_shared<Ship>(this, dl_regime.picked, send_back((vec2)msp)));

				// int f = dl_regime.picked;
				// msg.AddMsg(L"你创建了一艘属于" + faction_names[f] + L"的飞船.", faction_colors[f]);
			}
		}
		break;
		case OBSERVE_SHIPYARD:
		{
			if (shipyard_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
		}
		break;
		case OBSERVE_TURRENT:
		{
			if (turrent_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
			else {
				if (turrent_observed->target != NULL && turrent_observed->target->DeadMode() == DEADMODE_ALIVE) {
					target_observed = turrent_observed->target;
				}
			}
		}
		break;
		case OBSERVE_SHIP:
		{
			if (ship_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
			else {
				if (ship_observed->target != NULL && ship_observed->target->DeadMode() == DEADMODE_ALIVE) {
					target_observed = ship_observed->target;
				}

				if (follow_ship) {
					cam_pos = ship_observed->pos;
					if (follow_ship_angle) {
						angle = -ship_observed->angle - PI / 2;
					}
					
				}
			}
		}
		break;
		case OBSERVE_CMD_CENTER:
		{
			if (cmd_center_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
			else {

			}
		}
		break;
		case OBSERVE_GENERATOR:
		{
			if (generator_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
			else {

			}
		}
		break;
		case OBSERVE_BULLET:
		{
			if (bullet_observed->DeadMode() != DEADMODE_ALIVE) {
				mode = OBSERVE_IDLE;
				pn_setting.SetCur(NULL);
				sc_setting.SetCl(NULL);
			}
		}
		break;
		}
	}
};

#pragma region Shipyard
vec2 Shipyard::Pos() const { return sb->pos_grid(pos_grid); }
bool Shipyard::TestHover(double &d) const {
	if (d > depth_shipyard) return false;
	dvec2 p;
	return sb->grid_chosen(p) && p == pos_grid ? d = depth_shipyard, true : false;
}

void Shipyard::Update() {
	if (dead_mode == 1) { 
		int id = pos_grid.y * sb->nx_grid + pos_grid.x;
		sb->grid[id] = 0;
		dead_mode = 2; 
		sb->msg.AddMsg(L"一个属于" + sb->faction_names[faction] + L"的船坞被摧毁了.", sb->faction_colors[faction]);
		return; 
	}
	if (dead_mode == 2) { return; }
	if (health < 0) { dead_mode = 1; return; }

	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);

	double init_v = 10;

	if (!sb->paused) {
		next_ship -= sb->dt;
		if (next_ship < 0) {
			next_ship = max_next_ship;
			sb->new_damageables.push_back(make_shared<Ship>(sb, faction, sb->pos_grid(pos_grid), vec2(init_v, 0)));
		}
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_SHIPYARD && sb->shipyard_observed == this;
	highlight_target = sb->target_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_SHIPYARD;
			sb->shipyard_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_shipyard);
			sb->sc_setting.SetCl(&sb->cl_observe_shipyard);
			break;
		}
	}
}

void Shipyard::Render() const {
	double r = sb->s_grid * 0.4 / sb->eye_dist;
	draw_eclipse_framed(sb->scr, sb->send(sb->pos_grid(pos_grid)), r, r, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);

	if (highlight) {
		double r_h = sb->s_grid * 0.6 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
	}
	else if (highlight_target) {
		double r_h = sb->s_grid * 0.6 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(255, 128, 0), 3);
	}
}
#pragma endregion

#pragma region Generator
vec2 Generator::Pos() const { return sb->pos_grid(pos_grid); }
bool Generator::TestHover(double &d) const {
	if (d > depth_generator) return false;
	dvec2 p;
	return sb->grid_chosen(p) && p == pos_grid ? d = depth_generator, true : false;
}

void Generator::Update() {
	if (dead_mode == 1) {
		int id = pos_grid.y * sb->nx_grid + pos_grid.x;
		sb->grid[id] = 0;
		dead_mode = 2;
		sb->msg.AddMsg(L"一个属于" + sb->faction_names[faction] + L"的反应堆被摧毁了.", sb->faction_colors[faction]);
		return;
	}
	if (dead_mode == 2) { return; }
	if (health < 0) { dead_mode = 1; return; }

	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);

	double de = 1;
	if (!sb->paused) {
		sb->faction_energy[faction] += de * sb->dt;
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_GENERATOR && sb->generator_observed == this;
	highlight_target = sb->target_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_GENERATOR;
			sb->generator_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_generator);
			sb->sc_setting.SetCl(&sb->cl_observe_generator);
			break;
		}
	}
}

void Generator::Render() const {
	double as = sb->s_grid * 0.6;
	double bs = sb->s_grid * 0.3;
	draw_rect_framed_transform(sb->scr, sb->send(sb->pos_grid(pos_grid)), as, bs, sb->tsf, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);

	if (highlight) {
		double r_h = sb->s_grid * 0.6 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
	}
	else if (highlight_target) {
		double r_h = sb->s_grid * 0.6 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(255, 128, 0), 3);
	}
}
#pragma endregion

#pragma region CmdCenter
vec2 CmdCenter::Pos() const { return sb->pos_grid(pos_grid); }
bool CmdCenter::TestHover(double &d) const {
	if (d > depth_cmd_center) return false;
	dvec2 p;
	return sb->grid_chosen(p) && p == pos_grid ? d = depth_cmd_center, true : false;
}

void CmdCenter::Update() {
	if (dead_mode == 1) {
		int id = pos_grid.y * sb->nx_grid + pos_grid.x;
		sb->grid[id] = 0;
		dead_mode = 2;
		sb->msg.AddMsg(L"一个属于" + sb->faction_names[faction] + L"的指挥中心被摧毁了.", sb->faction_colors[faction]);
		return;
	}
	if (dead_mode == 2) { return; }
	if (health < 0) { dead_mode = 1; return; }

	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);

	double max_next_structure = 10;
	double de = 2.0;
	if (!sb->paused) {
		sb->faction_energy[faction] += de * sb->dt;
		if (sb->faction_energy[faction] > sb->plan_energy(plan)) {
			vector<dvec2> free_pos;
			rep(i, max(0, pos_grid.x - sb->h_build_s), min(sb->nx_grid, pos_grid.x + sb->h_build_s + 1))
				rep(j, max(0, pos_grid.y - sb->h_build_s), min(sb->ny_grid, pos_grid.y + sb->h_build_s + 1)) {
				int id = j * sb->nx_grid + i;
				if (sb->grid[id] == 0) {
					free_pos.push_back({ i, j });
				}
			}
			if (!free_pos.empty()) {
				sb->faction_energy[faction] -= sb->plan_energy(plan);
				dvec2 p = free_pos[drand(0, (int)free_pos.size() - 1)];
				int id = p.y * sb->nx_grid + p.x;
				sb->grid[id] = 1;

				switch (plan) {
				case PLAN_SHIPYARD: 
					sb->msg.AddMsg(sb->faction_names[faction] + L"建造了一个船坞.", sb->faction_colors[faction]);
					sb->new_damageables.push_back(make_shared<Shipyard>(sb, faction, p)); 
					break;
				case PLAN_TURRENT: 
					sb->msg.AddMsg(sb->faction_names[faction] + L"建造了一个炮塔.", sb->faction_colors[faction]);
					sb->new_damageables.push_back(make_shared<Turrent>(sb, faction, p)); 
					break;
				case PLAN_GENERATOR: 
					sb->msg.AddMsg(sb->faction_names[faction] + L"建造了一个反应堆.", sb->faction_colors[faction]);
					sb->new_damageables.push_back(make_shared<Generator>(sb, faction, p)); 
					break;
				}

				double dice = frand();
				plan = dice < 0.2 ? PLAN_GENERATOR : dice < 0.6 ? PLAN_SHIPYARD : PLAN_TURRENT;
			}
		}
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_CMD_CENTER && sb->cmd_center_observed == this;
	highlight_target = sb->target_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_CMD_CENTER;
			sb->cmd_center_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_cmd_center);
			sb->sc_setting.SetCl(&sb->cl_observe_cmd_center);
			break;
		}
	}
}
void CmdCenter::Render() const {
	double s = sb->s_grid * 0.8;
	draw_rect_framed_transform(sb->scr, sb->send(sb->pos_grid(pos_grid)), s, s, sb->tsf, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);
	draw_rect_frame_transform(sb->scr, sb->send(sb->pos_grid(pos_grid)), sb->s_grid * sb->build_s(), sb->s_grid * sb->build_s(), sb->tsf, sb->vp_film(), sb->faction_colors[faction], 3);

	if (highlight) {
		double r_h = sb->s_grid * 0.8 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
	}
	else if (highlight_target) {
		double r_h = sb->s_grid * 0.8 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(255, 128, 0), 3);
	}
}
#pragma endregion

#pragma region Turrent
vec2 Turrent::Pos() const { return sb->pos_grid(pos_grid); }
bool Turrent::TestHover(double &d) const {
	if (d > depth_turrent) return false;
	dvec2 p;
	return sb->grid_chosen(p) && p == pos_grid ? d = depth_turrent, true : false;
}

void Turrent::Update() {
	if (dead_mode == 1) {
		int id = pos_grid.y * sb->nx_grid + pos_grid.x;
		sb->grid[id] = 0;
		dead_mode = 2; 
		sb->msg.AddMsg(L"一个属于" + sb->faction_names[faction] + L"的炮塔被摧毁了.", sb->faction_colors[faction]);
		return; }
	if (dead_mode == 2) { return; }
	if (health < 0) { dead_mode = 1; return; }

	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);

	double max_next_fire = 1;
	double bullet_v = 600;
	double fire_rr = turrent_fire_r * turrent_fire_r;
	double d_an = 3;


	if (!sb->paused) {
		next_fire -= sb->dt;
		vec2 pos = sb->pos_grid(pos_grid);

		if (target == NULL || target->DeadMode() != DEADMODE_ALIVE || (target->Pos() - pos).lensqr() > turrent_target_rr) {
			target = NULL;
			for (auto d : sb->damageables) {
				if (d->Faction() != faction && d->DeadMode() == DEADMODE_ALIVE && (target == NULL || (target->Pos() - pos).lensqr() > (d->Pos() - pos).lensqr())) {
					target = &*d;
				}
			}
		}

		if (target != NULL) {
			vec2 da = target->Pos() - pos;
			
			if (da != vec2()) {
				double an_change = d_an * sb->dt;
				double an = atan2(da.y, da.x);
				double dif = an - angle;
				while (dif > PI) dif -= PI * 2;
				while (dif < -PI) dif += PI * 2;

				if (abs(dif) < an_change) {
					angle = an;
				}
				else {
					angle += dif > 0 ? an_change : -an_change;
				}
			}
			
		}

		if (next_fire < 0 && target != NULL) {
			vec2 da = target->Pos() - pos;
			if (da != vec2()) {
				// vec2 ua = da / sqrt(da.lensqr());
				vec2 ua = vec2(cos(angle), sin(angle));
				double an_change = d_an * sb->dt;
				double an = atan2(da.y, da.x);
				double dif = an - angle;
				while (dif > PI) dif -= PI * 2;
				while (dif < -PI) dif += PI * 2;

				if (da.lensqr() < fire_rr && abs(dif) < small_angle) {
					next_fire = max_next_fire;
					sb->new_others.push_back(make_shared<Bullet>(sb, faction, pos, ua * bullet_v));
				}
			}
			else {
				sb->new_others.push_back(make_shared<Bullet>(sb, faction, pos, vec2()));
			}
		}
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_TURRENT && sb->turrent_observed == this;
	highlight_target = sb->target_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_TURRENT;
			sb->turrent_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_turrent);
			sb->sc_setting.SetCl(&sb->cl_observe_turrent);
			break;
		}
	}
}

void Turrent::Render() const {
	double s = sb->s_grid * 0.4;
	draw_rect_framed_transform(sb->scr, sb->send(sb->pos_grid(pos_grid)), s, s, sb->tsf, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);
	
	double r_a = sb->s_grid * 0.4;
	draw_segment(sb->scr, (dvec2)sb->send(sb->pos_grid(pos_grid)), (dvec2)sb->send(sb->pos_grid(pos_grid) + r_a * vec2(cos(angle), sin(angle))), 2, sb->vp_film(), {}, 255);

	if (highlight) {
		double r_h = sb->s_grid * 0.4 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), turrent_fire_r / sb->eye_dist, turrent_fire_r / sb->eye_dist, sb->vp_film(), dcol3(0, 255, 128), 3);
	}
	else if (highlight_target) {
		double r_h = sb->s_grid * 0.4 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(sb->pos_grid(pos_grid)), r_h, r_h, sb->vp_film(), dcol3(255, 128, 0), 3);
	}
}
#pragma endregion

#pragma region Ship
bool Ship::TestHover(double &d) const {
	if (d > depth_shipyard) return false;
	double rr = sb->s_grid * 0.2;
	rr *= rr;
	return inside(sb->msp, sb->vp_film()) && (sb->send_back((vec2)sb->msp) - pos).lensqr() < rr ? d = depth_ship, true : false;
}

void Ship::Update() {
	if (dead_mode == 1) { 
		dead_mode = 2; 
		// sb->msg.AddMsg(L"一艘属于" + sb->faction_names[faction] + L"的飞船被摧毁了.", sb->faction_colors[faction]);
		return; 
	}
	if (dead_mode == 2) { return; }
	if (health < 0) { dead_mode = 1; return; }
	
	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);


	double accelaration = 120.0;
	double max_vertical_vv = 12000.0;
	double max_vv = 80000.0;
	double close_vv = 30000.0;
	double max_next_fire = 1;
	double bullet_v = 600;
	double fire_rr = ship_fire_r * ship_fire_r;
	double d_an = 3;

	if (!sb->paused) {
		next_fire -= sb->dt;
		if (target == NULL || target->DeadMode() != DEADMODE_ALIVE || (target->Pos() - pos).lensqr() > ship_target_rr) {
			target = NULL;
			for (auto d : sb->damageables) {
				if (d->Faction() != faction && d->DeadMode() == DEADMODE_ALIVE && (target == NULL || (target->Pos() - pos).lensqr() > (d->Pos() - pos).lensqr())) {
					target = &*d;
				}
			}
		}

		if (target != NULL) {
			vec2 da = target->Pos() - pos;
			if (da != vec2()) {
				vec2 ua = da / sqrt(da.lensqr());
				vec2 va = ua * dot(ua, v);
				vec2 vb = v - va;
				double vv = v.lensqr();
				if (vv != 0) {
					vec2 uv = v / sqrt(vv);
					double bb = vb.lensqr();

					if (bb != 0) {
						vec2 ub = vb / sqrt(bb);
						if (bb > max_vertical_vv) {
							v -= ub * sb->dt * accelaration;
						}
						else if (da.lensqr() > ship_close_rr) {
							if (dot(ua, v) < 0 || vv < max_vv) {
								v += ua * sb->dt * accelaration;
							}
						}
						else if (vv > close_vv) {
							v -= uv * sb->dt * accelaration;
						}
					}
					else if (da.lensqr() > ship_close_rr) {
						if (dot(ua, v) < 0 || vv < max_vv) {
							v += ua * sb->dt * accelaration;
						}
					}
					else if (vv > close_vv) {
						v -= uv * sb->dt * accelaration;
					}
				}
				else if (da.lensqr() > ship_close_rr) {
					v += ua * sb->dt * accelaration;
				}
			}
		}

		pos += v * sb->dt;

		if (target != NULL) {
			vec2 da = target->Pos() - pos;

			if (da != vec2()) {
				double an_change = d_an * sb->dt;
				double an = atan2(da.y, da.x);
				double dif = an - angle;
				while (dif > PI) dif -= PI * 2;
				while (dif < -PI) dif += PI * 2;

				if (abs(dif) < an_change) {
					angle = an;
				}
				else {
					angle += dif > 0 ? an_change : -an_change;
				}
			}

		}

		if (next_fire < 0 && target != NULL) {
			vec2 da = target->Pos() - pos;
			if (da != vec2()) {
				// vec2 ua = da / sqrt(da.lensqr());
				vec2 ua = vec2(cos(angle), sin(angle));
				double an_change = d_an * sb->dt;
				double an = atan2(da.y, da.x);
				double dif = an - angle;
				while (dif > PI) dif -= PI * 2;
				while (dif < -PI) dif += PI * 2;

				if (da.lensqr() < fire_rr && abs(dif) < small_angle) {
					next_fire = max_next_fire;
					sb->new_others.push_back(make_shared<Bullet>(sb, faction, pos, ua * bullet_v));
				}
			}
			else {
				sb->new_others.push_back(make_shared<Bullet>(sb, faction, pos, vec2()));
			}
		}
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_SHIP && sb->ship_observed == this;
	highlight_target = sb->target_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_SHIP;
			sb->ship_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_ship);
			sb->sc_setting.SetCl(&sb->cl_observe_ship);
			break;
		}
	}
}

void Ship::Render() const {
	double r = sb->s_grid * 0.2 / sb->eye_dist;
	draw_eclipse_framed(sb->scr, sb->send(pos), r, r, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);

	double r_a = sb->s_grid * 0.4;
	draw_segment(sb->scr, (dvec2)sb->send(pos), (dvec2)sb->send(pos + r_a * vec2(cos(angle), sin(angle))), 2, sb->vp_film(), {}, 255);

	if (highlight) {
		double r_h = sb->s_grid * 0.4 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(pos), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
		draw_eclipse_frame(sb->scr, sb->send(pos), ship_fire_r / sb->eye_dist, ship_fire_r / sb->eye_dist, sb->vp_film(), dcol3(0, 255, 128), 3);
	}
	else if (highlight_target) {
		double r_h = sb->s_grid * 0.4 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(pos), r_h, r_h, sb->vp_film(), dcol3(255, 128, 0), 3);
	}
}
#pragma endregion

#pragma region Bullet
bool Bullet::TestHover(double &d) const {
	if (d > depth_shipyard) return false;
	double rr = sb->s_grid * 0.1;
	rr *= rr;
	return inside(sb->msp, sb->vp_film()) && (sb->send_back((vec2)sb->msp) - pos).lensqr() < rr ? d = depth_bullet, true : false;
}

void Bullet::Update() {
	if (dead_mode == 1) { 
		dead_mode = 2; 
		return; }
	if (dead_mode == 2) { return; }
	if (time_left < 0) { dead_mode = 1; return; }
	

	sb->Renderables.push_back(this);
	sb->Hoverables.push_back(this);

	double hit_rr = 10.0 * 10;
	double health_lost = 1.01;

	if (!sb->paused) {
		time_left -= sb->dt;
		for (auto d : sb->damageables) {
			if (d->Faction() != faction && (d->Pos() - pos).lensqr() < hit_rr) {
				d->SetHealth(d->GetHealth() - health_lost);
				dead_mode = DEADMODE_ALMOST_DEAD;
				break;
			}
		}
		pos += v * sb->dt;
	}

	highlight = sb->mode == SpaceBattle::OBSERVE_BULLET && sb->bullet_observed == this;

	if (hovered && sb->msd[0] && !sb->msd_old[0]) {
		switch (sb->mode) {
		case SpaceBattle::OBSERVE_IDLE:
		case SpaceBattle::OBSERVE_SHIPYARD:
		case SpaceBattle::OBSERVE_TURRENT:
		case SpaceBattle::OBSERVE_SHIP:
		case SpaceBattle::OBSERVE_BULLET:
		case SpaceBattle::OBSERVE_CMD_CENTER:
		case SpaceBattle::OBSERVE_GENERATOR:
			sb->mode = SpaceBattle::OBSERVE_BULLET;
			sb->bullet_observed = this;
			sb->pn_setting.SetCur(&sb->cl_observe_bullet);
			sb->sc_setting.SetCl(&sb->cl_observe_bullet);
			break;
		}
	}
}

void Bullet::Render() const {
	double r = sb->s_grid * 0.1 / sb->eye_dist;
	draw_eclipse_framed(sb->scr, sb->send(pos), r, r, sb->vp_film(), sb->faction_colors[faction], 255, {}, 2);

	if (highlight) {
		double r_h = sb->s_grid * 0.3 / sb->eye_dist;
		draw_eclipse_frame(sb->scr, sb->send(pos), r_h, r_h, sb->vp_film(), dcol3(0, 0, 255), 3);
	}
}
#pragma endregion


int main() {
	SpaceBattle sb;
	sb.Load();
	sb.Run(false, false);
}

// TODO:
// 可变高度的富文本框.

// 带有字体的字符串.
// 自己的 dt