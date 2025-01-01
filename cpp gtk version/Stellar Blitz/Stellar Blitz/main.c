#include <gtk/gtk.h>      // GTK+ 庫，用於建立圖形用戶界面
#include <locale.h>       // 本地化設定
#include <math.h>         // 數學函式庫，提供數學運算
#include <stdlib.h>       // 標準庫，包含通用工具函式
#include <time.h>         // 時間相關函式庫，用於生成隨機數
#include <string.h>       // 字串處理函式庫

#ifndef M_PI
#define M_PI 3.14159265358979323846   // 定義圓周率常數，如果未定義
#endif

/* === 常數定義 === */
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600
#define GAME_TICK_MS   16              // 遊戲更新的間隔時間，以毫秒為單位

#define PLAYER_SPEED   5.0            // 玩家移動速度
#define PLAYER_SIZE    20.0           // 玩家大小（半徑）
#define INVINCIBLE_TIME 1.0           // 無敵時間（秒）
#define HP_MAX         3              // 玩家最大生命值

#define BULLET_SPEED   8.0            // 子彈速度
#define BULLET_SIZE    5              // 子彈大小（半徑）
#define BULLET_COOLDOWN 0.2           // 子彈冷卻時間（秒）

#define ENEMY_SIZE     15             // 敵機大小（半徑）
#define ENEMY_SPEED    3.0            // 敵機移動速度
#define ENEMY_SCORE    100            // 擊敗敵機獲得的分數
#define ENEMY_SPAWN_INTERVAL 1.0      // 敵機生成間隔時間（秒）

#define DODGE_SCORE_PER_SEC  10       // 閃避模式每秒獲得的分數
#define TIME_ATTACK_LIMIT    30.0     // 時間攻擊模式的時間限制（秒）
#define CONQUEST_KILL_TARGET 10       // 征服模式需要擊殺的目標數量
#define BOSS_SIZE_RATIO      5.0      // Boss 大小倍率
#define BOSS_SPEED_RATIO     0.75     // Boss 移動速度倍率
#define BOSS_HP              10       // Boss 生命值
#define BOSS_SCORE           500      // 擊敗 Boss 獲得的分數

typedef enum {
    STATE_MENU,   // 主選單狀態
    STATE_GAME    // 遊戲進行狀態
} GameState;

typedef enum {
    MODE_DODGE,        // 閃避模式
    MODE_TIME_ATTACK,  // 時間攻擊模式
    MODE_CONQUEST      // 征服模式
} GameMode;

/* 子彈結構，包含子彈的位置及速度 */
typedef struct {
    double x, y;      // 子彈的 x 和 y 座標
    double speed;     // 子彈的移動速度
} Bullet;

/* 敵機結構，包含敵機的位置、方向、速度及是否為Boss */
typedef struct {
    double x, y;          // 敵機的 x 和 y 座標
    double dx, dy;        // 敵機的移動方向向量
    double speed;         // 敵機的移動速度
    gboolean is_boss;     // 是否為 Boss
    int boss_hp;          // Boss 的生命值
} Enemy;

/* 遊戲資料結構，包含遊戲的所有狀態和元素 */
typedef struct {
    GameState state;      // 當前遊戲狀態
    GameMode  mode;       // 當前遊戲模式

    GtkWidget* window;    // 主視窗
    GtkWidget* stack;     // 堆疊容器，用於切換不同頁面
    GtkWidget* page_menu; // 主選單頁面
    GtkWidget* page_game; // 遊戲頁面

    int width;            // 視窗寬度
    int height;           // 視窗高度

    /* 玩家相關資料 */
    double player_x, player_y; // 玩家位置
    int hp;                    // 玩家當前生命值
    gboolean invincible;      // 玩家是否處於無敵狀態
    double invincible_timer;  // 無敵計時器
    gboolean up_pressed;      // 是否按下上鍵
    gboolean down_pressed;    // 是否按下下鍵
    gboolean left_pressed;    // 是否按下左鍵
    gboolean right_pressed;   // 是否按下右鍵
    gboolean space_pressed;   // 是否按下空白鍵

    /* 子彈相關資料 */
    GList* bullets;           // 子彈鏈表
    double bullet_cooldown;  // 子彈冷卻計時器

    /* 敵機相關資料 */
    GList* enemies;           // 敵機鏈表
    double enemy_spawn_timer; // 敵機生成計時器

    /* 分數與時間 */
    int score;                // 玩家分數
    double last_time;         // 上一次更新的時間戳

    double dodge_score_timer; // 閃避模式分數計時器
    double time_left;         // 時間攻擊模式剩餘時間
    gboolean time_attack_done; // 時間攻擊模式是否完成

    int enemies_killed;       // 征服模式擊殺的敵機數量
    gboolean boss_spawned;    // 是否已生成 Boss

    /* 遊戲更新循環是否已啟動 */
    gboolean game_loop_started;
} GameData;

