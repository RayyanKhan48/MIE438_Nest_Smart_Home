#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>


#define HEATER_PIN 9 


Adafruit_TMP117 tmp117; //this was from the temp test, define other variable definitions here

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(HEATER_PIN, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(HEATER_PIN, HIGH); 
  delay(10000);
  digitalWrite(HEATER_PIN, LOW);
  delay(10000);

}