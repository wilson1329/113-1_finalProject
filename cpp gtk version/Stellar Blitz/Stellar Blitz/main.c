#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ===================== [ 常数 ] =====================
#define GRID_SIZE     40
#define CELL_SIZE     20
#define MAX_ENEMIES   8  // 最大敌人数量（普通敌人）
#define MAX_MISSILES  10 // 最大飞弹数量
#define PLAYER_RADIUS 15
#define ENEMY_RADIUS  15
#define MISSILE_RADIUS 5
#define LARGE_ENEMY_RADIUS 75  // 大型敌人尺寸（普通敌人5倍）
#define INVINCIBLE_TIME 1  // 无敌时间（秒）
#define BONUS_TIME 10  // 每存活10秒增加10分

// ===================== [ 游戏模式 ] =====================
typedef enum {
    MODE_NONE = 0,
    MODE_MENU,
    MODE_PLAYING
} GameMode;

// ===================== [ 结构定义 ] =====================
typedef struct {
    int x, y;
    int dx, dy;
    gboolean is_active;  // 是否是活动的敌人
    int hit_count;  // 跟踪大型敌人被击中次数
    gboolean is_large;  // 是否为大型敌人
} Enemy;

typedef struct {
    int x, y;
    int dx, dy;
    gboolean active;
} Missile;

typedef struct {
    int x, y;
    int dx, dy;
    int lives;
    int score;
    gboolean invincible;
    guint invincible_timeout;
    guint survival_time;  // 存活时间，用于计算得分
} Player;

// ===================== [ 全局变量 ] =====================
static GtkWidget* window = NULL;
static GtkWidget* canvas = NULL;
static GameMode current_mode = MODE_MENU;
static gboolean game_over = FALSE;
static gboolean paused = FALSE;

// 玩家相关
static Player player;

// 飞弹
static Missile missiles[MAX_MISSILES];

// 普通敌人
static Enemy enemies[MAX_ENEMIES];

// 大型敌人
static Enemy large_enemy[1];  // 只有一个大型敌人

// 敌人生成与飞弹发射定时器
static guint enemy_timeout;
static guint missile_timeout;
static guint survival_time_timeout;  // 存活时间计时器

// ===================== [ 函数声明 ] =====================
static void show_main_menu(void);
static void on_back_to_menu_clicked(GtkButton* button, gpointer user_data);
static void on_quit_clicked(GtkButton* button, gpointer user_data);

static void start_game(void);
static void generate_enemy(void);
static gboolean update_game(gpointer data);
static void move_player(int x, int y);
static void fire_missile(void);
static void check_collisions(void);
static void update_scores(void);

static void draw_game(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer user_data);
static void draw_player(cairo_t* cr);
static void draw_missiles(cairo_t* cr);
static void draw_enemies(cairo_t* cr);
static void draw_large_enemy(cairo_t* cr);
static void draw_score_and_lives(cairo_t* cr);

static gboolean on_key_press(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);

// ===================== [ 游戏开始 ] =====================
static void start_game(void) {
    // 初始化玩家
    player.x = GRID_SIZE / 2 * CELL_SIZE;  // 设置玩家初始位置在视窗下方
    player.y = GRID_SIZE * CELL_SIZE - PLAYER_RADIUS - 10;
    player.lives = 3;
    player.score = 0;
    player.invincible = FALSE;
    player.survival_time = 0;

    // 初始化飞弹
    for (int i = 0; i < MAX_MISSILES; i++) {
        missiles[i].active = FALSE;
    }

    // 初始化普通敌人
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].dx = 0;
        enemies[i].dy = 0;
        enemies[i].is_active = FALSE;
        enemies[i].is_large = FALSE;
        enemies[i].hit_count = 0;
    }

    // 初始化大型敌人
    large_enemy[0].dx = 0;
    large_enemy[0].dy = 0;
    large_enemy[0].is_active = FALSE;
    large_enemy[0].hit_count = 0;
    large_enemy[0].is_large = TRUE;

    // 创建并初始化 canvas 进行游戏显示
    if (canvas == NULL) {
        canvas = gtk_drawing_area_new();
        gtk_widget_set_size_request(canvas, GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(canvas), draw_game, NULL, NULL);
        gtk_window_set_child(GTK_WINDOW(window), canvas);
    }

    // 启动定时器
    g_timeout_add(16, update_game, NULL);  // 更新游戏
    missile_timeout = g_timeout_add(500, update_game, NULL); // 更新飞弹状态
    enemy_timeout = g_timeout_add(500, generate_enemy, NULL); // 每500毫秒生成敌人
    survival_time_timeout = g_timeout_add(1000, update_scores, NULL); // 每秒更新存活时间
}