/* === 前置宣告 === */
static void app_activate(GApplication* app, gpointer user_data);
static void build_ui(GameData* gd);

static void on_button_start_clicked(GtkButton* btn, gpointer user_data);
static void on_button_mode_clicked(GtkButton* btn, gpointer user_data);
static void on_button_exit_clicked(GtkButton* btn, gpointer user_data);

static void start_game(GameData* gd);
static void game_return_to_menu(GameData* gd);

static double rand_range(double min, double max);
static gboolean circle_collide(double x1, double y1, double r1,
    double x2, double y2, double r2);
static void game_data_init(GameData* gd);

static Bullet* bullet_new(double x, double y);
static Enemy* enemy_new_normal(int w, int h);
static Enemy* enemy_new_boss(GameData* gd);
static gboolean can_player_fire(GameData* gd);

static void update_mode_specific(GameData* gd, double dt);
static void game_update(GameData* gd);
static gboolean game_loop(gpointer user_data);

static void on_draw(GtkDrawingArea* area, cairo_t* cr,
    int w, int h, gpointer user_data);
static gboolean on_key_press(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data);
static gboolean on_key_release(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data);


/* === main 函式 === */
int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");             // 設定本地化，以支持不同語言
    srand((unsigned int)time(NULL));   // 初始化隨機數生成器

    GameData* gd = g_new0(GameData, 1); // 分配並初始化 GameData 結構
    game_data_init(gd);                  // 初始化遊戲資料

    GtkApplication* app = gtk_application_new("org.example.StellarBlitzFixedBoss",
        G_APPLICATION_DEFAULT_FLAGS);    // 建立新的 GTK 應用程序
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), gd); // 連接激活信號

    int status = g_application_run(G_APPLICATION(app), argc, argv); // 執行應用程序

    g_object_unref(app); // 釋放應用程序對象
    g_free(gd);         // 釋放遊戲資料內存
    return status;      // 返回應用程序狀態
}

/* === app_activate 函式：應用程序啟動時的回調函式 === */
static void app_activate(GApplication* app, gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 轉換用戶數據為 GameData 指針

    gd->window = gtk_application_window_new(GTK_APPLICATION(app)); // 創建主視窗
    gtk_window_set_title(GTK_WINDOW(gd->window), "Stellar Blitz - Fixed Boss Crash"); // 設定視窗標題
    gtk_window_set_default_size(GTK_WINDOW(gd->window), WINDOW_WIDTH, WINDOW_HEIGHT); // 設定預設大小
    gtk_window_set_resizable(GTK_WINDOW(gd->window), FALSE); // 禁止改變視窗大小

    build_ui(gd); // 建立用戶界面

    gtk_widget_show(gd->window); // 顯示視窗

    /* 只啟動一次遊戲計時器，避免重複啟動 */
    if (!gd->game_loop_started) {
        g_timeout_add(GAME_TICK_MS, game_loop, gd); // 每 GAME_TICK_MS 毫秒呼叫 game_loop
        gd->game_loop_started = TRUE;               // 標記遊戲循環已啟動
    }
}

/* === build_ui 函式：建立用戶界面 === */
static void build_ui(GameData* gd)
{
    if (gd->stack) { // 檢查是否已建立堆疊容器
        g_print("[WARN] build_ui() called again => skip\n"); // 警告訊息
        return; // 避免重複建立 UI
    }

    gd->stack = gtk_stack_new(); // 創建新的堆疊容器
    gtk_stack_set_transition_type(GTK_STACK(gd->stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE); // 設定轉場效果
    gtk_stack_set_transition_duration(GTK_STACK(gd->stack), 300); // 設定轉場時間（毫秒）

    /* 建立主選單容器 */
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // 創建垂直排列的盒子，間距為 10
    gtk_widget_set_margin_top(vbox, 50);    // 設定頂部邊距
    gtk_widget_set_margin_bottom(vbox, 50); // 設定底部邊距
    gtk_widget_set_margin_start(vbox, 50);  // 設定起始邊距（左）
    gtk_widget_set_margin_end(vbox, 50);    // 設定結束邊距（右）

    /* 創建主選單按鈕 */
    GtkWidget* btn_start = gtk_button_new_with_label("Start Game");           // 開始遊戲按鈕
    GtkWidget* btn_mode = gtk_button_new_with_label("Mode: Dodge");          // 遊戲模式切換按鈕
    GtkWidget* btn_exit = gtk_button_new_with_label("Exit");                  // 退出遊戲按鈕

    /* 連接按鈕點擊事件到相應的回調函式 */
    g_signal_connect(btn_start, "clicked", G_CALLBACK(on_button_start_clicked), gd);
    g_signal_connect(btn_mode, "clicked", G_CALLBACK(on_button_mode_clicked), gd);
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(on_button_exit_clicked), gd);

    /* 將按鈕加入垂直盒子中 */
    gtk_box_append(GTK_BOX(vbox), btn_start);
    gtk_box_append(GTK_BOX(vbox), btn_mode);
    gtk_box_append(GTK_BOX(vbox), btn_exit);

    gd->page_menu = vbox; // 設定主選單頁面
    gd->page_game = gtk_label_new("Placeholder: Press Start Game to begin"); // 遊戲頁面的占位符

    /* 將主選單和遊戲頁面加入堆疊容器 */
    gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_menu, "menu");
    gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_game, "game");
    gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "menu"); // 顯示主選單頁面

    gtk_window_set_child(GTK_WINDOW(gd->window), gd->stack); // 將堆疊容器設為視窗的子元件
}

