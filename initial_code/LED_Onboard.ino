#include <Adafruit_NeoPixel.h> 

#define PIN 38 // ESP32-C3 built-in RGB led
#define NUMPIXELS 1

Adafruit_NeoPixel pixels (NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500


void setup () {  
    Serial.begin (115200);

    Serial.printf ("   PIN %i\n", PIN);          
    
    //pinMode(pin, OUTPUT);           

    pixels.begin();

    pixels.clear();

    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color (100, 50, 200));
      pixels.show();
      delay(2000);
    }

}

void loop () {

}