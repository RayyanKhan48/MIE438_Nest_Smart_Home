#include <WiFi.h>
#include "Adafruit_TMP117.h"
#include <Adafruit_NeoPixel.h> 
#include <Wire.h>

// Replace with your network credentials
const char* ssid = "Galaxy S203765"; 
const char* password = "ohnu5053";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variables to store the current output state
String LED_pin_state = "off";
String heat_pin_state = "off";
String fan_pin_state = "off";
String mode_state = "Manual"; // Default to Manual mode

// Assign output variables to GPIO pins
const int LED_pin = 38;
const int heat_pin = 9;
const int fan_pin = 10;
const int PWM = 48; // PWM pin for fan speed control

// Led Setup
Adafruit_NeoPixel pixels (1, LED_pin, NEO_GRB + NEO_KHZ800);

// Timing variables
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // Timeout in milliseconds

// Create an instance of the TMP117 sensor
Adafruit_TMP117 tmp117;

// Temperature variable and constants
float currentTemperature = 0.0;
//defining temp threshold, ideal temp is around 20 C
const float LOWER_LIM = 23.0;
const float UPPER_LIM = 26.0;

// Fan speed constants (PWM values: 0-255)
const int FAN_LOW = 1;
const int FAN_MEDIUM = 170;
const int FAN_HIGH = 255;

// Function to send the web page content to the client
void sendWebPage(WiFiClient& client) {
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<meta http-equiv=\"refresh\" content=\"2\">");  // Refresh every 5 seconds
    client.println("<link rel=\"icon\" href=\"data:,\">");
    client.println("<style>");
    client.println("html { font-family: Helvetica; display: inline-block; margin: 0px Auto; text-align: center;} ");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;} ");
    client.println(".button2 { background-color: #F52718; }");
    client.println(".button3 { background-color: #0075FF; }");  // Blue color
    client.println("</style></head>");
    client.println("<body><h1>MIE438 NEST SMART HOME PROJECT</h1>");

    // Display temperature
    client.println("<p>Current Temperature: " + String(currentTemperature) + " &deg;C</p>");

    // Display mode state and buttons
    client.println("<p>Mode: " + mode_state + "</p>");
    if (mode_state == "Manual") {
        client.println("<p><a href=\"/mode/Auto\"><button class=\"button button2\">Switch to Auto</button></a></p>");
    } else {
        client.println("<p><a href=\"/mode/Manual\"><button class=\"button button3\">Auto Mode ON</button></a></p>");
        // Changed from button2 (red) to button3 (blue)
    }

    // Display buttons for LED, HEAT, and FAN (only in Manual mode)
    if (mode_state == "Manual") {
            sendButton(client, "LED", LED_pin_state);
            sendButton(client, "HEAT", heat_pin_state);
            sendButton(client, "FAN", fan_pin_state);
        }

    client.println("</body></html>");
}

// Function to display the current state and buttons
void sendButton(WiFiClient& client, const String& pin_name, String& pin_state) {
    client.println("<p>" + pin_name + " - State: " + pin_state + "</p>");
    if (pin_state == "off") {
        client.println("<p><a href=\"/" + pin_name + "/on\"><button class=\"button button2\">OFF</button></a></p>");
    } else {
        client.println("<p><a href=\"/" + pin_name + "/off\"><button class=\"button\">ON</button></a></p>");
    }
}

// Automatic control function
void AutomaticControl() {

  //in this code 
    if (currentTemperature < LOWER_LIM) {
        Serial.println("Exceeded lower limit, turning on heater and off fan");
        digitalWrite(heat_pin, LOW); // Turn on heater
        digitalWrite(fan_pin, LOW); // Turn on fan
        analogWrite(PWM, FAN_LOW);
        heat_pin_state = "on";
        fan_pin_state = "on";
    } else if (currentTemperature > UPPER_LIM) {
        Serial.println("Exceeded upper limit, turning on fan and off heater");
        digitalWrite(heat_pin, HIGH); // Turn off heater
        digitalWrite(fan_pin, LOW);   // Turn on fan
        analogWrite(PWM, FAN_HIGH);   // Max fan speed
        heat_pin_state = "off";
        fan_pin_state = "on";

    } 

}

