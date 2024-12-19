import pygame
import random

FPS=60
white=(255,255,255)
red=(255,0,0)
green=(0,255,0)
blue=(0,0,255)
yellow=(255,255,0)
width=600
height=700
# 遊戲初始化 and 創建視窗
pygame.init()#把pygame函式做初始化

screen=pygame.display.set_mode((width,height))#設定視窗大小
#公式:pygame.display.set_mode((寬度,高度))

pygame.display.set_caption("DEMO 1")
#pygame.display.set.caption("你的程式")

clock=pygame.time.Clock()#clock函示

#sprite(物件/角色)
#Player
class Player(pygame.sprite.Sprite):
    def __init__(self):#init=初始化
        pygame.sprite.Sprite.__init__(self)#初始化
        self.image=pygame.Surface((50,40))#物體大小
        self.image.fill(green)
        self.rect=self.image.get_rect()#定位
        self.rect.center=(width/2 ,height/2) #置於視窗中心
        self.speedx=6#速度
 
    def update(self):
        #self.rect.y+=5 重力因子(常駐向下)

        #操控
        key_pressed=pygame.key.get_pressed()#利用函式偵測是否按下按鍵
        if key_pressed[pygame.K_RIGHT]:
            self.rect.x+=self.speedx
        elif key_pressed[pygame.K_d]:
            self.rect.x+=self.speedx
        if key_pressed[pygame.K_LEFT]:
            self.rect.x-=self.speedx
        elif key_pressed[pygame.K_a]:
            self.rect.x-=self.speedx
        if key_pressed[pygame.K_UP]:
            self.rect.y-=self.speedx
        elif key_pressed[pygame.K_w]:
            self.rect.y-=self.speedx
        if key_pressed[pygame.K_DOWN]:
            self.rect.y+=self.speedx
        elif key_pressed[pygame.K_s]:
            self.rect.y+=self.speedx

 #設定邊界
        if self.rect.right>width:
            self.rect.right=width
        if self.rect.left<0:
            self.rect.left=0
        if self.rect.top<0:
            self.rect.top=0
        if self.rect.bottom>height:
            self.rect.bottom=height
 #發射導彈
    def fire(self):
        missile = Missile(self.rect.centerx,self.rect.top)
        all_sprites.add(missile)
#Enemy
class Enemy(pygame.sprite.Sprite):
    def __init__(self):
        pygame.sprite.Sprite.__init__(self)
        self.image=pygame.Surface((30,40))
        self.image.fill(red)
        self.rect=self.image.get_rect()
        self.rect.x=random.randrange(0,width-self.rect.width)#讓怪物出現在畫面內
        #(0,width-self.rect.width)=視窗寬度-怪物本身寬度
        self.rect.y=random.randrange(-100,-50)
        self.speedy=random.randrange(3,7)#鉛直速度
        self.speedx=random.randrange(-3,3)#水平速度
    def update(self):
        self.rect.y+=self.speedy 
        self.rect.x+=self.speedx
        if self.rect.top>height:
            self.rect.x=random.randrange(0,width-self.rect.width)#讓怪物出現在畫面內
        #(0,width-self.rect.width)=視窗寬度-怪物本身寬度
            self.rect.y=random.randrange(-100,-50)
            self.speedy=random.randrange(3,6)#鉛直速度
            self.speedx=random.randrange(-2,2)#水平速度
#Missile
class Missile(pygame.sprite.Sprite):
    def __init__(self,x,y):#init=初始化
        pygame.sprite.Sprite.__init__(self)#初始化
        self.image=pygame.Surface((5,15))#物體大小
        self.image.fill(blue)
        self.rect=self.image.get_rect()#定位
        self.rect.centerx= x#取玩家角色的X
        self.rect.bottom=y
        self.speedy=-15#速度
    
    
    def update(self):
      self.rect.y+=self.speedy
      if self.rect.bottom<0:#如果超出視窗就刪除
        self.kill()#將這個sprite群組移除
      

       
        
   
 



       
#角色組別
all_sprites=pygame.sprite.Group()
player=Player()
all_sprites.add(player)


for i in range(0,8):
    enemy=Enemy()#每一次都要呼叫

    all_sprites.add(enemy)


# 遊戲迴圈
running=True
while running:
    clock.tick(FPS)#.tick()=在一秒鐘執行()次
    # 取得輸入
    for event in pygame.event.get():#取得在視窗中的事件(按鍵)
        if event.type==pygame.QUIT:#當偵測到關閉視窗指令時
         running=False#跳出迴圈
        elif event.type==pygame.KEYDOWN:#如果偵測到按鍵按下
            if event.key==pygame.K_SPACE:#如果偵測到的是space鍵
                player.fire()
            
                



    # 更新遊戲
    all_sprites.update()


    # 畫面顯示
    screen.fill(white)#.fill((R,G,B))將視窗填滿為某顏色
    all_sprites.draw(screen)
    pygame.display.update()