/* === 主選單按鈕回調函式 === */
/* 開始遊戲按鈕點擊事件的回調函式 */
static void on_button_start_clicked(GtkButton* btn, gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 取得遊戲資料
    start_game(gd);                      // 啟動遊戲
}

/* 遊戲模式切換按鈕點擊事件的回調函式 */
static void on_button_mode_clicked(GtkButton* btn, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;           // 取得遊戲資料
    const char* lbl = gtk_button_get_label(btn);   // 取得按鈕當前的標籤

    /* 根據當前模式切換到下一個模式，並更新按鈕標籤 */
    if (g_strcmp0(lbl, "Mode: Dodge") == 0) {
        gd->mode = MODE_TIME_ATTACK;
        gtk_button_set_label(btn, "Mode: Time Attack");
    }
    else if (g_strcmp0(lbl, "Mode: Time Attack") == 0) {
        gd->mode = MODE_CONQUEST;
        gtk_button_set_label(btn, "Mode: Conquest");
    }
    else {
        gd->mode = MODE_DODGE;
        gtk_button_set_label(btn, "Mode: Dodge");
    }
}

/* 退出遊戲按鈕點擊事件的回調函式 */
static void on_button_exit_clicked(GtkButton* btn, gpointer user_data)
{
    GApplication* app = g_application_get_default(); // 取得默認的應用程序
    if (app) {
        g_application_quit(app); // 結束應用程序
    }
}

/* === start_game 函式：開始遊戲 === */
static void start_game(GameData* gd)
{
    gd->state = STATE_GAME; // 設定遊戲狀態為進行中

    /* 清除現有的子彈和敵機列表，並釋放內存 */
    g_list_free_full(gd->bullets, g_free);
    gd->bullets = NULL;
    g_list_free_full(gd->enemies, g_free);
    gd->enemies = NULL;

    /* 重設玩家資料和分數 */
    gd->hp = HP_MAX;              // 設定玩家生命值為最大值
    gd->invincible = FALSE;      // 取消無敵狀態
    gd->invincible_timer = 0.0;  // 重設無敵計時器
    gd->up_pressed = FALSE;      // 重設按鍵狀態
    gd->down_pressed = FALSE;
    gd->left_pressed = FALSE;
    gd->right_pressed = FALSE;
    gd->space_pressed = FALSE;

    gd->bullet_cooldown = 0.0;   // 重設子彈冷卻計時器
    gd->enemy_spawn_timer = 0.0; // 重設敵機生成計時器

    gd->player_x = gd->width / 2.0; // 將玩家置於視窗中央
    gd->player_y = gd->height / 2.0;
    gd->score = 0;                   // 重設分數
    gd->dodge_score_timer = 0.0;    // 重設閃避模式分數計時器
    gd->time_left = TIME_ATTACK_LIMIT; // 重設時間攻擊模式的剩餘時間
    gd->time_attack_done = FALSE;    // 重設時間攻擊完成標記
    gd->enemies_killed = 0;          // 重設徵服模式擊殺數量
    gd->boss_spawned = FALSE;         // 重設 Boss 生成標記
    gd->last_time = (double)g_get_monotonic_time() / 1000000.0; // 紀錄當前時間戳

    /* 建立真正的遊戲畫面：繪圖區域 */
    GtkWidget* drawing_area = gtk_drawing_area_new(); // 創建繪圖區域
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), gd->width); // 設定繪圖區域寬度
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), gd->height); // 設定繪圖區域高度

    gtk_widget_set_focusable(drawing_area, TRUE); // 允許繪圖區域接收焦點
    GtkEventController* keyctrl = gtk_event_controller_key_new(); // 創建鍵盤事件控制器
    g_signal_connect(keyctrl, "key-pressed", G_CALLBACK(on_key_press), gd);    // 連接鍵盤按下事件
    g_signal_connect(keyctrl, "key-released", G_CALLBACK(on_key_release), gd); // 連接鍵盤釋放事件
    gtk_widget_add_controller(drawing_area, keyctrl); // 將事件控制器加入繪圖區域

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area),
        on_draw, gd, NULL); // 設定繪圖回調函式

    /* 將繪圖區域加入堆疊容器，並顯示遊戲頁面 */
    if (gd->stack && GTK_IS_STACK(gd->stack)) {
        if (gtk_stack_get_child_by_name(GTK_STACK(gd->stack), "game") != NULL) {
            gtk_stack_remove(GTK_STACK(gd->stack), gd->page_game); // 移除舊的遊戲頁面
        }
        gd->page_game = drawing_area; // 設定新的遊戲頁面
        gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_game, "game"); // 加入堆疊容器
        gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "game");   // 顯示遊戲頁面
    }
}

