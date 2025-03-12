#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

Adafruit_TMP117 tmp117;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Pause until serial console opens
  Serial.println("Adafruit TMP117 test on ESP32-S3!");

  // Initialize I2C on specific pins (optional if using default pins)
  Wire.begin(8, 9); // SDA = GPIO 8, SCL = GPIO 9

  // Try to initialize the TMP117 sensor
  if (!tmp117.begin()) {
    Serial.println("Failed to find TMP117 chip");
    while (1) { delay(10); }
  }
  Serial.println("TMP117 Found!");
}

void loop() {
  sensors_event_t temp; // Create an empty event
  tmp117.getEvent(&temp); // Fill the event with the current temperature measurement
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  delay(1000); // Wait 1 second
}