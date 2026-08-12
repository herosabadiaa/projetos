import pgzrun
import sys

HEIGHT = 600
WIDTH = 800
max_cube = 10
cubes = []

for i in range(max_cube):
    cube = Actor("azul.png")
    cube.pos = WIDTH/2 ,1*HEIGHT/8
    cubes.append(cube)


def update():
    cubes[1].y += 1
    if cubes[1].x > 7*WIDTH/8:
        if cubes[1].y > HEIGHT:
            sys.exit()
    elif cubes[1].y >= HEIGHT -100:
        cubes[1].y = HEIGHT -100
        cubes[1].x += 1


def draw():
    screen.clear()
    cubes[1].draw()

pgzrun.go()