/* === game_return_to_menu 函式：返回主選單 === */
static void game_return_to_menu(GameData* gd)
{
    gd->state = STATE_MENU; // 設定遊戲狀態為主選單

    /* 清除所有子彈和敵機，釋放內存 */
    g_list_free_full(gd->bullets, g_free);
    gd->bullets = NULL;
    g_list_free_full(gd->enemies, g_free);
    gd->enemies = NULL;

    /* 顯示主選單頁面 */
    if (gd->stack && GTK_IS_STACK(gd->stack)) {
        gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "menu");
    }
}

/* === game_data_init 函式：初始化遊戲資料 === */
static void game_data_init(GameData* gd)
{
    gd->state = STATE_MENU;    // 初始狀態為主選單
    gd->mode = MODE_DODGE;     // 初始遊戲模式為閃避模式

    gd->window = NULL;         // 初始化視窗指針
    gd->stack = NULL;          // 初始化堆疊容器指針
    gd->page_menu = NULL;      // 初始化主選單頁面指針
    gd->page_game = NULL;      // 初始化遊戲頁面指針

    gd->width = WINDOW_WIDTH;  // 設定視窗寬度
    gd->height = WINDOW_HEIGHT;// 設定視窗高度

    /* 初始化玩家資料 */
    gd->player_x = gd->width / 2.0; // 玩家初始位置置中
    gd->player_y = gd->height / 2.0;
    gd->hp = HP_MAX;                // 設定玩家生命值為最大值
    gd->invincible = FALSE;        // 玩家初始不無敵
    gd->invincible_timer = 0.0;    // 重設無敵計時器

    /* 初始化按鍵狀態 */
    gd->up_pressed = FALSE;
    gd->down_pressed = FALSE;
    gd->left_pressed = FALSE;
    gd->right_pressed = FALSE;
    gd->space_pressed = FALSE;

    gd->bullets = NULL;             // 初始化子彈鏈表
    gd->bullet_cooldown = 0.0;     // 重設子彈冷卻計時器

    gd->enemies = NULL;             // 初始化敵機鏈表
    gd->enemy_spawn_timer = 0.0;    // 重設敵機生成計時器

    gd->score = 0;                   // 重設分數
    gd->last_time = (double)g_get_monotonic_time() / 1000000.0; // 紀錄當前時間戳

    gd->dodge_score_timer = 0.0;    // 重設閃避模式分數計時器
    gd->time_left = TIME_ATTACK_LIMIT; // 重設時間攻擊模式剩餘時間
    gd->time_attack_done = FALSE;    // 重設時間攻擊完成標記
    gd->enemies_killed = 0;          // 重設徵服模式擊殺數量
    gd->boss_spawned = FALSE;         // 重設 Boss 生成標記

    gd->game_loop_started = FALSE;    // 標記遊戲循環尚未啟動
}

/* === 工具函式 === */
/* rand_range 函式：生成 min 到 max 之間的隨機數 */
static double rand_range(double min, double max)
{
    return min + (double)rand() / (double)RAND_MAX * (max - min);
}

/* circle_collide 函式：檢查兩個圓形是否碰撞 */
static gboolean circle_collide(double x1, double y1, double r1,
    double x2, double y2, double r2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist2 = dx * dx + dy * dy;        // 計算距離的平方
    double rr = (r1 + r2) * (r1 + r2);      // 計算半徑和的平方
    return(dist2 <= rr);                     // 如果距離平方小於等於半徑和的平方，則碰撞
}

/* === 建立子彈與敵機 === */
/* bullet_new 函式：創建新的子彈 */
static Bullet* bullet_new(double x, double y)
{
    Bullet* b = g_new0(Bullet, 1); // 分配並初始化 Bullet 結構
    b->x = x; b->y = y;             // 設定子彈的初始位置
    b->speed = BULLET_SPEED;        // 設定子彈的速度
    return b;                        // 返回子彈指針
}

