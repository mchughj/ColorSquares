// vim: ts=4 sw=4 ft=cpp ai cindent

// This is a 3x3 light squares with a tilt-a-whirl.  The intent of this project was just 
// to have a bit of fun and keep myself occupied while the family watched the 2014 Novemeber 
// soccer game.  
//
// I saw a project on my feed that showed color squares.  It looked pretty neat and I decided to 
// try my own version of the same project.  Products used:
//
// The light source:
//   9 Neopixels - https://www.adafruit.com/products/1461 
//   $3.75
//
// The breadboard:
//   Solderable mini breadboard - https://www.sparkfun.com/products/12702 - 
//   $2.95
//
// Main processor
//   Arduino Pro mini - https://www.sparkfun.com/products/11113
//   $9.95
//   I would have used an AT Tiny, in the form of an Adafruit Trinket, since it is cheaper, but 
//   this project required the 32k memory of the full Atmega328.  
//
// Input
//   Tilt-A-Whirl
//   https://www.sparkfun.com/products/12011
//   $5.95
//   I was going to go with buttons, then with a capacative touch sensor, but since
//   this was going to sit on a desk I really thought that being able to flip the box would
//   be a fun way of changing the state.  
//
// This build is a 3x3 pixel grid with some really nice colors.  I used the orientation of the entire
// installation to decide the specific state of the project.  If you flip the
// project onto its sides then the tilt-a-whirl will choose a new state.

// The light squares are oriented with the following pixel locations:
// 0,  1,  2
// 5,  4,  3
// 6,  7,  8


#include <Adafruit_NeoPixel.h>

#define COUNT 9

// The pins that are used in this build.  The Neopixel pin controls the squares and the tilt sensor uses two 
// pins to capture the four states associated with where the tilt is.
#define NEOPIXELPIN 6
#define TILTPIN1 8
#define TILTPIN2 9

