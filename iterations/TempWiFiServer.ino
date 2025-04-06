#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Adafruit_TMP117.h"

// WiFi credentials
const char *ssid = "ssid";
const char *password = "pass";

// Static IP setup
IPAddress gateway(192, 168, 68, 1);    //gateway needs to be your router IP
IPAddress local_ip(192, 168, 68, 150);   //change this to a random static IP, make sure the last 3 parts of your gateway and IP should be same, if gateway is 192.168.100.###; IP need to be 192.168.100.... as well
IPAddress subnet(255, 255, 255, 0);   
IPAddress dns(8, 8, 8, 8);              

Adafruit_TMP117 tempSensor;
WebServer server(80); // Web server instance

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!tempSensor.begin()) {
    Serial.println("TMP117 not detected. Check wiring.");
    while (1);
  }

  // Set static IP
  WiFi.config(local_ip, gateway, subnet, dns);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", HTTP_GET, sendHTMLPage);
  server.on("/temperature", HTTP_GET, sendTemperature);

  server.begin(); // Start web server
}

void loop() {
  server.handleClient(); // Handle incoming HTTP requests
}

// Function to send the HTML page with AJAX for real-time updates
void sendHTMLPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 Temperature Sensor</title>";
  html += "<script>";
  html += "function updateTemperature() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('tempValue').innerHTML = xhr.responseText + ' °C';";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/temperature', true);";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(updateTemperature, 1000);";  // Call updateTemperature every second
  html += "</script></head><body>";
  html += "<h1>ESP32 Temperature Sensor</h1>";
  html += "<p>Temperature: <span id='tempValue'>Loading...</span></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Function to send the latest temperature as plain text
void sendTemperature() {
  sensors_event_t temp;
  tempSensor.getEvent(&temp);
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" C");

  server.send(200, "text/plain", String(temp.temperature)); // Send temperature as plain text
}