/* enemy_new_normal 函式：創建新的普通敵機 */
static Enemy* enemy_new_normal(int w, int h)
{
    Enemy* e = g_new0(Enemy, 1); // 分配並初始化 Enemy 結構
    e->is_boss = FALSE;          // 設定為普通敵機
    e->boss_hp = 0;              // 普通敵機無 Boss 生命值

    int edge = rand() % 4;       // 隨機選擇生成敵機的邊緣
    if (edge == 0) {
        e->x = rand_range(0, w); e->y = 0;          // 上邊緣
    }
    else if (edge == 1) {
        e->x = rand_range(0, w); e->y = h;          // 下邊緣
    }
    else if (edge == 2) {
        e->x = 0; e->y = rand_range(0, h);          // 左邊緣
    }
    else {
        e->x = w; e->y = rand_range(0, h);          // 右邊緣
    }

    double tx = rand_range(0, w), ty = rand_range(0, h); // 目標點隨機在視窗內
    double dx = tx - e->x, dy = ty - e->y;               // 計算目標方向向量
    double length = sqrt(dx * dx + dy * dy);             // 計算向量長度
    if (length > 0) { e->dx = dx / length; e->dy = dy / length; } // 正規化方向向量
    else { e->dx = 0; e->dy = 1; }                        // 如果長度為零，預設向下移動
    e->speed = ENEMY_SPEED;                               // 設定敵機速度
    return e;                                              // 返回敵機指針
}

/* enemy_new_boss 函式：創建新的 Boss 敵機 */
static Enemy* enemy_new_boss(GameData* gd)
{
    Enemy* e = g_new0(Enemy, 1); // 分配並初始化 Enemy 結構
    e->is_boss = TRUE;           // 設定為 Boss
    e->boss_hp = BOSS_HP;        // 設定 Boss 的生命值

    int edge = rand() % 4;       // 隨機選擇生成 Boss 的邊緣
    if (edge == 0) {
        e->x = rand_range(0, gd->width); e->y = 0;          // 上邊緣
    }
    else if (edge == 1) {
        e->x = rand_range(0, gd->width); e->y = gd->height; // 下邊緣
    }
    else if (edge == 2) {
        e->x = 0; e->y = rand_range(0, gd->height);          // 左邊緣
    }
    else {
        e->x = gd->width; e->y = rand_range(0, gd->height); // 右邊緣
    }

    double tx = gd->player_x - e->x, ty = gd->player_y - e->y; // 目標點為玩家位置
    double length = sqrt(tx * tx + ty * ty);                     // 計算向量長度
    if (length > 0) { e->dx = tx / length; e->dy = ty / length; } // 正規化方向向量
    else { e->dx = 0; e->dy = 1; }                               // 如果長度為零，預設向下移動
    e->speed = ENEMY_SPEED * BOSS_SPEED_RATIO;                   // 設定 Boss 的速度
    return e;                                                     // 返回 Boss 指針
}

/* can_player_fire 函式：判斷玩家是否可以開火 */
static gboolean can_player_fire(GameData* gd)
{
    return (gd->mode != MODE_DODGE); // 除了閃避模式外，其他模式玩家都可以開火
}

/* === 模式特定的更新邏輯 === */
static void update_mode_specific(GameData* gd, double dt)
{
    switch (gd->mode) {
    case MODE_DODGE:
        gd->dodge_score_timer += dt; // 累加計時器
        while (gd->dodge_score_timer >= 1.0) { // 每累積一秒
            gd->dodge_score_timer -= 1.0;         // 減去一秒
            if (!gd->invincible && gd->hp > 0) {  // 如果玩家不無敵且有生命
                gd->score += DODGE_SCORE_PER_SEC; // 增加分數
            }
        }
        break;
    case MODE_TIME_ATTACK:
        if (!gd->time_attack_done) {           // 如果時間攻擊尚未完成
            gd->time_left -= dt;                // 減少剩餘時間
            if (gd->time_left <= 0) {           // 如果時間用完
                gd->time_left = 0;               // 確保時間不為負數
                gd->time_attack_done = TRUE;     // 標記時間攻擊完成
            }
        }
        if (gd->time_attack_done || gd->hp <= 0) { // 如果時間攻擊完成或玩家生命耗盡
            game_return_to_menu(gd);                // 返回主選單
        }
        break;
    case MODE_CONQUEST:
        if (!gd->boss_spawned && gd->enemies_killed >= CONQUEST_KILL_TARGET) { // 如果未生成 Boss 且擊殺數達標
            gd->boss_spawned = TRUE;            // 標記 Boss 已生成
            Enemy* boss = enemy_new_boss(gd);   // 創建新的 Boss
            gd->enemies = g_list_append(gd->enemies, boss); // 將 Boss 加入敵機列表
        }
        if (gd->hp <= 0) { // 如果玩家生命耗盡
            game_return_to_menu(gd); // 返回主選單
        }
        break;
    }
}

