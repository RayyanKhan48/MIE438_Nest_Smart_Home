#include <WiFi.h>
#include <WebServer.h>
#include "Adafruit_TMP117.h"
#include <Adafruit_NeoPixel.h> 

// Replace with your network credentials
const char* ssid = "ESP32"; 
const char* password = "1234";

Adafruit_TMP117 tempSensor;

// Set web server port number to 80
WebServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String LED_pin_state = "off";
String heat_pin_state = "off";
String fan_pin_state = "off";

// Assign output variables to GPIO pins
const int LED_pin = 38;
const int heat_pin = 9;
const int fan_pin = 10;
const int temp_SDA = 1;
const int temp_SCL = 2;

//Led Setup
Adafruit_NeoPixel pixels (0, LED_pin, NEO_GRB + NEO_KHZ800);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(heat_pin, OUTPUT);
  pinMode(fan_pin, OUTPUT);
  // Set outputs to LOW
  digitalWrite(heat_pin, HIGH);
  digitalWrite(fan_pin, HIGH);
  Wire.begin(temp_SDA, temp_SCL);
  //Setup LED
  pixels.begin();
  pixels.clear();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // Define routes
  server.on("/", HTTP_GET, sendHTMLPage);
  server.on("/temperature", HTTP_GET, sendTemperature);
}

// Function to process incoming requests and turn GPIO pins on/off
void processRequest() {
  if (header.indexOf("GET /LED/on") >= 0) {
    Serial.println("LED_pin on");
    pixels.setPixelColor(0, pixels.Color(100, 50, 200));
    pixels.show();
  } else if (header.indexOf("GET /LED/off") >= 0) {
    Serial.println("LED_pin off");
    LED_pin_state = "off";
    pixels.clear();
  } else if (header.indexOf("GET /HEAT/on") >= 0) {
    Serial.println("heat_pin on");
    heat_pin_state = "on";
    digitalWrite(heat_pin, HIGH);
  } else if (header.indexOf("GET /HEAT/off") >= 0) {
    Serial.println("heat_pin off");
    heat_pin_state = "off";
    digitalWrite(heat_pin, LOW);
  } else if (header.indexOf("GET /FAN/on") >= 0) {
    Serial.println("fan_pin on");
    fan_pin_state = "on";
    digitalWrite(fan_pin, HIGH);
  } else if (header.indexOf("GET /FAN/off") >= 0) {
    Serial.println("fan_pin off");
    fan_pin_state = "off";
    digitalWrite(fan_pin, LOW);
  }
}

// Function to send HTML content to the client
void sendHTMLPage() {
  // Display the HTML web page
  server.send(200, "text/html", "<!DOCTYPE html><html>");
  server.send(200, "text/html", "<head>");
  server.send(200, "text/html", "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  server.send(200, "text/html", "<link rel=\"icon\" href=\"data:,\">");

  // Define web page fonts and button styles
  server.send(200, "text/html", "<style>");
  server.send(200, "text/html", "html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }");
  server.send(200, "text/html", ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");
  server.send(200, "text/html", ".button2 { background-color: #F52718; }");
  server.send(200, "text/html", "</style>");

  server.send(200, "text/html", "<title>ESP32 Temperature Sensor</title>");
  server.send(200, "text/html", "<script>");

  // JavaScript to fetch temperature data
  server.send(200, "text/html", "function updateTemperature() {");
  server.send(200, "text/html", "  var xhr = new XMLHttpRequest();");
  server.send(200, "text/html", "  xhr.onreadystatechange = function() {");
  server.send(200, "text/html", "    if (xhr.readyState == 4 && xhr.status == 200) {");
  server.send(200, "text/html", "      document.getElementById('tempValue').innerHTML = xhr.responseText + ' °C';");
  server.send(200, "text/html", "    }");
  server.send(200, "text/html", "  };");
  server.send(200, "text/html", "  xhr.open('GET', '/temperature', true);");
  server.send(200, "text/html", "  xhr.send();");
  server.send(200, "text/html", "}");
  server.send(200, "text/html", "setInterval(updateTemperature, 1000);");  // Call updateTemperature every second

  server.send(200, "text/html", "</script>");
  server.send(200, "text/html", "</head>");

  server.send(200, "text/html", "<body>");
  server.send(200, "text/html", "<h1>APS480 NEST SMART HOME PROJECT</h1>");
  server.send(200, "text/html", "<p>Temperature: <span id='tempValue'>Loading...</span></p>");

  // Function to display the buttons for LED, HEAT, and FAN
  sendButton();

  server.send(200, "text/html", "</body>");
  server.send(200, "text/html", "</html>");
}

// Function to display the current state & on/off button for output pins
void sendButton() {
  server.send(200, "text/html", "<p>LED - State " + LED_pin_state + "</p>");
  if (LED_pin_state == "off") {
    server.send(200, "text/html", "<p><a href=\"/LED/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    server.send(200, "text/html", "<p><a href=\"/LED/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  server.send(200, "text/html", "<p>HEAT - State " + heat_pin_state + "</p>");
  if (heat_pin_state == "off") {
    server.send(200, "text/html", "<p><a href=\"/HEAT/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    server.send(200, "text/html", "<p><a href=\"/HEAT/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  server.send(200, "text/html", "<p>FAN - State " + fan_pin_state + "</p>");
  if (fan_pin_state == "off") {
    server.send(200, "text/html", "<p><a href=\"/FAN/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    server.send(200, "text/html", "<p><a href=\"/FAN/off\"><button class=\"button button2\">OFF</button></a></p>");
  }
}

void sendTemperature() {
  sensors_event_t temp;
  tempSensor.getEvent(&temp);
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" C");

  server.send(200, "text/plain", String(temp.temperature)); // Send temperature as plain text
}

void loop() {
  server.handleClient();  // Handle incoming HTTP requests
}
