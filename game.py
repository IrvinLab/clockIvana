# -*- coding: utf-8 -*-
import pygame, sys, random
import pygame.freetype
import pickle as pkl
import os, subprocess
import time
import datetime
import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

import subprocess

# Raspberry Pi pin configuration:
RST = None     # on the PiOLED this pin isnt used
# Note the following are only used with SPI:
DC = 23
SPI_PORT = 0
SPI_DEVICE = 0

disp = Adafruit_SSD1306.SSD1306_128_64(rst=RST)

# Initialize library.
disp.begin()

# Clear display.
disp.clear()
disp.display()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
width = disp.width
height = disp.height
image = Image.new('1', (width, height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

# Draw a black filled box to clear the image.
draw.rectangle((0,0,width,height), outline=0, fill=0)

# Draw some shapes.
# First define some constants to allow easy resizing of shapes.
padding = -2
top = padding
bottom = height-padding
# Move left to right keeping track of the current x position for drawing shapes.
x = 0
top = 24

# Load default font.
font = ImageFont.load_default()

# Alternatively load a TTF font.  Make sure the .ttf font file is in the same directory as the python script!
# Some other nice fonts to try: http://www.dafont.com/bitmap.php
font = ImageFont.truetype('fonts/cube.ttf', 24)
draw.rectangle((0,0,width,height), outline=0, fill=0)
draw.text((x, top),       "Irvin Lab",  font=font, fill=255)
disp.image(image)
disp.display()
time.sleep(.1)

pygame.init()
sc = pygame.display.set_mode((150, 150))
pygame.display.set_caption("game")
clock = pygame.time.Clock()
pygame.display.update()


disp.clear()
disp.display()
font = ImageFont.load_default()
x = 0   # x-координата
top = 0 # y-координата
command = '' # Набираемая строка в интерпретаторе
terminal = []
n = 0
terminal.append(n)
terminal[n] = '' # массив комманд
setScreen = 0 # конечная y-координата
noOutput = 0
clockIvana = 1 # 1 - отображать часы, 0 - интерпретатор
setFont = 0
setSize = 10
myCeil=[] # Ячейки памяти программы
for n in range(30000):
    myCeil.append(n)
    myCeil[n] = 0
n = 0
currentCeil = 0 # Текущая ячейка

def visibleScreen(myString): # Функция отображения листа/экрана
    global n, terminal
    i = 0
    resultString = ''
    for i in range(n):
        resultString += str(terminal[i]) + '\n'
    
    resultString += myString
    draw.rectangle((0,0,width,height), outline=0, fill=0)
    draw.text((x, top), resultString,  font=font, fill=255)
    disp.image(image)
    disp.display()
    time.sleep(.1)

def pressKey(sumbol): # фУНКЦИЯ НАЖАТИЙ КЛАВИШ
    global command, top, setScreen
    #top = setScreen
    command += sumbol    

def compilator(argum): # Псевдокомпилятор, запускает последовательность команд из оперативной памяти или из файла
    global terminal, command, top, x, setScreen, n, noOutput,font
    noOutput = 1
    if argum == '0': 
        g = 0
        tmpMass = []
        for c in range(len(terminal[:-1])):
            tmpMass.append(c)
            tmpMass[c] = terminal[c]
        while g != len(tmpMass[0:-1]):
            if tmpMass[g] == 'run': 
                computing('Please clear display')
                computing('or save this program')
                break    
            computing(tmpMass[g])
            g+=1
    else:
        print("starting")
        tmpMass = []
        try:
            fileName = 'files/' + argum
            del tmpMass [:]
            top = 0
            i = 0
            g = 0
            tmpMass.append(g)
            f = open(fileName,'r')
            for line in f:
                    tmpMass[g] = line[:-1]
                    g+= 1
                    tmpMass.append(g)
            f.close()
            tmpMass.append(g+1)
            g = 0
            while g != len(tmpMass[0:]):
                print(tmpMass[g])
                computing(tmpMass[g])
                g+=1
        except IOError as e:
           terminal[n] = 'Can not find the File' 
           n += 1
           terminal.append(n)
    noOutput = 0           

def brainfuck(myStr): #Это брейнфак
    global myCeil, currentCeil,n
    i = 0
    while (i < len(myStr)):
        if myStr[i] == 'q': myCeil[currentCeil] += myCeil[currentCeil]
        if myStr[i] == 'w': myCeil[currentCeil] += myCeil[currentCeil]
        if myStr[i] == 'e': 
            if currentCeil > 0: currentCeil-=1
        if myStr[i] == 'r': 
            if currentCeil < 30000: currentCeil+=1
        if myStr[i] == 't':
            terminal[n] = myCeil[currentCeil]
            n += 1
            terminal.append(n)
        
                     
        i+=1
    a=0
    for a in range(30000):
        myCeil[a] = 0
    a = 0
    currentCeil = 0
    
def computing(comLine): # Функция-интерпретатор команд
    global command, top, n, terminal, setScreen, noOutput, setFont, setSize, font
    terminal[n] = comLine
    n += 1
    terminal.append(n)
    
    #if comLine[0:3] == 'run': # Запуск компилятора
    #    if len(comLine) == 3: compilator('0')
    #    else: compilator(comLine[4:]); print("external programm")
    
    if comLine == 'run': # запустить brainfuck интерпретатор
        brainfuck(terminal[n-2])
        n+=1
        terminal.append(n)
        
    elif comLine == 'help':
        terminal[n] = 'My brainfuck'
        n += 1
        terminal.append(n)
        terminal[n] = 'q = <'
        n += 1
        terminal.append(n)
        terminal[n] = 'w = >'
        n += 1
        terminal.append(n)
        terminal[n] = 'e = -'
        n += 1
        terminal.append(n)
        terminal[n] = 'r = +'
        n += 1
        terminal.append(n)
        terminal[n] = 't = .'
        n += 1
        terminal.append(n)
        terminal[n] = 'y = ,'
        n += 1
        terminal.append(n)
        terminal[n] = 'u = ['
        n += 1
        terminal.append(n)
        terminal[n] = 'i = ]'
        n += 1
        terminal.append(n)
    elif comLine == 'time': # Вывод времени
        now = datetime.datetime.now()
        terminal[n] = now.strftime("%d-%m-%Y %H:%M:%S") 
        n += 1
        terminal.append(n)
    
    elif comLine == 'clear': # Очистка экрана 
        del terminal [:]
        top = 0
        n = 0
        terminal.append(n)
    
    elif comLine[0:2] == 'ls': # Просмотр папки
        fileName = 'files/'+comLine[3:]
        terminal[n] = os.listdir(fileName)
        n += 1
        terminal.append(n)
        
    elif comLine[0:2] == 'rm':
        fileName = 'files/' + comLine[3:]
        osCommand = 'rm ' + fileName
        os.system(osCommand)
        
    elif comLine[0:4] == 'open': # Открытие файла
        try:
            fileName = 'files/' + comLine[5:]
            del terminal [:]
            top = 0
            i = 0
            n = 0
            terminal.append(n)
            f = open(fileName,'r')
            for line in f:
                    terminal[n] = line[:-1]
                    n+= 1
                    terminal.append(n)
            f.close()
        except IOError as e:
           terminal[n] = 'File not found' 
           n += 1
           terminal.append(n)
           
                   
    elif comLine[0:4] == 'save': # Сохранение файла
        fileName = comLine[5:]
        f = open(fileName, 'w')     
        i = 0
        for i in range(n-1):
            f.write(str(terminal[i] + '\n'))
        f.close()    
        osCommand = 'mv ' + fileName + ' files/'
        os.system(osCommand)
    
    elif comLine[0:7] == 'setfont': # Устанавливаем шрифт
        if comLine == 'setfont default' or comLine == 'setfont 0':
            font = ImageFont.load_default()
        else: 
            setFont = 'fonts/' + str(comLine[8:])
            font = ImageFont.truetype(setFont, setSize)
    
    elif comLine[0:7] == 'setsize': # Устанавливаем размер шрифта
        setSize = int(comLine[8:])
        
    else: 
        if noOutput == 1: # Если работает компилятор - не печатаем лишний текст
           terminal[n-1] = ''

                
    if n >= 4: 
        top -= 20
        setScreen = top
    command = ''                

def ivanClock(): # Функция часов, активна при включении
    x = 0
    top = 0
    now = datetime.datetime.now()
    str1 = now.strftime("%d-%m-%Y") 
    str2 = now.strftime("%H:%M:%S") 
    font = ImageFont.truetype('fonts/cube.ttf', 20)
    draw.rectangle((0,0,width,height), outline=0, fill=0)
    draw.text((x, top+15),str1,  font=font, fill=255)
    font = ImageFont.truetype('fonts/cube.ttf', 25)
    draw.text((x+7, top+35),str2,  font=font, fill=255)
    disp.image(image)
    disp.display()
    time.sleep(.1)            
    
while True:

    clock.tick(30) 
    for i in pygame.event.get():
        if i.type == pygame.QUIT:
            exit()
            
        elif i.type == pygame.KEYDOWN:
            
            if i.key == pygame.K_LEFT:
                if x<0: x += 10
                
            elif i.key == pygame.K_RETURN:
                computing(command) 
                
            elif i.key == pygame.K_RIGHT:
                x -= 10
                
            elif i.key == pygame.K_UP:
                top += 10
                
            elif i.key == pygame.K_DOWN: 
                top -= 10
            
            elif i.key == pygame.K_BACKSPACE:
                command = command[:len(command)-1]
                
            elif i.key == pygame.K_SPACE:
                pressKey(' ')
            
            elif i.key == pygame.K_ESCAPE: # Клавишей ESC включаем/выключаем часы/интерпретатор
                if clockIvana == 1: 
                    clockIvana = 0
                    x = 0
                    top = 0
                    computing('clear')
                elif clockIvana == 0: clockIvana = 1
                    
            elif i.key == pygame.K_1:
                pressKey('1')
            elif i.key == pygame.K_2:
                pressKey('2')
            elif i.key == pygame.K_3:
                pressKey('3')
            elif i.key == pygame.K_4:
                pressKey('4')
            elif i.key == pygame.K_5:
                pressKey('5')
            elif i.key == pygame.K_6:
                pressKey('6')
            elif i.key == pygame.K_7:
                pressKey('7')
            elif i.key == pygame.K_8:
                pressKey('8')
            elif i.key == pygame.K_9:
                pressKey('9')
            elif i.key == pygame.K_0:
                pressKey('0')                                   
            elif i.key == pygame.K_1 and i.key == pygame.K_LSHIFT:
                pressKey('!')
            elif i.key == pygame.K_HASH:
                pressKey('#')
            elif i.key == pygame.K_DOLLAR:
                pressKey('$')
            elif i.key == pygame.K_AMPERSAND:
                pressKey('&')
            elif i.key == pygame.K_LEFTPAREN:
                pressKey('(')
            elif i.key == pygame.K_RIGHTPAREN:
                pressKey(')')
            elif i.key == pygame.K_ASTERISK:
                pressKey('*')
            elif i.key == pygame.K_PLUS:
                pressKey('+')
            elif i.key == pygame.K_COMMA:
                pressKey(',')
            elif i.key == pygame.K_MINUS:
                pressKey('-')
            elif i.key == pygame.K_PERIOD:
                pressKey('.')
            elif i.key == pygame.K_SLASH:
                pressKey('/')
            elif i.key == pygame.K_COLON:
                pressKey(':')
            elif i.key == pygame.K_SEMICOLON:
                pressKey(';')
            elif i.key == pygame.K_EQUALS:
                pressKey('=')
            elif i.key == pygame.K_QUESTION:
                pressKey('?')
            elif i.key == pygame.K_AT:
                pressKey('@')
            elif i.key == pygame.K_LEFTBRACKET:
                pressKey('[')
            elif i.key == pygame.K_RIGHTBRACKET:
                pressKey(']')
            elif i.key == pygame.K_CARET:
                pressKey('^')
            elif i.key == pygame.K_UNDERSCORE:
                pressKey('_')       
            elif i.key == pygame.K_q:
                pressKey('q')
            elif i.key == pygame.K_w:
                pressKey('w')
            elif i.key == pygame.K_e:
                pressKey('e')
            elif i.key == pygame.K_r:
                pressKey('r')
            elif i.key == pygame.K_t:
                pressKey('t')
            elif i.key == pygame.K_y:
                pressKey('y')
            elif i.key == pygame.K_u:
                pressKey('u')
            elif i.key == pygame.K_i:
                pressKey('i')
            elif i.key == pygame.K_o:
                pressKey('o')
            elif i.key == pygame.K_p:
                pressKey('p')
            elif i.key == pygame.K_a:
                pressKey('a')
            elif i.key == pygame.K_s:
                pressKey('s')
            elif i.key == pygame.K_d:
                pressKey('d')
            elif i.key == pygame.K_f:
                pressKey('f')
            elif i.key == pygame.K_g:
                pressKey('g')
            elif i.key == pygame.K_h:
                pressKey('h')
            elif i.key == pygame.K_j:
                pressKey('j')
            elif i.key == pygame.K_k:
                pressKey('k')
            elif i.key == pygame.K_l:
                pressKey('l')
            elif i.key == pygame.K_z:
                pressKey('z')
            elif i.key == pygame.K_x:
                pressKey('x')
            elif i.key == pygame.K_c:
                pressKey('c')
            elif i.key == pygame.K_v:
                pressKey('v')
            elif i.key == pygame.K_b:
                pressKey('b')
            elif i.key == pygame.K_n:
                pressKey('n')
            elif i.key == pygame.K_m:
                pressKey('m')

    
    if clockIvana == 0: visibleScreen(command) 
    elif clockIvana == 1: ivanClock()