/* === game_update 函式：更新遊戲狀態 === */
static void game_update(GameData* gd)
{
    double now = (double)g_get_monotonic_time() / 1000000.0; // 取得當前時間（秒）
    double dt = now - gd->last_time;                         // 計算時間差
    gd->last_time = now;                                      // 更新上一次時間

    if (gd->hp > 0) { // 如果玩家仍有生命
        /* 玩家移動(以優化) */
        double dx = 0, dy = 0; // 初始化移動方向
        if (gd->up_pressed)    dy -= 1; // 按下上鍵，向上移動
        if (gd->down_pressed)  dy += 1; // 按下下鍵，向下移動
        if (gd->left_pressed)  dx -= 1; // 按下左鍵，向左移動
        if (gd->right_pressed) dx += 1; // 按下右鍵，向右移動
        double len = sqrt(dx * dx + dy * dy); // 計算方向向量長度
        if (len > 0) { dx /= len; dy /= len; } // 正規化方向向量
        gd->player_x += dx * PLAYER_SPEED; // 更新玩家位置
        gd->player_y += dy * PLAYER_SPEED;

        /* 確保玩家不超出視窗邊界 */
        if (gd->player_x < 0) gd->player_x = 0;
        if (gd->player_x > gd->width)  gd->player_x = gd->width;
        if (gd->player_y < 0) gd->player_y = 0;
        if (gd->player_y > gd->height) gd->player_y = gd->height;

        /* 處理無敵狀態計時器 */
        if (gd->invincible) {
            gd->invincible_timer -= dt; // 減少無敵計時器
            if (gd->invincible_timer <= 0) { // 如果無敵時間結束
                gd->invincible = FALSE;       // 取消無敵狀態
                gd->invincible_timer = 0;
            }
        }

        /* 玩家開火 */
        if (can_player_fire(gd) && gd->space_pressed && gd->bullet_cooldown <= 0) {
            Bullet* b = bullet_new(gd->player_x, gd->player_y); // 創建新子彈
            gd->bullets = g_list_append(gd->bullets, b);         // 將子彈加入子彈列表
            gd->bullet_cooldown = BULLET_COOLDOWN;               // 重設子彈冷卻計時器
        }
        else {
            gd->bullet_cooldown -= dt; // 減少子彈冷卻計時器
            if (gd->bullet_cooldown < 0) gd->bullet_cooldown = 0; // 確保冷卻時間不為負數
        }

        /* 更新子彈位置並移除出界的子彈 */
        for (GList* bl = gd->bullets; bl; ) {
            Bullet* b = (Bullet*)bl->data; // 取得子彈指針
            GList* next_b = bl->next;      // 記錄下一個子彈，避免在移除當前子彈後失效

            b->y -= b->speed;               // 更新子彈位置（向上移動）
            if (b->y < 0) {                 // 如果子彈移出視窗上方
                gd->bullets = g_list_remove(gd->bullets, b); // 從子彈列表中移除
                g_free(b);                                   // 釋放子彈內存
            }
            bl = next_b; // 移動到下一個子彈
        }

        /* 產生敵機 */
        gd->enemy_spawn_timer += dt; // 累加敵機生成計時器
        if (gd->enemy_spawn_timer >= ENEMY_SPAWN_INTERVAL) { // 如果到達生成間隔
            gd->enemy_spawn_timer = 0;                           // 重設生成計時器
            Enemy* e = enemy_new_normal(gd->width, gd->height); // 創建新敵機
            gd->enemies = g_list_append(gd->enemies, e);        // 將敵機加入敵機列表
        }

        /* ---- 修正重點：避免在迴圈中途直接呼叫返回主選單函式 ---- */
        gboolean end_game = FALSE; /* 記錄是否需要在迴圈結束後返回主選單 */

        /* 更新敵機位置，檢查碰撞，處理擊敗邏輯 */
        for (GList* el = gd->enemies; el; ) {
            Enemy* e = (Enemy*)el->data;    // 取得敵機指針
            GList* next_e = el->next;       // 記錄下一個敵機，避免在移除當前敵機後失效

            /* 如果是 Boss，讓 Boss 朝向玩家移動 */
            if (e->is_boss) {
                double tx = gd->player_x - e->x;
                double ty = gd->player_y - e->y;
                double length2 = sqrt(tx * tx + ty * ty); // 計算距離
                if (length2 > 0) {
                    tx /= length2;
                    ty /= length2;
                }
                e->dx = tx;
                e->dy = ty; // 更新 Boss 的移動方向
            }
            e->x += e->dx * e->speed; // 更新敵機位置
            e->y += e->dy * e->speed;

            double r = e->is_boss ? (ENEMY_SIZE * BOSS_SIZE_RATIO) : ENEMY_SIZE; // 計算敵機半徑
            /* 檢查敵機是否出界 */
            if (e->x < 0 || e->x > gd->width || e->y < 0 || e->y > gd->height) {
                gd->enemies = g_list_remove(gd->enemies, e); // 從敵機列表中移除
                g_free(e);                                   // 釋放敵機內存
                el = next_e;                                 // 移動到下一個敵機
                continue;                                    // 繼續迴圈
            }

            /* 檢查玩家與敵機的碰撞 */
            if (!gd->invincible) { // 如果玩家不無敵
                if (circle_collide(gd->player_x, gd->player_y, PLAYER_SIZE, e->x, e->y, r)) {
                    gd->hp--; // 減少玩家生命值
                    if (gd->hp <= 0) { // 如果玩家生命耗盡
                        end_game = TRUE; // 標記需要結束遊戲
                        break;          // 離開敵機迴圈
                    }
                    gd->invincible = TRUE;        // 設定玩家為無敵狀態
                    gd->invincible_timer = INVINCIBLE_TIME; // 設定無敵計時器
                }
            }

            /* 檢查子彈與敵機的碰撞 */
            if (can_player_fire(gd)) {
                gboolean destroyed = FALSE; // 標記敵機是否被擊敗
                for (GList* bl2 = gd->bullets; bl2; ) {
                    Bullet* b2 = (Bullet*)bl2->data;    // 取得子彈指針
                    GList* next_bu = bl2->next;          // 記錄下一個子彈

                    if (circle_collide(b2->x, b2->y, BULLET_SIZE, e->x, e->y, r)) {
                        /* 擊中敵機 */
                        if (e->is_boss) {
                            e->boss_hp--; // Boss 受到傷害
                            if (e->boss_hp <= 0) { // 如果 Boss 被擊敗
                                /* Boss 死亡處理 */
                                gd->score += BOSS_SCORE;          // 增加分數
                                gd->enemies = g_list_remove(gd->enemies, e); // 移除 Boss
                                g_free(e);                          // 釋放 Boss 內存
                                destroyed = TRUE;                   // 標記 Boss 被擊敗

                                /* 擊敗 Boss 後回到主選單 */
                                end_game = TRUE;
                            }
                        }
                        else {
                            gd->score += ENEMY_SCORE;             // 增加分數
                            gd->enemies = g_list_remove(gd->enemies, e); // 移除敵機
                            g_free(e);                               // 釋放敵機內存
                            destroyed = TRUE;                        // 標記敵機被擊敗
                            if (gd->mode == MODE_CONQUEST) gd->enemies_killed++; // 增加擊殺數
                        }

                        gd->bullets = g_list_remove(gd->bullets, b2); // 移除子彈
                        g_free(b2);                                   // 釋放子彈內存

                        if (destroyed) break; // 如果敵機被擊敗，跳出子彈迴圈
                    }
                    bl2 = next_bu; // 移動到下一個子彈
                }
                if (destroyed) {
                    el = next_e; // 移動到下一個敵機
                    continue;    // 繼續敵機迴圈
                }
            }

            el = next_e; // 移動到下一個敵機
            if (end_game) break; // 如果標記需要結束遊戲，跳出敵機迴圈
        }

        /* 迴圈結束後根據標記決定是否返回主選單 */
        if (end_game) {
            /* 若玩家HP<=0 或 Boss被擊敗，返回主選單 */
            game_return_to_menu(gd);
            return; // 結束更新函式
        }
    }

    /* 根據當前遊戲模式進行特定的更新 */
    update_mode_specific(gd, dt);
}