Adafruit_NeoPixel strip = Adafruit_NeoPixel(COUNT, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

// There are a couple of different light programs in this file - each is a
// 'state'.  All of the programs have at their core a transition which moves each individual square from a
// priorColor to a nextColor.  The current color is captured in currentColor and it is somewhere between the
// prior and next.   
//
// The transition time to move from prior to next is controled by
// transitionCount, transitionTime, and transitionSpeed.  transitionCount is
// the total 'time' to move that particular cell from prior to next.
// transitionTime is the total 'time' remaining.  Finally, transitionSpeed is
// how quickly 'time' passes for this cell.  Time here is all relative to the 
// particular program and how long we delay between frames.
// 
uint32_t nextColor[COUNT];
uint32_t currentColor[COUNT];
uint32_t priorColor[COUNT];
uint8_t transitionCount[COUNT];
float transitionTime[COUNT];
float transitionSpeed[COUNT];

// I used the accompanying color program to choose the colors that I liked the most.  These appear below.
#define COLORS_ARRAY_LENGTH 17
uint32_t colors[] = {
    0xff00,
    0x42bd00,
    0x639c00,
    0x936c00,
    0xb74800,
    0xde2100,
    0xff0000,
    0xcf0030,
    0xb4004b,
    0x84007b,
    0x60009f,
    0x3900c6,
    0x3fc,
    0x30cf,
    0x6c93,
    0xa857,
    0xe41b,
};

// The different states of my project.  The transitions and current state are determined by the tilt-a-whirl.
enum State {
    RANDOM_INDIVIDUAL,
    RANDOM_SWEEP,
    RANDOM_WALK,
    OFF
};

// The current state.
State state;

// When the state is using a walk then this captures the current position of the walk.
uint8_t walkPosition;

// Initialization routines
void initRandomIndividual();
void initRandomSweep();
void initOff();

void initNextState( uint8_t s ) { 
    if( s == RANDOM_INDIVIDUAL ) { 
        initRandomIndividual();
    } else if( s == RANDOM_SWEEP ) { 
        initRandomSweep();
    } else if( s == RANDOM_WALK ) { 
        initRandomSweep();
        walkPosition = 0;
    } else if( s == OFF ) { 
        initOff();
    }
}


void setup() {
    strip.begin();
    strip.show(); 
    strip.setBrightness(255);
    Serial.begin(9600);

    state = RANDOM_INDIVIDUAL;
    initNextState( state );
}

uint8_t getTilt() {
    return digitalRead( TILTPIN1 ) << 1 | digitalRead( TILTPIN2 );
}

uint8_t convertTiltToState( uint8_t tilt ) 
{ 
    switch( tilt ) {
        case 0: return RANDOM_INDIVIDUAL;    
        case 1: return RANDOM_SWEEP;
        case 2: return RANDOM_WALK;
        case 3: return OFF;
        default:
                Serial.print( "Unexpected state " );
                Serial.println( tilt );
                return OFF;
    }
}

uint32_t chooseRandomColor() { 
    int i = random(COLORS_ARRAY_LENGTH);
    return colors[i];
}

void initRandomIndividual() {
    for( int i = 0; i < COUNT; i++ ) { 
        uint32_t c = chooseRandomColor();
        nextColor[i] = c;
        priorColor[i] = c;
        currentColor[i] = c;
        transitionCount[i] = 0;
        transitionTime[i] = 0.0;
    }
}

void initRandomSweep() {
    uint32_t c = chooseRandomColor();
    for( int i = 0; i < COUNT; i++ ) { 
        nextColor[i] = c;
        priorColor[i] = c;
        currentColor[i] = c;
        transitionCount[i] = 0;
        transitionTime[i] = 0.0;
    }
}


void initOff() {
    for( int i = 0; i < COUNT; i++ ) { 
        nextColor[i] = 0x101010;
        priorColor[i] = 0;
        currentColor[i] = 0;
        transitionCount[i] = 10;
        transitionTime[i] = 0.0;
    }
}

uint32_t chooseNewRandomColor() { 
    uint32_t c = chooseRandomColor();

    while(1) { 
        int found = 0;
        for( int i = 0; i < COUNT; i++ ) { 
            if( c == nextColor[i] ) { 
                found = 1;
            }
        }
        if( found == 0 ) { 
            break;
        }
        c = chooseRandomColor();
    }

    return c;
}

void chooseNextWalk() {
    uint32_t c = chooseNewRandomColor();
    int t = random(4);

#ifdef DEBUG
    Serial.print( "Choosing next walk; walkPosition: " );
    Serial.print( walkPosition );
    Serial.print( ", color: ");
    Serial.print( c );
    Serial.print( ", direction: " );
    Serial.println( t );
#endif

    if( walkPosition == 0 ) { 
        if( t == 0 || t == 1 ) { 
            walkPosition = 1;
        } else { 
            walkPosition = 5;
        }
    } else if( walkPosition == 1 ) { 
        if( t == 0 ) { 
            walkPosition = 0;
        } else if( t == 1 ) { 
            walkPosition = 2;
        } else if( t == 2 ) { 
            walkPosition = 4;
        } else {
            return;
        }
    } else if( walkPosition == 2 ) { 
        if( t == 0 || t == 1 ) { 
            walkPosition = 1;
        } else { 
            walkPosition = 3;
        }
    } else if( walkPosition == 3 ) { 
        if( t == 0 ) { 
            walkPosition = 2;
        } else if( t == 1 ) { 
            walkPosition = 4;
        } else if( t == 2 ) { 
            walkPosition = 8;
        } else {
            return;
        }
    } else if( walkPosition == 4 ) { 
        if( t == 0 ) { 
            walkPosition = 1;
        } else if( t == 1 ) { 
            walkPosition = 2;
        } else if( t == 2 ) { 
            walkPosition = 5;
        } else if( t == 3 ) { 
            walkPosition = 7;
        }
    } else if( walkPosition == 5 ) { 
        if( t == 0 ) { 
            walkPosition = 0;
        } else if( t == 1 ) { 
            walkPosition = 4;
        } else if( t == 2 ) { 
            walkPosition = 6;
        } else {
            return;
        }
    } else if( walkPosition == 6 ) { 
        if( t == 0 || t == 1 ) { 
            walkPosition = 5;
        } else { 
            walkPosition = 7;
        }
    } else if( walkPosition == 7 ) { 
        if( t == 0 ) { 
            walkPosition = 6;
        } else if( t == 1 ) { 
            walkPosition = 4;
        } else if( t == 2 ) { 
            walkPosition = 8;
        } else {
            return;
        }
    } else if( walkPosition == 8 ) { 
        if( t == 0 || t == 1 ) { 
            walkPosition = 3;
        } else { 
            walkPosition = 7;
        }
    }

#ifdef DEBUG
    Serial.print( "Walk determined; walkPosition: " );
    Serial.println( walkPosition );
#endif

    transitionCount[walkPosition] = 2;
    transitionTime[walkPosition] = transitionCount[walkPosition];
    priorColor[walkPosition] = currentColor[walkPosition];
    nextColor[walkPosition] = c;
    transitionSpeed[walkPosition] = 1;
}

// Transition arrays for the sweep program.  
//
float sweepSpeedCorner1[] = {
    1,    0.8,   0.6,
    0.3,  0.6,   0.8,
    0.6,  0.3,   0.4
};

float sweepSpeedCorner2[] = {
    0.6,  0.8,  1,    
    0.8,  0.6,  0.3,  
    0.4,  0.3,  0.6  
};
float sweepSpeedTopToBottom[] = {
    1,    1,     1,
    0.6,  0.6,   0.6,
    0.3,  0.3,   0.3
};
float sweepSpeedBottomToTop[] = {
    0.3,  0.3,   0.3,
    0.6,  0.6,   0.6,
    1,    1,     1,
};
float sweepSpeedLeftToRight[] = {
    1,   0.6,   0.3,
    0.3,  0.6,  1,
    1,   0.6,   0.3
};
float sweepSpeedRightToLeft[] = {
    0.3,   0.6,  1,   
    1,     0.6,  0.3, 
    0.3,   0.6,  1   
};
float sweepSpeedCenterOut[] = {
    0.2,   0.5,  0.2,   
    0.5,     1,  0.5, 
    0.2,   0.5,  0.2   
};


void chooseNewSweepRandom() {
    uint32_t c = chooseNewRandomColor();

#ifdef DEBUG
    Serial.print( "Changing all squares: ");
    Serial.println( c );
#endif

    int t = random(20)+15;
    int direction = random(7);

    for( int s = 0; s < COUNT; s++ ) { 
        transitionCount[s] = t;
        transitionTime[s] = transitionCount[s];
        priorColor[s] = currentColor[s];
        nextColor[s] = c;
        if( direction == 0 ) { 
            transitionSpeed[s] = sweepSpeedCorner1[s];
        } else if( direction == 1 ) { 
            transitionSpeed[s] = sweepSpeedCorner2[s];
        } else if( direction == 2 ) { 
            transitionSpeed[s] = sweepSpeedTopToBottom[s];
        } else if( direction == 3 ) { 
            transitionSpeed[s] = sweepSpeedBottomToTop[s];
        } else if( direction == 4 ) { 
            transitionSpeed[s] = sweepSpeedLeftToRight[s];
        } else if( direction == 5 ) { 
            transitionSpeed[s] = sweepSpeedRightToLeft[s];
        } else if( direction == 6 ) { 
            transitionSpeed[s] = sweepSpeedCenterOut[s];
        }
    }
}


void chooseNewIndividualRandom() {
    int s = random(COUNT);
    uint32_t c = chooseNewRandomColor();

    transitionCount[s] = random(40)+5;
    transitionTime[s] = transitionCount[s];
    priorColor[s] = currentColor[s];
    nextColor[s] = c;

    transitionSpeed[s] = 0.75 + ( ( float ) random(50) ) / 100;

#ifdef DEBUG
    Serial.print( "Changing square: " );
    Serial.print(s);
    Serial.print( " with color: " );
    Serial.print( nextColor[s] );
    Serial.print( ", speed: "  );
    Serial.println( transitionSpeed[s] );

    for( int i = 0; i < 10; i++ ) { 
        strip.setPixelColor(s, currentColor[s]);
        strip.show();
        delay(200);
        strip.setPixelColor(s, nextColor[s]);
        strip.show();
        delay(200);
    }
#endif
    for( int i = 0; i < 3; i++ ) { 
        strip.setPixelColor(s, currentColor[s]);
        strip.show();
        delay(15);
        strip.setPixelColor(s, 0);
        strip.show();
        delay(15);
    }
}

// Move colors along the path from the current color to the next chosen color.
void stepAllColors() {
    for( int i = 0; i < COUNT; i++ ) { 
        if( transitionTime[i] > 0 ) {

            transitionTime[i] -= transitionSpeed[i];
            if( transitionTime[i] < 0 ) { 
                transitionTime[i] = 0;
            }

            uint8_t redPrior = (priorColor[i] & 0xff0000) >> 16;
            uint8_t greenPrior = (priorColor[i] & 0xff00) >> 8;
            uint8_t bluePrior = (priorColor[i] & 0xff);

            uint8_t redNext = (nextColor[i] & 0xff0000) >> 16;
            uint8_t greenNext = (nextColor[i] & 0xff00) >> 8;
            uint8_t blueNext = (nextColor[i] & 0xff);

            float stepRed = ( (float) (redNext - redPrior)) / (float) transitionCount[i];
            float stepGreen = ( (float) (greenNext - greenPrior)) / (float) transitionCount[i];
            float stepBlue = ( (float) (blueNext - bluePrior)) / (float) transitionCount[i];

            uint8_t stepValue = transitionCount[i] - (int)transitionTime[i];

            redPrior += ( stepRed * stepValue );
            greenPrior += ( stepGreen * stepValue );
            bluePrior += ( stepBlue * stepValue );

#ifdef DEBUG           
            Serial.print( "Stepping square: " );
            Serial.print( i );

            Serial.print( ", going from color; ( " );
            Serial.print( redPrior );
            Serial.print( ", " );
            Serial.print( greenPrior );
            Serial.print( ", " );
            Serial.print( bluePrior );

            Serial.print( "), to color; ( " );
            Serial.print( redNext );
            Serial.print( ", " );
            Serial.print( greenNext );
            Serial.print( ", " );
            Serial.print( blueNext );       

            Serial.print( "), step count: " );
            Serial.print( transitionCount[i] );
            Serial.print( ", step: " );
            Serial.print( stepValue );
            Serial.print( ", stepColor: ( " );
            Serial.print( stepRed );
            Serial.print( ", " );
            Serial.print( stepGreen );
            Serial.print( ", " );
            Serial.print( stepBlue );

            Serial.print( "), current value; ( " );
            Serial.print( redPrior );
            Serial.print( ", " );
            Serial.print( greenPrior );
            Serial.print( ", " );
            Serial.print( bluePrior );
            Serial.println (")");
#endif

            if( redPrior > 255 ) { 
                redPrior = 255;
            } 
            if( redPrior < 0 ) { 
                redPrior = 0;
            }            
            if( greenPrior > 255 ) { 
                greenPrior = 255;
            } 
            if( greenPrior < 0 ) { 
                greenPrior = 0;
            }            
            if( bluePrior > 255 ) { 
                bluePrior = 255;
            } 
            if( bluePrior < 0 ) { 
                bluePrior = 0;
            }
            currentColor[i] = strip.Color(redPrior, greenPrior, bluePrior);

            if( transitionTime[i] == 0 ) { 
                currentColor[i] = nextColor[i];
            }
        }
    }
}

void loop() {

    if( state == RANDOM_INDIVIDUAL ) { 
        stepRandomIndividual();
    } else if( state == RANDOM_SWEEP ) { 
        stepRandomSweep();
    } else if( state == RANDOM_WALK ) { 
        stepRandomWalk();
    } else if( state == OFF ) { 
        stepOff();
    }

    stepAllColors();
    for( int i = 0; i < COUNT; i++ ) { 
        strip.setPixelColor(i, currentColor[i]);
    }
    strip.show();

    if( state == RANDOM_INDIVIDUAL ) { 
        delay(33);
    } else if( state == RANDOM_SWEEP ) { 
        delay(15);
    } else if( state == RANDOM_WALK ) { 
        delay(33);
    } else if( state == OFF ) { 
        delay(33);
    }

    uint8_t nextState = convertTiltToState( getTilt() );
    if( nextState != state ) { 
        Serial.print( "Transition state; from: " );
        Serial.print( state );
        Serial.print( ", to: " );
        Serial.print( nextState );
        Serial.print( ", due to tilt: " );
        Serial.println( getTilt() );

        initNextState( nextState );
        state = (State) nextState;

        uint32_t color = 0xff << state;
        for( int i = 0; i < 5; i++ ) { 
            for( int c = 0; c < COUNT; c++ ) {
                strip.setPixelColor(c, 0);
            }
            strip.show();
            delay(200);   
            for( int c = 0; c < COUNT; c++ ) {
                strip.setPixelColor(c, color);
            }
            strip.show();
            delay(200);      
        }
    }
}

void stepRandomIndividual() { 
    if( random(75) <= 1 ) { 
        chooseNewIndividualRandom();
    }
}

void stepRandomSweep() { 
    if( random(75) <= 1 ) { 
        for( int i = 0; i < COUNT; i++ ) { 
            if( transitionTime[i] > 0 ) {
                return;
            }
        }
        chooseNewSweepRandom();
    }
}

void stepOff() { 
    // I decided to go with very dim lights for the 'off' state.  
    if( random(75) <= 1 ) { 
        if( transitionTime[0] != 0 ) { 
            return;
        }
        int s = random(15);
        int t = 100;

        for( int c = 0; c < COUNT; c++ ) {
            priorColor[c] = currentColor[c];
            nextColor[c] = s | ( s << 8 ) | ( s << 16 );
            transitionTime[c] = t;
            transitionCount[c] = t;
            transitionSpeed[c] = 1;
        }
    }
}

void stepRandomWalk() { 
    if( random(10) <= 1 ) { 
        for( int i = 0; i < COUNT; i++ ) { 
            if( transitionTime[i] > 0 ) {
                return;
            }
        }
        chooseNextWalk();
    }
}
