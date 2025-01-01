#include <gtk/gtk.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* === 常數 === */
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600
#define GAME_TICK_MS   16

#define PLAYER_SPEED   5.0
#define PLAYER_SIZE    20.0
#define INVINCIBLE_TIME 1.0
#define HP_MAX         3

#define BULLET_SPEED   8.0
#define BULLET_SIZE    5
#define BULLET_COOLDOWN 0.2

#define ENEMY_SIZE     15
#define ENEMY_SPEED    2.0
#define ENEMY_SCORE    100
#define ENEMY_SPAWN_INTERVAL 1.0

#define DODGE_SCORE_PER_SEC  10
#define TIME_ATTACK_LIMIT    60.0
#define CONQUEST_KILL_TARGET 10
#define BOSS_SIZE_RATIO      5.0
#define BOSS_SPEED_RATIO     0.75
#define BOSS_HP              5
#define BOSS_SCORE           500

typedef enum {
    STATE_MENU,
    STATE_GAME
} GameState;

typedef enum {
    MODE_DODGE,
    MODE_TIME_ATTACK,
    MODE_CONQUEST
} GameMode;

/* 子彈 / 敵機結構 */
typedef struct {
    double x, y;
    double speed;
} Bullet;

typedef struct {
    double x, y;
    double dx, dy;
    double speed;
    gboolean is_boss;
    int boss_hp;
} Enemy;

/* 遊戲資料 */
typedef struct {
    GameState state;
    GameMode  mode;

    GtkWidget* window;
    GtkWidget* stack;
    GtkWidget* page_menu;
    GtkWidget* page_game;

    int width;
    int height;

    /* 玩家 */
    double player_x, player_y;
    int hp;
    gboolean invincible;
    double invincible_timer;
    gboolean up_pressed;
    gboolean down_pressed;
    gboolean left_pressed;
    gboolean right_pressed;
    gboolean space_pressed;

    /* 子彈 */
    GList* bullets;
    double bullet_cooldown;

    /* 敵機 */
    GList* enemies;
    double enemy_spawn_timer;

    /* 分數 / 時間 */
    int score;
    double last_time;

    double dodge_score_timer;
    double time_left;
    gboolean time_attack_done;

    int enemies_killed;
    gboolean boss_spawned;

    /* 遊戲更新只啟用一次 timer 的標記 */
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


/* === main === */
int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");
    srand((unsigned int)time(NULL));

    GameData* gd = g_new0(GameData, 1);
    game_data_init(gd);

    GtkApplication* app = gtk_application_new("org.example.StellarBlitzFixedBoss",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), gd);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    g_free(gd);
    return status;
}

/* === app_activate === */
static void app_activate(GApplication* app, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;

    gd->window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(gd->window), "Stellar Blitz - Fixed Boss Crash");
    gtk_window_set_default_size(GTK_WINDOW(gd->window), WINDOW_WIDTH, WINDOW_HEIGHT);
    gtk_window_set_resizable(GTK_WINDOW(gd->window), FALSE);

    build_ui(gd);

    gtk_widget_show(gd->window);

    /* 只啟動一次遊戲計時器 */
    if (!gd->game_loop_started) {
        g_timeout_add(GAME_TICK_MS, game_loop, gd);
        gd->game_loop_started = TRUE;
    }
}

/* === build_ui === */
static void build_ui(GameData* gd)
{
    if (gd->stack) {
        g_print("[WARN] build_ui() called again => skip\n");
        return;
    }

    gd->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(gd->stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(gd->stack), 300);

    /* 建立主選單 */
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 50);
    gtk_widget_set_margin_bottom(vbox, 50);
    gtk_widget_set_margin_start(vbox, 50);
    gtk_widget_set_margin_end(vbox, 50);

    GtkWidget* btn_start = gtk_button_new_with_label("Start Game");
    GtkWidget* btn_mode = gtk_button_new_with_label("Mode: Dodge");
    GtkWidget* btn_exit = gtk_button_new_with_label("Exit");

    g_signal_connect(btn_start, "clicked", G_CALLBACK(on_button_start_clicked), gd);
    g_signal_connect(btn_mode, "clicked", G_CALLBACK(on_button_mode_clicked), gd);
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(on_button_exit_clicked), gd);

    gtk_box_append(GTK_BOX(vbox), btn_start);
    gtk_box_append(GTK_BOX(vbox), btn_mode);
    gtk_box_append(GTK_BOX(vbox), btn_exit);

    gd->page_menu = vbox;
    gd->page_game = gtk_label_new("Placeholder: Press Start Game to begin");

    gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_menu, "menu");
    gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_game, "game");
    gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "menu");

    gtk_window_set_child(GTK_WINDOW(gd->window), gd->stack);
}