/* === game_loop 函式：遊戲主循環 === */
static gboolean game_loop(gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 取得遊戲資料
    if (gd->state == STATE_GAME) {        // 如果遊戲正在進行中
        game_update(gd);                  // 更新遊戲狀態
        gtk_widget_queue_draw(gd->page_game); // 請求重新繪製遊戲畫面
    }
    return TRUE; // 返回 TRUE 以保持循環繼續
}

/* === on_draw 函式：繪製遊戲畫面 === */
static void on_draw(GtkDrawingArea* area, cairo_t* cr,
    int w, int h, gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 取得遊戲資料

    /* 背景填充為黑色 */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    /* 繪製所有子彈（白色） */
    cairo_set_source_rgb(cr, 1, 1, 1); // 設定繪圖顏色為白色
    for (GList* bl = gd->bullets; bl; bl = bl->next) {
        Bullet* b = (Bullet*)bl->data; // 取得子彈指針
        cairo_arc(cr, b->x, b->y, BULLET_SIZE, 0, 2 * M_PI); // 繪製子彈圓形
        cairo_fill(cr); // 填充圓形
    }

    /* 繪製所有敵機 */
    for (GList* el = gd->enemies; el; el = el->next) {
        Enemy* e = (Enemy*)el->data; // 取得敵機指針
        double r = e->is_boss ? (ENEMY_SIZE * BOSS_SIZE_RATIO) : ENEMY_SIZE; // 計算敵機半徑
        if (e->is_boss) cairo_set_source_rgb(cr, 1.0, 0.3, 0.3); // Boss 為紅色調(偏粉紅)
        else           cairo_set_source_rgb(cr, 1.0, 0, 0);       // 普通敵機為紅色

        cairo_arc(cr, e->x, e->y, r, 0, 2 * M_PI); // 繪製敵機圓形
        cairo_fill(cr); // 填充圓形
    }

    /* 繪製玩家 */
    if (gd->hp > 0) { // 如果玩家仍有生命
        if (gd->invincible) { // 如果玩家無敵
            static gboolean toggle = FALSE; // 用於閃爍效果的靜態變數
            toggle = !toggle;               // 切換狀態
            if (toggle) cairo_set_source_rgb(cr, 1, 1, 0); // 黃色
            else       cairo_set_source_rgb(cr, 1, 0.5, 0); // 橙色
        }
        else {
            cairo_set_source_rgb(cr, 0, 1, 0); // 綠色
        }
        cairo_arc(cr, gd->player_x, gd->player_y, PLAYER_SIZE, 0, 2 * M_PI); // 繪製玩家圓形
        cairo_fill(cr); // 填充圓形
    }

    /* 顯示模式、分數及其他資訊 */
    cairo_set_source_rgb(cr, 1, 1, 1); // 設定文字顏色為白色
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD); // 設定字體
    cairo_set_font_size(cr, 20); // 設定字體大小

    char info[128]; // 用於儲存資訊字串
    switch (gd->mode) {
    case MODE_DODGE://選到閃避模式
        snprintf(info, sizeof(info),
            "Mode: Dodge | HP:%d | Score:%d",
            gd->hp, gd->score);
        break;
    case MODE_TIME_ATTACK://選到限時奪分模式
        snprintf(info, sizeof(info),
            "Mode: Time Attack | HP:%d | Score:%d | Time:%.1f",
            gd->hp, gd->score, gd->time_left);
        break;
    case MODE_CONQUEST://選到討伐模式
        snprintf(info, sizeof(info),
            "Mode: Conquest | HP:%d | Score:%d | Kills:%d",
            gd->hp, gd->score, gd->enemies_killed);
        break;
    }
    cairo_move_to(cr, 10, 30);          // 設定文字起始位置
    cairo_show_text(cr, info);          // 顯示資訊文字
}

