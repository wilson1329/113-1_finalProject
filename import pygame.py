import pygame
import random

FPS = 60
white = (255, 255, 255)
red = (255, 0, 0)
green = (0, 255, 0)
blue = (0, 0, 255)
yellow = (255, 255, 0)
width = 600
height = 700

# 遊戲初始化 and 創建視窗
pygame.init()  # 初始化 pygame 函數
screen = pygame.display.set_mode((width, height))  # 設定視窗大小
pygame.display.set_caption("DEMO 1")  # 設定視窗標題
clock = pygame.time.Clock()  # 建立時鐘物件

# 分數和生命
score = 0  # 初始分數
lives = 3  # 玩家初始生命
invincible = False  # 是否無敵狀態
invincible_start_time = 0  # 無敵開始時間
invincible_duration = 1000  # 無敵時間（毫秒）

# sprite(物件/角色)
# Player
class Player(pygame.sprite.Sprite):
    def __init__(self):  # 初始化玩家
        pygame.sprite.Sprite.__init__(self)
        self.image = pygame.Surface((50, 40))  # 設定玩家角色的大小
        self.image.fill(green)  # 填充顏色
        self.rect = self.image.get_rect()  # 獲取角色的矩形範圍
        self.rect.center = (width / 2, height / 2)  # 設定初始位置
        self.speedx = 6  # 設定水平速度
        self.health = lives  # 玩家血量

    def update(self):  # 更新玩家狀態
        global invincible

        key_pressed = pygame.key.get_pressed()  # 偵測按鍵輸入
        if key_pressed[pygame.K_RIGHT] or key_pressed[pygame.K_d]:  # 向右移動
            self.rect.x += self.speedx
        if key_pressed[pygame.K_LEFT] or key_pressed[pygame.K_a]:  # 向左移動
            self.rect.x -= self.speedx
        if key_pressed[pygame.K_UP] or key_pressed[pygame.K_w]:  # 向上移動
            self.rect.y -= self.speedx
        if key_pressed[pygame.K_DOWN] or key_pressed[pygame.K_s]:  # 向下移動
            self.rect.y += self.speedx

        # 邊界限制
        if self.rect.right > width:  # 防止超出右邊界
            self.rect.right = width
        if self.rect.left < 0:  # 防止超出左邊界
            self.rect.left = 0
        if self.rect.top < 0:  # 防止超出上邊界
            self.rect.top = 0
        if self.rect.bottom > height:  # 防止超出下邊界
            self.rect.bottom = height

        # 無敵閃爍效果
        if invincible:
            current_time = pygame.time.get_ticks()
            if (current_time // 200) % 2 == 0:  # 閃爍效果
                self.image.set_alpha(100)
            else:
                self.image.set_alpha(255)
            if current_time - invincible_start_time > invincible_duration:  # 結束無敵狀態
                invincible = False
                self.image.set_alpha(255)

    def fire(self):  # 玩家發射導彈
        missile = Missile(self.rect.centerx, self.rect.top)  # 建立導彈物件
        all_sprites.add(missile)  # 將導彈加入全體物件群組
        missiles.add(missile)  # 將導彈加入導彈群組

# Enemy
class Enemy(pygame.sprite.Sprite):
    def __init__(self):  # 初始化敵人
        pygame.sprite.Sprite.__init__(self)
        self.image = pygame.Surface((30, 40))  # 設定敵人大小
        self.image.fill(red)  # 填充顏色
        self.rect = self.image.get_rect()  # 獲取敵人的矩形範圍
        self.rect.x = random.randrange(0, width - self.rect.width)  # 設定水平位置
        self.rect.y = random.randrange(-100, -50)  # 設定垂直位置
        self.speedy = random.randrange(3, 7)  # 垂直速度
        self.speedx = random.randrange(-3, 3)  # 水平速度

    def update(self):  # 更新敵人狀態
        self.rect.y += self.speedy  # 垂直移動
        self.rect.x += self.speedx  # 水平移動
        if self.rect.top > height:  # 如果超出視窗下方，重新生成
            self.rect.x = random.randrange(0, width - self.rect.width)
            self.rect.y = random.randrange(-100, -50)
            self.speedy = random.randrange(3, 6)
            self.speedx = random.randrange(-2, 2)

# Missile
class Missile(pygame.sprite.Sprite):
    def __init__(self, x, y):  # 初始化導彈
        pygame.sprite.Sprite.__init__(self)
        self.image = pygame.Surface((5, 15))  # 設定導彈大小
        self.image.fill(blue)  # 填充顏色
        self.rect = self.image.get_rect()  # 獲取導彈矩形範圍
        self.rect.centerx = x  # 導彈水平中心與玩家對齊
        self.rect.bottom = y  # 導彈底部對齊玩家頂部
        self.speedy = -15  # 導彈垂直速度（向上）

    def update(self):  # 更新導彈狀態
        self.rect.y += self.speedy  # 向上移動
        if self.rect.bottom < 0:  # 如果超出視窗上方，刪除導彈
            self.kill()

# sprite群組
all_sprites = pygame.sprite.Group()  # 全體物件群組
missiles = pygame.sprite.Group()  # 導彈群組
enemies = pygame.sprite.Group()  # 敵人物件群組
player = Player()  # 建立玩家物件
all_sprites.add(player)  # 將玩家加入全體物件群組

for i in range(8):  # 生成 8 個敵人
    enemy = Enemy()
    all_sprites.add(enemy)
    enemies.add(enemy)

# 遊戲迴圈
running = True
start_time = pygame.time.get_ticks()  # 記錄遊戲開始時間

while running:
    clock.tick(FPS)  # 控制遊戲執行速度

    # 取得輸入
    for event in pygame.event.get():  # 處理事件
        if event.type == pygame.QUIT:  # 點擊關閉視窗
            running = False
        elif event.type == pygame.KEYDOWN:  # 按鍵事件
            if event.key == pygame.K_SPACE:  # 空白鍵發射導彈
                player.fire()

    # 更新遊戲
    all_sprites.update()  # 更新所有物件

    # 檢查碰撞
    if not invincible:  # 如果玩家不在無敵狀態
        hits = pygame.sprite.spritecollide(player, enemies, True)  # 檢測玩家與敵人碰撞
        for hit in hits:
            player.health -= 1  # 減少玩家血量
            enemy = Enemy()  # 生成新敵人
            all_sprites.add(enemy)
            enemies.add(enemy)
            invincible = True  # 啟動無敵狀態
            invincible_start_time = pygame.time.get_ticks()  # 記錄無敵開始時間
            if player.health <= 0:  # 如果血量歸零
                running = False

    missile_hits = pygame.sprite.groupcollide(enemies, missiles, True, True)  # 檢測導彈與敵人碰撞
    for hit in missile_hits:
        score += 100  # 擊敗敵人加分
        enemy = Enemy()  # 生成新敵人
        all_sprites.add(enemy)
        enemies.add(enemy)

    # 計算分數
    current_time = pygame.time.get_ticks()
    if (current_time - start_time) >= 1000:  # 每秒加分
        score += 10
        start_time = current_time

    # 畫面顯示
    screen.fill(white)  # 填充背景顏色
    all_sprites.draw(screen)  # 繪製所有物件

    # 繪製分數和血量
    font = pygame.font.SysFont(None, 36)  # 設定字體
    score_text = font.render(f"Score: {score}", True, (0, 0, 0))  # 分數文字
    lives_text = font.render(f"Lives: {player.health}", True, (0, 0, 0))  # 血量文字
    screen.blit(score_text, (10, 10))  # 顯示分數
    screen.blit(lives_text, (10, 50))  # 顯示血量

    pygame.display.update()  # 更新畫面

pygame.quit()