/* === Menu callbacks === */
static void on_button_start_clicked(GtkButton* btn, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;
    start_game(gd);
}

static void on_button_mode_clicked(GtkButton* btn, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;
    const char* lbl = gtk_button_get_label(btn);

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

static void on_button_exit_clicked(GtkButton* btn, gpointer user_data)
{
    GApplication* app = g_application_get_default();
    if (app) {
        g_application_quit(app);
    }
}

/* === start_game === */
static void start_game(GameData* gd)
{
    gd->state = STATE_GAME;

    /* 清除敵人/子彈 */
    g_list_free_full(gd->bullets, g_free);
    gd->bullets = NULL;
    g_list_free_full(gd->enemies, g_free);
    gd->enemies = NULL;

    /* 重設玩家資料 / 分數 */
    gd->hp = HP_MAX;
    gd->invincible = FALSE;
    gd->invincible_timer = 0.0;
    gd->up_pressed = FALSE;
    gd->down_pressed = FALSE;
    gd->left_pressed = FALSE;
    gd->right_pressed = FALSE;
    gd->space_pressed = FALSE;

    gd->bullet_cooldown = 0.0;
    gd->enemy_spawn_timer = 0.0;

    gd->player_x = gd->width / 2.0;
    gd->player_y = gd->height / 2.0;
    gd->score = 0;
    gd->dodge_score_timer = 0.0;
    gd->time_left = TIME_ATTACK_LIMIT;
    gd->time_attack_done = FALSE;
    gd->enemies_killed = 0;
    gd->boss_spawned = FALSE;
    gd->last_time = (double)g_get_monotonic_time() / 1000000.0;

    /* 建立真正遊戲畫面 */
    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), gd->width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), gd->height);

    gtk_widget_set_focusable(drawing_area, TRUE);
    GtkEventController* keyctrl = gtk_event_controller_key_new();
    g_signal_connect(keyctrl, "key-pressed", G_CALLBACK(on_key_press), gd);
    g_signal_connect(keyctrl, "key-released", G_CALLBACK(on_key_release), gd);
    gtk_widget_add_controller(drawing_area, keyctrl);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area),
        on_draw, gd, NULL);

    if (gd->stack && GTK_IS_STACK(gd->stack)) {
        if (gtk_stack_get_child_by_name(GTK_STACK(gd->stack), "game") != NULL) {
            gtk_stack_remove(GTK_STACK(gd->stack), gd->page_game);
        }
        gd->page_game = drawing_area;
        gtk_stack_add_named(GTK_STACK(gd->stack), gd->page_game, "game");
        gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "game");
    }
}

/* === 回主選單 === */
static void game_return_to_menu(GameData* gd)
{
    gd->state = STATE_MENU;

    g_list_free_full(gd->bullets, g_free);
    gd->bullets = NULL;
    g_list_free_full(gd->enemies, g_free);
    gd->enemies = NULL;

    if (gd->stack && GTK_IS_STACK(gd->stack)) {
        gtk_stack_set_visible_child_name(GTK_STACK(gd->stack), "menu");
    }
}

/* === game_data_init === */
static void game_data_init(GameData* gd)
{
    gd->state = STATE_MENU;
    gd->mode = MODE_DODGE;

    gd->window = NULL;
    gd->stack = NULL;
    gd->page_menu = NULL;
    gd->page_game = NULL;

    gd->width = WINDOW_WIDTH;
    gd->height = WINDOW_HEIGHT;

    gd->player_x = gd->width / 2.0;
    gd->player_y = gd->height / 2.0;
    gd->hp = HP_MAX;
    gd->invincible = FALSE;
    gd->invincible_timer = 0.0;

    gd->up_pressed = FALSE;
    gd->down_pressed = FALSE;
    gd->left_pressed = FALSE;
    gd->right_pressed = FALSE;
    gd->space_pressed = FALSE;

    gd->bullets = NULL;
    gd->bullet_cooldown = 0.0;

    gd->enemies = NULL;
    gd->enemy_spawn_timer = 0.0;

    gd->score = 0;
    gd->last_time = (double)g_get_monotonic_time() / 1000000.0;

    gd->dodge_score_timer = 0.0;
    gd->time_left = TIME_ATTACK_LIMIT;
    gd->time_attack_done = FALSE;
    gd->enemies_killed = 0;
    gd->boss_spawned = FALSE;

    gd->game_loop_started = FALSE;
}

