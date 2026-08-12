import pgzrun
import random

WIDTH = 800
HEIGHT = 600
BACKGROUND_COLOR = (255, 255, 255)
começou = False
max_quads = 30

def x_y():
    x = random.randint(40, WIDTH - 40)
    y = random.randint(40, HEIGHT - 40)
    return x, y

quads = []
def criar():
    quad = Actor('amarelo')
    spawn_valido = False
    while spawn_valido == False:
        x,y = x_y()
        quad.pos = x, y 
        collisao = False
        for q in quads:
            if quad.colliderect(q):
                collisao = True  
                break  
        if collisao == False:
            spawn_valido = True  
    quad.vx = random.choice([-1, 1])
    quad.vy = random.choice([-1, 1])
    quad.money = 100
    quads.append(quad)
    


for i in range(max_quads):
    criar()

def draw():
    screen.clear()
    screen.fill(BACKGROUND_COLOR)
    for quad in quads:
        quad.draw()

def update():
               
    for quad in quads:
        if quad.money <=30:
            quad.image = 'cinza'
        if quad.money >= 170:
            quad.image = 'verde'
        if quad.money >= 300:
            quad.image = 'azul'
    for quad in quads:
        quad.x += quad.vx
        quad.y += quad.vy
        if quad.x > WIDTH -20 or quad.x < 20:
            quad.vx = (-1)*quad.vx
        if quad.y > HEIGHT -20 or quad.y < 20:
            quad.vy = (-1)*quad.vy

    for i in range(max_quads):
        for j in range(i + 1, max_quads):
            if quads[i].colliderect(quads[j]):
                if quads[j].money >= 5:
                    quads[j].money -= 5
                    quads[i].money += 5
                hor = abs(quads[i].x - quads[j].x)
                ver = abs(quads[i].y - quads[j].y)
                if hor > ver:
                    quads[i].vx = (-1) * quads[i].vx
                    quads[j].vx = (-1) * quads[j].vx
                elif ver > hor:
                    quads[i].vy = (-1) * quads[i].vy
                    quads[j].vy = (-1) * quads[j].vy
                quads[i].x += quads[i].vx
                quads[i].y += quads[i].vy
                quads[j].x += quads[j].vx
                quads[j].y += quads[j].vy

def on_mouse_down(pos):
    for i in range(max_quads):
        if quads[i].collidepoint(pos):
            x,y = x_y()
            quads[i].pos = x,y

# executar o jogo
pgzrun.go()