// 更新游戏状态
static gboolean update_game(gpointer data) {
    if (game_over || paused) return G_SOURCE_CONTINUE;

    // 更新玩家和飞弹
    check_collisions();

    // 更新飞弹位置
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (missiles[i].active) {
            missiles[i].y -= 10; // 飞弹向上移动
            if (missiles[i].y < 0) {
                missiles[i].active = FALSE;  // 超出屏幕后停用
            }
        }
    }

    // 更新敌人位置
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].is_active) {
            enemies[i].x += enemies[i].dx;
            enemies[i].y += enemies[i].dy;

            // 如果敌人碰到边缘，重新生成新的敌人
            if (enemies[i].x < 0 || enemies[i].x > GRID_SIZE * CELL_SIZE || enemies[i].y < 0 || enemies[i].y > GRID_SIZE * CELL_SIZE) {
                enemies[i].dx = 0;
                enemies[i].dy = 0;
                enemies[i].is_active = FALSE;  // 删除敌人
                generate_enemy();  // 生成新的敌人
            }
        }
    }

    // 更新大型敌人
    if (large_enemy[0].is_active) {
        large_enemy[0].x += large_enemy[0].dx;
        large_enemy[0].y += large_enemy[0].dy;

        // 控制大型敌人不超出边界
        if (large_enemy[0].x < 0 || large_enemy[0].x > GRID_SIZE * CELL_SIZE || large_enemy[0].y < 0 || large_enemy[0].y > GRID_SIZE * CELL_SIZE) {
            large_enemy[0].dx = 0;
            large_enemy[0].dy = 0;
        }
    }

    // 确保canvas有效才进行绘制
    if (canvas) {
        gtk_widget_queue_draw(canvas);  // 刷新画面
    }

    return TRUE;
}

// ===================== [ 玩家控制与射击 ] =====================
static void move_player(int x, int y) {
    if (!game_over && !paused) {
        player.x = x;
        player.y = y;
    }
}

static void fire_missile(void) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].active) {
            missiles[i].active = TRUE;
            missiles[i].x = player.x;
            missiles[i].y = player.y - PLAYER_RADIUS;
            break;
        }
    }
}

// ===================== [ 碰撞检测 ] =====================
static void check_collisions(void) {
    // 检查玩家与敌人碰撞
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].is_active) {
            if (abs(player.x - enemies[i].x) < PLAYER_RADIUS + ENEMY_RADIUS &&
                abs(player.y - enemies[i].y) < PLAYER_RADIUS + ENEMY_RADIUS) {
                if (!player.invincible) {
                    player.lives -= 1;
                    player.invincible = TRUE;
                    // 触发闪烁效果
                    if (player.invincible_timeout) {
                        g_source_remove(player.invincible_timeout);
                    }
                    player.invincible_timeout = g_timeout_add_seconds(INVINCIBLE_TIME, (GSourceFunc)g_object_ref, NULL);  // 无敌状态
                }
            }
        }
    }

    // 检查飞弹与敌人的碰撞
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (missiles[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].is_active) {
                    if (abs(missiles[i].x - enemies[j].x) < MISSILE_RADIUS + ENEMY_RADIUS &&
                        abs(missiles[i].y - enemies[j].y) < MISSILE_RADIUS + ENEMY_RADIUS) {
                        missiles[i].active = FALSE;
                        enemies[j].is_active = FALSE;  // 删除敌人
                        player.score += 100;
                        generate_enemy();  // 生成新的敌人
                    }
                }
            }
        }
    }
}

// ===================== [ 更新分数与奖励 ] =====================
static void update_scores(void) {
    if (player.lives > 0) {
        player.survival_time += 1;
        if (player.survival_time % 1 == 0) {
            player.score += 10;  // 每秒增加10分
        }
    }
}

// ===================== [ 绘制游戏内容 ] =====================
static void draw_game(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer user_data) {
    // 背景
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_paint(cr);

    // 绘制玩家
    draw_player(cr);

    // 绘制飞弹
    draw_missiles(cr);

    // 绘制普通敌人
    draw_enemies(cr);

    // 绘制大型敌人
    draw_large_enemy(cr);

    // 绘制分数与血量
    draw_score_and_lives(cr);
}

// 绘制玩家
static void draw_player(cairo_t* cr) {
    if (player.invincible) {
        // 如果是无敌状态，使用闪烁效果（改变颜色）
        double r = (sin(g_get_monotonic_time() / 1000000.0) + 1) / 2;  // 闪烁效果
        cairo_set_source_rgb(cr, r, 0.0, 1.0);  // 闪烁的颜色
    }
    else {
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);  // 正常颜色
    }
    cairo_arc(cr, player.x, player.y, PLAYER_RADIUS, 0, 2 * G_PI);
    cairo_fill(cr);
}