/* === 工具 === */
static double rand_range(double min, double max)
{
    return min + (double)rand() / (double)RAND_MAX * (max - min);
}

static gboolean circle_collide(double x1, double y1, double r1,
    double x2, double y2, double r2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist2 = dx * dx + dy * dy;
    double rr = (r1 + r2) * (r1 + r2);
    return(dist2 <= rr);
}

/* === 建立子彈 / 敵機 === */
static Bullet* bullet_new(double x, double y)
{
    Bullet* b = g_new0(Bullet, 1);
    b->x = x; b->y = y;
    b->speed = BULLET_SPEED;
    return b;
}

static Enemy* enemy_new_normal(int w, int h)
{
    Enemy* e = g_new0(Enemy, 1);
    e->is_boss = FALSE;
    e->boss_hp = 0;

    int edge = rand() % 4;
    if (edge == 0) {
        e->x = rand_range(0, w); e->y = 0;
    }
    else if (edge == 1) {
        e->x = rand_range(0, w); e->y = h;
    }
    else if (edge == 2) {
        e->x = 0; e->y = rand_range(0, h);
    }
    else {
        e->x = w; e->y = rand_range(0, h);
    }

    double tx = rand_range(0, w), ty = rand_range(0, h);
    double dx = tx - e->x, dy = ty - e->y;
    double length = sqrt(dx * dx + dy * dy);
    if (length > 0) { e->dx = dx / length; e->dy = dy / length; }
    else { e->dx = 0; e->dy = 1; }
    e->speed = ENEMY_SPEED;
    return e;
}

static Enemy* enemy_new_boss(GameData* gd)
{
    Enemy* e = g_new0(Enemy, 1);
    e->is_boss = TRUE;
    e->boss_hp = BOSS_HP;

    int edge = rand() % 4;
    if (edge == 0) {
        e->x = rand_range(0, gd->width); e->y = 0;
    }
    else if (edge == 1) {
        e->x = rand_range(0, gd->width); e->y = gd->height;
    }
    else if (edge == 2) {
        e->x = 0; e->y = rand_range(0, gd->height);
    }
    else {
        e->x = gd->width; e->y = rand_range(0, gd->height);
    }

    double tx = gd->player_x - e->x, ty = gd->player_y - e->y;
    double length = sqrt(tx * tx + ty * ty);
    if (length > 0) { e->dx = tx / length; e->dy = ty / length; }
    else { e->dx = 0; e->dy = 1; }
    e->speed = ENEMY_SPEED * BOSS_SPEED_RATIO;
    return e;
}

/* === 是否可開火 === */
static gboolean can_player_fire(GameData* gd)
{
    return (gd->mode != MODE_DODGE);
}

/* === 模式邏輯 === */
static void update_mode_specific(GameData* gd, double dt)
{
    switch (gd->mode) {
    case MODE_DODGE:
        gd->dodge_score_timer += dt;
        while (gd->dodge_score_timer >= 1.0) {
            gd->dodge_score_timer -= 1.0;
            if (!gd->invincible && gd->hp > 0) {
                gd->score += DODGE_SCORE_PER_SEC;
            }
        }
        break;
    case MODE_TIME_ATTACK:
        if (!gd->time_attack_done) {
            gd->time_left -= dt;
            if (gd->time_left <= 0) {
                gd->time_left = 0;
                gd->time_attack_done = TRUE;
            }
        }
        if (gd->time_attack_done || gd->hp <= 0) {
            game_return_to_menu(gd);
        }
        break;
    case MODE_CONQUEST:
        if (!gd->boss_spawned && gd->enemies_killed >= CONQUEST_KILL_TARGET) {
            gd->boss_spawned = TRUE;
            Enemy* boss = enemy_new_boss(gd);
            gd->enemies = g_list_append(gd->enemies, boss);
        }
        if (gd->hp <= 0) {
            game_return_to_menu(gd);
        }
        break;
    }
}

