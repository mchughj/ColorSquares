# vim ft:python

# I needed to pick some colors that I really liked so I used this driver program, along with the .ino 
# in this directory, to quickly go through the colors and construct a final color array that I could
# copy and paste into my actual .ino.  
#
# This program pops up a pygame window and then allows you to control the colors by sending 
# serial communications to an attached device.  Use the arrow keys to cycle up and down the
# color.  Hit <SPACE> to record the current color and 'l' to list all of the current colors
# in a C array.

import serial
import sys
import time
import pygame

# Pygame.  Just the easiest way I know to use the arrow keys and keyboard controls
# to quickly control what color is displayed.
from pygame.locals import *

serial_speed = 9600
serial_port = 'com18'

def ColorWheel(wheelPos):
  if wheelPos < 85:
     return (wheelPos * 3, 255 - wheelPos * 3, 0)
  elif wheelPos < 170:
     wheelPos -= 85;
     return (255 - wheelPos * 3, 0, wheelPos * 3)
  else:
     wheelPos -= 170;
     return (0, wheelPos * 3, 255 - wheelPos * 3)

# Super simple harness to drive the colors.  This 
def keyboardControlProgram():
    print "Establishing the communication with the arduino"

    ser = serial.Serial(serial_port, serial_speed, timeout=1)

    print "Waiting for arduino bootloader to finish"
    time.sleep(2)

    pygame.display.update()

    c = 0
    colors = [] 
    while 1:
        (r,g,b) = ColorWheel(c)
        ser.write( "%d %d %d\n" % ( r,g,b ) )
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                keys = pygame.key.get_pressed()
                if keys[pygame.K_q]:
                    print "Done"
                    sys.exit(1)
                if keys[pygame.K_SPACE]:
                    print "Recording color"
                    colors.append( ( r,g,b ) )
                if keys[pygame.K_l]:
                    print "uint32_t colors[] = {"
                    for e in colors:
                        color = (e[0] << 16 ) + (e[1] << 8 ) + e[2]
                        print "  " + str(hex(color)) + ", "
                    print "};"

        keys = pygame.key.get_pressed()
        if keys[pygame.K_q]:
            print "Done"
            sys.exit(1)
        if keys[pygame.K_UP]:
            c = c + 1
            if c > 255:
               c = 1
            (r,g,b) = ColorWheel(c)
            print "Increasing color; colorWheel: %d, (%d,%d,%d)" % ( c, r,g,b)
            time.sleep(0.25)
        if keys[pygame.K_DOWN]:
            print "Decreasing color"
            c = c - 1
            if c < 0:
               c = 254
            (r,g,b) = ColorWheel(c)
            print "Increasing color; colorWheel: %d, (%d,%d,%d)" % ( c, r,g,b)
            time.sleep(0.25)



if __name__ == "__main__":
   pygame.init() 
   window = pygame.display.set_mode((400, 300))
   keyboardControlProgram()

