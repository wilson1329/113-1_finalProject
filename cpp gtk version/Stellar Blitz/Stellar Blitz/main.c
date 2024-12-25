#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* 定義遊戲視窗大小和物件大小 */
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 700
#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 40
#define ENEMY_WIDTH 30
#define ENEMY_HEIGHT 40
#define MISSILE_WIDTH 5
#define MISSILE_HEIGHT 15
#define FPS 60

/* 定義遊戲物件結構，包括位置、大小、速度和狀態 */
typedef struct {
    int x, y;
    int width, height;
    int speed_x, speed_y;
    bool active;
} GameObject;

/* 全域變數，用於保存遊戲狀態和物件 */
GameObject player;
GameObject enemies[8];
GameObject missiles[20];
int missile_count = 0;
int score = 0;
int lives = 3;
bool running = true;
bool invincible = false;
int invincible_timer = 0;
GtkWidget* drawing_area;

/* 初始化或重置遊戲狀態 */
void reset_game();

/* 生成新敵人 */
void spawn_enemy(GameObject* enemy);

/* 更新遊戲狀態，例如物件移動和碰撞檢測 */
void update_game_state();

/* 繪製遊戲畫面，包括玩家、敵人和飛彈 */
void draw_game(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data);

/* 處理鍵盤按下事件，例如移動玩家或發射飛彈 */
void on_key_press(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);
void on_key_release(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);

/* 主遊戲迴圈，定期更新遊戲狀態並刷新畫面 */
gboolean game_loop(gpointer user_data) {
    if (!running) return G_SOURCE_REMOVE;  // 停止定時器
    update_game_state();
    gtk_widget_queue_draw(drawing_area);  // 刷新畫面
    return G_SOURCE_CONTINUE;
}

/* 初始化或重置遊戲狀態，包括玩家位置、敵人生成和分數重置 */
void reset_game() {
    player.x = WINDOW_WIDTH / 2 - PLAYER_WIDTH / 2;
    player.y = WINDOW_HEIGHT - PLAYER_HEIGHT - 10;
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
    player.speed_x = 0;
    player.speed_y = 0;
    player.active = true;

    for (int i = 0; i < 8; i++) {
        spawn_enemy(&enemies[i]);
    }

    missile_count = 0;
    score = 0;
    lives = 3;
    running = true;
}

/* 隨機生成敵人，設置其初始位置和速度 */
void spawn_enemy(GameObject* enemy) {
    enemy->x = rand() % (WINDOW_WIDTH - ENEMY_WIDTH);
    enemy->y = -(rand() % 200);
    enemy->width = ENEMY_WIDTH;
    enemy->height = ENEMY_HEIGHT;
    enemy->speed_x = (rand() % 7) - 3;
    enemy->speed_y = 3 + rand() % 3;
    enemy->active = true;
}

/* 更新遊戲邏輯，包括物件位置更新和碰撞處理 */
void update_game_state() {
    if (!running) return;

    /* 更新玩家位置，限制玩家在視窗內移動 */
    player.x += player.speed_x;
    player.y += player.speed_y;
    if (player.x < 0) player.x = 0;
    if (player.x + player.width > WINDOW_WIDTH) player.x = WINDOW_WIDTH - player.width;
    if (player.y < 0) player.y = 0;
    if (player.y + player.height > WINDOW_HEIGHT) player.y = WINDOW_HEIGHT - player.height;

    /* 更新敵人位置，如果超出視窗則重新生成 */
    for (int i = 0; i < 8; i++) {
        if (enemies[i].active) {
            enemies[i].x += enemies[i].speed_x;
            enemies[i].y += enemies[i].speed_y;
            if (enemies[i].y > WINDOW_HEIGHT) {
                spawn_enemy(&enemies[i]);
                lives--;
                if (lives <= 0) running = false;
            }
        }
    }

    /* 更新飛彈位置，如果超出視窗則失效 */
    for (int i = 0; i < missile_count; i++) {
        if (missiles[i].active) {
            missiles[i].y += missiles[i].speed_y;
            if (missiles[i].y < 0) missiles[i].active = false;
        }
    }

    /* 檢查飛彈和敵人之間的碰撞 */
    for (int i = 0; i < 8; i++) {
        if (!enemies[i].active) continue;

        for (int j = 0; j < missile_count; j++) {
            if (!missiles[j].active) continue;

            if (missiles[j].x < enemies[i].x + enemies[i].width &&
                missiles[j].x + MISSILE_WIDTH > enemies[i].x &&
                missiles[j].y < enemies[i].y + enemies[i].height &&
                missiles[j].y + MISSILE_HEIGHT > enemies[i].y) {
                missiles[j].active = false;
                enemies[i].active = false;
                score += 100;
                spawn_enemy(&enemies[i]);
            }
        }

        /* 檢查玩家和敵人之間的碰撞 */
        if (player.x < enemies[i].x + enemies[i].width &&
            player.x + player.width > enemies[i].x &&
            player.y < enemies[i].y + enemies[i].height &&
            player.y + player.height > enemies[i].y) {
            if (!invincible) {
                enemies[i].active = false;
                lives--;
                invincible = true;
                invincible_timer = 60;  // 60 frames的無敵時間
                if (lives <= 0) running = false;
            }
        }
    }

    /* 處理玩家的無敵狀態計時 */
    if (invincible) {
        invincible_timer--;
        if (invincible_timer <= 0) invincible = false;
    }
}