/* === game_update === */
static void game_update(GameData* gd)
{
    double now = (double)g_get_monotonic_time() / 1000000.0;
    double dt = now - gd->last_time;
    gd->last_time = now;

    if (gd->hp > 0) {
        /* 玩家移動... (略同之前) */
        double dx = 0, dy = 0;
        if (gd->up_pressed)    dy -= 1;
        if (gd->down_pressed)  dy += 1;
        if (gd->left_pressed)  dx -= 1;
        if (gd->right_pressed) dx += 1;
        double len = sqrt(dx * dx + dy * dy);
        if (len > 0) { dx /= len; dy /= len; }
        gd->player_x += dx * PLAYER_SPEED;
        gd->player_y += dy * PLAYER_SPEED;

        if (gd->player_x < 0) gd->player_x = 0;
        if (gd->player_x > gd->width)  gd->player_x = gd->width;
        if (gd->player_y < 0) gd->player_y = 0;
        if (gd->player_y > gd->height) gd->player_y = gd->height;

        if (gd->invincible) {
            gd->invincible_timer -= dt;
            if (gd->invincible_timer <= 0) {
                gd->invincible = FALSE;
                gd->invincible_timer = 0;
            }
        }

        /* 開火 */
        if (can_player_fire(gd) && gd->space_pressed && gd->bullet_cooldown <= 0) {
            Bullet* b = bullet_new(gd->player_x, gd->player_y);
            gd->bullets = g_list_append(gd->bullets, b);
            gd->bullet_cooldown = BULLET_COOLDOWN;
        }
        else {
            gd->bullet_cooldown -= dt;
            if (gd->bullet_cooldown < 0) gd->bullet_cooldown = 0;
        }

        /* 子彈更新 */
        for (GList* bl = gd->bullets; bl; ) {
            Bullet* b = (Bullet*)bl->data;
            GList* next_b = bl->next;
            b->y -= b->speed;
            if (b->y < 0) {
                gd->bullets = g_list_remove(gd->bullets, b);
                g_free(b);
            }
            bl = next_b;
        }

        /* 產生敵機 */
        gd->enemy_spawn_timer += dt;
        if (gd->enemy_spawn_timer >= ENEMY_SPAWN_INTERVAL) {
            gd->enemy_spawn_timer = 0;
            Enemy* e = enemy_new_normal(gd->width, gd->height);
            gd->enemies = g_list_append(gd->enemies, e);
        }

        /* ---- 修正重點：boss死亡 + return_to_menu 不要在迴圈中途直接呼叫 ---- */
        gboolean end_game = FALSE; /* 記錄「這個迴圈結束後要不要回主選單」 */

        /* 敵機更新/碰撞 */
        for (GList* el = gd->enemies; el; ) {
            Enemy* e = (Enemy*)el->data;
            GList* next_e = el->next; /* 記下下一個，避免 remove 後失效 */

            if (e->is_boss) {
                double tx = gd->player_x - e->x;
                double ty = gd->player_y - e->y;
                double length2 = sqrt(tx * tx + ty * ty);
                if (length2 > 0) { tx /= length2; ty /= length2; }
                e->dx = tx; e->dy = ty;
            }
            e->x += e->dx * e->speed;
            e->y += e->dy * e->speed;

            double r = e->is_boss ? (ENEMY_SIZE * BOSS_SIZE_RATIO) : ENEMY_SIZE;
            /* 出界 */
            if (e->x<0 || e->x>gd->width || e->y<0 || e->y>gd->height) {
                gd->enemies = g_list_remove(gd->enemies, e);
                g_free(e);
                el = next_e;
                continue;
            }

            if (!gd->invincible) {
                if (circle_collide(gd->player_x, gd->player_y, PLAYER_SIZE, e->x, e->y, r)) {
                    gd->hp--;
                    if (gd->hp <= 0) {
                        /* 記錄 end_game */
                        end_game = TRUE;
                        break;
                    }
                    gd->invincible = TRUE;
                    gd->invincible_timer = INVINCIBLE_TIME;
                }
            }

            if (can_player_fire(gd)) {
                gboolean destroyed = FALSE;
                for (GList* bl2 = gd->bullets; bl2; ) {
                    Bullet* b2 = (Bullet*)bl2->data;
                    GList* next_bu = bl2->next;

                    if (circle_collide(b2->x, b2->y, BULLET_SIZE, e->x, e->y, r)) {
                        /* 擊中敵機 */
                        if (e->is_boss) {
                            e->boss_hp--;
                            if (e->boss_hp <= 0) {
                                /* Boss死 => +BOSS_SCORE & remove */
                                gd->score += BOSS_SCORE;
                                gd->enemies = g_list_remove(gd->enemies, e);
                                g_free(e);
                                destroyed = TRUE;

                                /* 依需求：擊敗Boss就回主選單 => end_game=TRUE */
                                end_game = TRUE;
                            }
                        }
                        else {
                            gd->score += ENEMY_SCORE;
                            gd->enemies = g_list_remove(gd->enemies, e);
                            g_free(e);
                            destroyed = TRUE;
                            if (gd->mode == MODE_CONQUEST) gd->enemies_killed++;
                        }

                        gd->bullets = g_list_remove(gd->bullets, b2);
                        g_free(b2);

                        if (destroyed) break;
                    }
                    bl2 = next_bu;
                }
                if (destroyed) {
                    el = next_e;
                    continue;
                }
            }

            el = next_e;
            if (end_game) break; /* 若已標記要結束 => 離開迴圈 */
        }

        /* 迴圈結束後再決定是否回主選單，避免在迴圈中直接呼叫 return_to_menu()  */
        if (end_game) {
            /* 若玩家HP<=0 或 Boss死 => 回主選單 */
            game_return_to_menu(gd);
            return;
        }
    }

    update_mode_specific(gd, dt);
}

