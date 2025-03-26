#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

#define FAN_PIN 10 
#define RPM 48
#define PWM 45


const int FAN_LOW = 35;
const int FAN_MEDIUM = 170;
const int FAN_HIGH = 255;

Adafruit_TMP117 tmp117; //this was from the temp test, define other variable definitions here

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PWM, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(FAN_PIN, HIGH);   
  //analogWrite(PWM, FAN_MEDIUM);
  delay(30000);
  digitalWrite(FAN_PIN, LOW);
  analogWrite(PWM, FAN_MEDIUM);
  delay(10000);

}