/* 使用 Cairo 繪製遊戲物件，包括玩家、敵人和飛彈 */
void draw_game(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data) {
    /* 填充背景為白色 */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    /* 繪製玩家 */
    if (invincible && (invincible_timer % 10 < 5)) {
        cairo_set_source_rgb(cr, 1, 1, 0);  // 如果是無敵狀態，讓玩家閃爍
    }
    else {
        cairo_set_source_rgb(cr, 0, 1, 0);  // 玩家是綠色的
    }
    cairo_rectangle(cr, player.x, player.y, player.width, player.height);
    cairo_fill(cr);

    /* 繪製敵人 */
    for (int i = 0; i < 8; i++) {
        if (enemies[i].active) {
            cairo_set_source_rgb(cr, 1, 0, 0);  // 设定敌人颜色为红色
            cairo_rectangle(cr, enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height);
            cairo_fill(cr);
        }
    }

    /* 繪製飛彈 */
    for (int i = 0; i < missile_count; i++) {
        if (missiles[i].active) {
            cairo_set_source_rgb(cr, 0, 0, 1);  // 飞弹颜色为蓝色
            cairo_rectangle(cr, missiles[i].x, missiles[i].y, MISSILE_WIDTH, MISSILE_HEIGHT);
            cairo_fill(cr);
        }
    }

    /* 繪製分數和生命值 */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);

    char buffer[32];
    sprintf(buffer, "Score: %d", score);
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, buffer);

    sprintf(buffer, "Lives: %d", lives);
    cairo_move_to(cr, 10, 50);
    cairo_show_text(cr, buffer);
}

/* 使用 GtkEventControllerKey 處理鍵盤按下輸入 */
void on_key_press(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    switch (keyval) {
    case GDK_KEY_Left:
        player.speed_x = -5;
        break;
    case GDK_KEY_Right:
        player.speed_x = 5;
        break;
    case GDK_KEY_Up:
        player.speed_y = -5;
        break;
    case GDK_KEY_Down:
        player.speed_y = 5;
        break;
    case GDK_KEY_space:
        if (missile_count < 20) {
            missiles[missile_count++] = (GameObject){
                .x = player.x + player.width / 2 - MISSILE_WIDTH / 2,
                .y = player.y,
                .width = MISSILE_WIDTH,
                .height = MISSILE_HEIGHT,
                .speed_x = 0,
                .speed_y = -10,
                .active = true
            };
        }
        break;
    }
}

/* 使用 GtkEventControllerKey 處理鍵盤釋放輸入 */
void on_key_release(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    switch (keyval) {
    case GDK_KEY_Left:
    case GDK_KEY_Right:
        player.speed_x = 0;
        break;
    case GDK_KEY_Up:
    case GDK_KEY_Down:
        player.speed_y = 0;
        break;
    }
}

/* GTK activate 回調函數，負責初始化視窗和遊戲邏輯 */
void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Stellar Blitz");
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, WINDOW_WIDTH, WINDOW_HEIGHT); // 設置繪圖區域大小
    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

    /* 設置繪圖回調函數 */
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_game, NULL, NULL);

    /* 初始化鍵盤事件控制器 */
    GtkEventController* key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_press), NULL);
    g_signal_connect(key_controller, "key-released", G_CALLBACK(on_key_release), NULL);
    gtk_widget_add_controller(drawing_area, key_controller);

    reset_game();  // 初始化遊戲狀態
    gtk_widget_show(window);

    /* 啟動遊戲邏輯定時器 */
    g_timeout_add(1000 / FPS, game_loop, NULL);
}

/* 主函數，初始化 GTK 和遊戲邏輯 */
int main(int argc, char** argv) {
    srand(time(NULL));

    GtkApplication* app = gtk_application_new("com.example.game", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return 0;
}