/* === game_loop === */
static gboolean game_loop(gpointer user_data)
{
    GameData* gd = (GameData*)user_data;
    if (gd->state == STATE_GAME) {
        game_update(gd);
        gtk_widget_queue_draw(gd->page_game);
    }
    return TRUE;
}

/* === on_draw === */
static void on_draw(GtkDrawingArea* area, cairo_t* cr,
    int w, int h, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    /* 子彈(白) */
    cairo_set_source_rgb(cr, 1, 1, 1);
    for (GList* bl = gd->bullets; bl; bl = bl->next) {
        Bullet* b = (Bullet*)bl->data;
        cairo_arc(cr, b->x, b->y, BULLET_SIZE, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    /* 敵機 */
    for (GList* el = gd->enemies; el; el = el->next) {
        Enemy* e = (Enemy*)el->data;
        double r = e->is_boss ? (ENEMY_SIZE * BOSS_SIZE_RATIO) : ENEMY_SIZE;
        if (e->is_boss) cairo_set_source_rgb(cr, 1.0, 0.3, 0.3);
        else           cairo_set_source_rgb(cr, 1.0, 0, 0);

        cairo_arc(cr, e->x, e->y, r, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    /* 玩家 */
    if (gd->hp > 0) {
        if (gd->invincible) {
            static gboolean toggle = FALSE;
            toggle = !toggle;
            if (toggle) cairo_set_source_rgb(cr, 1, 1, 0);
            else       cairo_set_source_rgb(cr, 1, 0.5, 0);
        }
        else {
            cairo_set_source_rgb(cr, 0, 1, 0);
        }
        cairo_arc(cr, gd->player_x, gd->player_y, PLAYER_SIZE, 0, 2 * M_PI);
        cairo_fill(cr);
    }

    /* 顯示模式/分數資訊 */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);

    char info[128];
    switch (gd->mode) {
    case MODE_DODGE:
        snprintf(info, sizeof(info),
            "Mode: Dodge | HP:%d | Score:%d",
            gd->hp, gd->score);
        break;
    case MODE_TIME_ATTACK:
        snprintf(info, sizeof(info),
            "Mode: Time Attack | HP:%d | Score:%d | Time:%.1f",
            gd->hp, gd->score, gd->time_left);
        break;
    case MODE_CONQUEST:
        snprintf(info, sizeof(info),
            "Mode: Conquest | HP:%d | Score:%d | Kills:%d",
            gd->hp, gd->score, gd->enemies_killed);
        break;
    }
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, info);
}

/* === 鍵盤事件 === */
static gboolean on_key_press(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;
    switch (keyval) {
    case GDK_KEY_w:
    case GDK_KEY_Up:    gd->up_pressed = TRUE;    break;
    case GDK_KEY_s:
    case GDK_KEY_Down:  gd->down_pressed = TRUE;  break;
    case GDK_KEY_a:
    case GDK_KEY_Left:  gd->left_pressed = TRUE;  break;
    case GDK_KEY_d:
    case GDK_KEY_Right: gd->right_pressed = TRUE; break;
    case GDK_KEY_space: gd->space_pressed = TRUE; break;
    default: break;
    }
    return TRUE;
}
static gboolean on_key_release(GtkEventControllerKey* ctrl,
    guint keyval, guint keycode,
    GdkModifierType state, gpointer user_data)
{
    GameData* gd = (GameData*)user_data;
    switch (keyval) {
    case GDK_KEY_w:
    case GDK_KEY_Up:    gd->up_pressed = FALSE;    break;
    case GDK_KEY_s:
    case GDK_KEY_Down:  gd->down_pressed = FALSE;  break;
    case GDK_KEY_a:
    case GDK_KEY_Left:  gd->left_pressed = FALSE;  break;
    case GDK_KEY_d:
    case GDK_KEY_Right: gd->right_pressed = FALSE; break;
    case GDK_KEY_space: gd->space_pressed = FALSE; break;
    default: break;
    }
    return TRUE;
}
