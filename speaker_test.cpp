#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

// Pin Definitions
#define SD_CS 41
#define SPI_MOSI 39
#define SPI_MISO 38
#define SPI_SCK 40

// I2S Connections
#define I2S_DOUT 4
#define I2S_BCLK 5
#define I2S_LRC 6

// Audio files
const char* audioFiles[] = {
  "/aria.wav",
  "/led_on.wav",
  "/manual_mode.wav"
};
const int NUM_FILES = sizeof(audioFiles) / sizeof(audioFiles[0]);

// Create Audio object
Audio audio;
int currentTrack = -1;
bool isPlaying = false;

void setup() {
    // Set microSD Card CS as OUTPUT and set HIGH
    pinMode(SD_CS, OUTPUT);      
    digitalWrite(SD_CS, HIGH); 
    
    // Initialize SPI bus for microSD Card
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Start Serial Port
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Start microSD Card
    if(!SD.begin(SD_CS)) {
        Serial.println("Error accessing microSD card!");
        while(true); 
    }
    Serial.println("microSD card initialized successfully!");

    // Check if all files exist
    for (int i = 0; i < NUM_FILES; i++) {
        if (SD.exists(audioFiles[i])) {
            Serial.print("File found: ");
            Serial.println(audioFiles[i]);
        } else {
            Serial.print("Warning: File not found: ");
            Serial.println(audioFiles[i]);
        }
    }

    // Setup I2S 
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    
    // Set Volume
    audio.setVolume(21);
    
    // Display menu
    printMenu();
}

void loop() {
    // Handle audio processing
    audio.loop();
    
    // Check if audio has finished playing
    if (isPlaying && !audio.isRunning()) {
        Serial.println("Track finished playing");
        isPlaying = false;
        printMenu();
    }
    
    // Check for serial input
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        // Convert input to integer
        int selection = input.toInt();
        
        // Check if selection is valid
        if (selection >= 1 && selection <= NUM_FILES) {
            playTrack(selection - 1);  // Adjust for 0-based index
        } else {
            Serial.println("Invalid selection. Please try again.");
            printMenu();
        }
    }
}

void playTrack(int trackIndex) {
    if (trackIndex >= 0 && trackIndex < NUM_FILES) {
        // Stop current audio if playing
        if (isPlaying) {
            audio.stopSong();
            isPlaying = false;
        }
        
        // Play the selected track
        Serial.print("Playing: ");
        Serial.println(audioFiles[trackIndex]);
        
        if (audio.connecttoFS(SD, audioFiles[trackIndex])) {
            currentTrack = trackIndex;
            isPlaying = true;
        } else {
            Serial.println("Error playing file");
            printMenu();
        }
    }
}

void printMenu() {
    Serial.println("\n--- Audio Player Menu ---");
    for (int i = 0; i < NUM_FILES; i++) {
        Serial.print(i + 1);
        Serial.print(": Play ");
        Serial.println(audioFiles[i]);
    }
    Serial.println("Enter a number to play a track");
}