void setup() {
    Serial.begin(115200);
    pixels.begin();
    pixels.clear();

    // Initialize GPIO pins as outputs
    pinMode(LED_pin, OUTPUT);
    pinMode(heat_pin, OUTPUT);
    pinMode(fan_pin, OUTPUT);
    pinMode(PWM, OUTPUT);

    // Set outputs to LOW initially
    digitalWrite(LED_pin, LOW);
    digitalWrite(heat_pin, HIGH);
    digitalWrite(fan_pin, HIGH);

    // Connect to Wi-Fi
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Print connection details
    Serial.println("\nWi-Fi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    server.begin();

    // Initialize the TMP117 sensor
    Wire.begin(1, 2); // SDA = GPIO 1, SCL = GPIO 2
    if (!tmp117.begin()) {
        Serial.println("Failed to find TMP117 chip");
        while (1) { delay(10); }
    }
    Serial.println("TMP117 Found!");
}

void loop() {
    // Read the temperature from TMP117
    sensors_event_t temp;
    tmp117.getEvent(&temp);
    currentTemperature = temp.temperature;

    WiFiClient client = server.available(); // Listen for incoming clients

    if (client) {
        currentTime = millis();
        previousTime = currentTime;
        Serial.println("New Client connected.");
        String currentLine = "";

        while (client.connected() && currentTime - previousTime <= timeoutTime) {
            currentTime = millis();
            if (client.available()) {
                char c = client.read();
                Serial.write(c); // Debug: print received character
                header += c;

                if (c == '\n') { // End of request line
                    if (currentLine.length() == 0) {
                        // HTTP header response
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        //process mode change request
                        if (header.indexOf("GET /mode/Auto") >= 0) {
                            mode_state = "Auto";
                            Serial.println("Switching to Automatic Mode");
                        } else if (header.indexOf("GET /mode/Manual") >= 0) {
                            mode_state = "Manual";
                            Serial.println("Switching to Manual Mode");
                        }

                        //process Manual control requests (only in Manual mode)
                        if (mode_state == "Manual"){ // Process the GET request

                        if (header.indexOf("GET /LED/on") >= 0) {
                            Serial.println("Turning LED ON");
                            LED_pin_state = "on";
                            pixels.setPixelColor(0, pixels.Color(100, 50, 200));
                            pixels.show();
                            //digitalWrite(LED_pin, HIGH);
                        } else if (header.indexOf("GET /LED/off") >= 0) {
                            Serial.println("Turning LED OFF");
                            LED_pin_state = "off";
                            pixels.setPixelColor(0, pixels.Color(0, 0, 0));
                            pixels.show();
                            //digitalWrite(LED_pin, LOW);
                        } else if (header.indexOf("GET /HEAT/on") >= 0) {
                            Serial.println("Turning HEAT ON");
                            heat_pin_state = "on";
                            digitalWrite(heat_pin, LOW);
                        } else if (header.indexOf("GET /HEAT/off") >= 0) {
                            Serial.println("Turning HEAT OFF");
                            heat_pin_state = "off";
                            digitalWrite(heat_pin, HIGH);
                        } else if (header.indexOf("GET /FAN/on") >= 0) {
                            Serial.println("Turning FAN ON");
                            fan_pin_state = "on";
                            digitalWrite(fan_pin, LOW);
                            analogWrite(PWM, FAN_HIGH);
                        } else if (header.indexOf("GET /FAN/off") >= 0) {
                            Serial.println("Turning FAN OFF");
                            fan_pin_state = "off";
                            digitalWrite(fan_pin, HIGH);
                        }
                        //end of Manual mode if statement
                        }

                        // Send the web page to the client
                        sendWebPage(client);
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        // Close the connection
        header = "";
        client.stop();
        Serial.println("Client disconnected.");
    }

    //if in Automatic mode, run the Automatic control logic
    if(mode_state == "Auto"){
      AutomaticControl();
    }
    
}