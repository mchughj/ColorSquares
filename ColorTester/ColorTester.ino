// vim: ts=4 sw=4 ft=cpp ai cindent

#include <Adafruit_NeoPixel.h>

#define COUNT 9
#define PIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(COUNT, PIN, NEO_GRB + NEO_KHZ800);

// Simple test program that allows me to control the pixels using three integers passed via serial communication. 

void setup() {
  strip.begin();
  strip.setBrightness(200);
  strip.show(); 
  Serial.begin(9600);
}

void loop() {
    if( Serial.available() > 0 ) {
        uint32_t nextRed = Serial.parseInt();
        uint32_t nextGreen = Serial.parseInt();
        uint32_t nextBlue = Serial.parseInt();

        uint32_t nextColor = strip.Color(nextRed, nextGreen, nextBlue );
        for( int i = 0; i < COUNT; i++ ) { 
            strip.setPixelColor(i,  nextColor);
        }
        strip.show();
    }
}