// 绘制飞弹
static void draw_missiles(cairo_t* cr) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (missiles[i].active) {
            cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // 红色
            cairo_arc(cr, missiles[i].x, missiles[i].y, MISSILE_RADIUS, 0, 2 * G_PI);
            cairo_fill(cr);
        }
    }
}

// 绘制普通敌人
static void draw_enemies(cairo_t* cr) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].is_active) {
            cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // 红色敌人
            cairo_arc(cr, enemies[i].x, enemies[i].y, ENEMY_RADIUS, 0, 2 * G_PI);
            cairo_fill(cr);
        }
    }
}

// 绘制大型敌人
static void draw_large_enemy(cairo_t* cr) {
    if (large_enemy[0].is_active) {
        cairo_set_source_rgb(cr, 0.5, 0.0, 0.0);  // 红色大型敌人
        cairo_arc(cr, large_enemy[0].x, large_enemy[0].y, LARGE_ENEMY_RADIUS, 0, 2 * G_PI);
        cairo_fill(cr);
    }
}

// 绘制分数和血量
static void draw_score_and_lives(cairo_t* cr) {
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, "Score: ");
    cairo_show_text(cr, g_strdup_printf("%d", player.score));

    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, "Lives: ");
    cairo_show_text(cr, g_strdup_printf("%d", player.lives));
}

// ===================== [ 生成敌人 ] =====================
static void generate_enemy(void) {
    // 随机生成敌人，最多生成8个敌人
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].is_active) {
            int num_enemies = rand() % 3 + 3; // 随机生成3~5个敌人
            for (int j = 0; j < num_enemies; j++) {
                // 随机生成敌人位置，确保方向指向玩家
                enemies[i].x = player.x + (rand() % 200 - 100);  // 随机生成左右位置
                enemies[i].y = 0;  // 固定在视窗顶部
                enemies[i].dx = (rand() % 3) - 1;  // 随机移动方向
                enemies[i].dy = 1;  // 向下移动

                enemies[i].is_active = TRUE;  // 激活敌人
            }
            break;
        }
    }
}

// ===================== [ 按键事件处理 ] =====================
static gboolean on_key_press(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    if (game_over || paused) return FALSE;

    if (keyval == GDK_KEY_Up) {
        if (player.y > 0) player.y -= 10;
    }
    else if (keyval == GDK_KEY_Down) {
        if (player.y < GRID_SIZE * CELL_SIZE) player.y += 10;
    }
    else if (keyval == GDK_KEY_Left) {
        if (player.x > 0) player.x -= 10;
    }
    else if (keyval == GDK_KEY_Right) {
        if (player.x < GRID_SIZE * CELL_SIZE) player.x += 10;
    }
    else if (keyval == GDK_KEY_space) {
        fire_missile();
    }

    return FALSE;
}

// ===================== [ 返回主菜单 / 退出 ] =====================
static void on_back_to_menu_clicked(GtkButton* button, gpointer user_data) {
    game_over = FALSE;
    paused = FALSE;
    current_mode = MODE_MENU;

    // 返回主菜单
    show_main_menu();
}

static void on_quit_clicked(GtkButton* button, gpointer user_data) {
    GtkApplication* app = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(window)));
    if (app) g_application_quit(G_APPLICATION(app));
}

// ===================== [ 主菜单 ] =====================
static void show_main_menu(void) {
    if (current_mode == MODE_MENU) {
        GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

        GtkWidget* label = gtk_label_new("Select Game Mode");
        gtk_box_append(GTK_BOX(vbox), label);

        GtkWidget* play_btn = gtk_button_new_with_label("Play");
        gtk_box_append(GTK_BOX(vbox), play_btn);
        g_signal_connect(play_btn, "clicked", G_CALLBACK(start_game), NULL);

        GtkWidget* quit_btn = gtk_button_new_with_label("Exit");
        gtk_box_append(GTK_BOX(vbox), quit_btn);
        g_signal_connect(quit_btn, "clicked", G_CALLBACK(on_quit_clicked), NULL);

        gtk_window_set_child(GTK_WINDOW(window), vbox);
    }
}

// ===================== [ 程序入口 ] =====================
static void activate(GtkApplication* app, gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Stellar Blitz");
    gtk_window_set_default_size(GTK_WINDOW(window), GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE);

    GtkEventController* controller = gtk_event_controller_key_new();
    g_signal_connect(controller, "key-pressed", G_CALLBACK(on_key_press), NULL);
    gtk_widget_add_controller(window, controller);

    show_main_menu();
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    GtkApplication* app = gtk_application_new("com.example.StellarBlitz", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
