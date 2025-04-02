#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"


//5V and GND
// microSD Card Reader connections
#define SD_CS 41
#define SPI_MOSI 39
#define SPI_MISO 38
#define SPI_SCK 40

// I2S Connections
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6


// Create Audio object
Audio audio;

void setup() {
    // Set microSD Card CS as OUTPUT and set HIGH
    pinMode(SD_CS, OUTPUT);      
    digitalWrite(SD_CS, HIGH); 
    
    // Initialize SPI bus for microSD Card
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Start Serial Port
    Serial.begin(115200);
    
    // Start microSD Card
    if(!SD.begin(SD_CS)) {
        Serial.println("Error accessing microSD card!");
        while(true); 
    }
    Serial.println("microSD card initialized successfully!");

    // Check if the file exists
    File musicFile = SD.open("/aria.wav");
    if (musicFile) {
        Serial.println("File opened successfully");
    } else {
        Serial.println("Failed to open file");
    }

    // Setup I2S 
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
    // Set Volume (adjust this if necessary)
    audio.setVolume(21);
    
    // Open music file
    audio.connecttoFS(SD, "/aria.wav");
}

void loop() {
    audio.loop();    
}