/* === 鍵盤事件回調函式 === */
/* on_key_press 函式：鍵盤按下事件的回調函式 */
static gboolean on_key_press(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 取得遊戲資料
    switch (keyval) { // 根據按下的鍵值進行處理
    case GDK_KEY_w:
    case GDK_KEY_Up:    gd->up_pressed = TRUE;    break; // 按下上鍵
    case GDK_KEY_s:
    case GDK_KEY_Down:  gd->down_pressed = TRUE;  break; // 按下下鍵
    case GDK_KEY_a:
    case GDK_KEY_Left:  gd->left_pressed = TRUE;  break; // 按下左鍵
    case GDK_KEY_d:
    case GDK_KEY_Right: gd->right_pressed = TRUE; break; // 按下右鍵
    case GDK_KEY_space: gd->space_pressed = TRUE; break; // 按下空白鍵
    default: break; // 其他鍵不處理
    }
    return TRUE; // 返回 TRUE 表示事件已被處理
}

/* on_key_release 函式：鍵盤釋放事件的回調函式 */
static gboolean on_key_release(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data)
{
    GameData* gd = (GameData*)user_data; // 取得遊戲資料
    switch (keyval) { // 根據釋放的鍵值進行處理
    case GDK_KEY_w:
    case GDK_KEY_Up:    gd->up_pressed = FALSE;    break; // 釋放上鍵
    case GDK_KEY_s:
    case GDK_KEY_Down:  gd->down_pressed = FALSE;  break; // 釋放下鍵
    case GDK_KEY_a:
    case GDK_KEY_Left:  gd->left_pressed = FALSE;  break; // 釋放左鍵
    case GDK_KEY_d:
    case GDK_KEY_Right: gd->right_pressed = FALSE; break; // 釋放右鍵
    case GDK_KEY_space: gd->space_pressed = FALSE; break; // 釋放空白鍵
    default: break; // 其他鍵不處理
    }
    return TRUE; // 返回 TRUE 表示事件已被處理
}
