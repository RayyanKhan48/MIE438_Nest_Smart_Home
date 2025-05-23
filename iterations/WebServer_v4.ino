#include <WiFi.h>
#include "Adafruit_TMP117.h"
#include <Adafruit_NeoPixel.h> 
#include <Wire.h>

// Replace with your network credentials
const char* ssid = "iPhoneAria"; 
const char* password = "virus2000";

// Set web server port number to 80
WiFiServer server(80);

IPAddress gateway(172, 20, 10, 1);    //gateway needs to be your router IP
IPAddress local_ip(172, 20, 10, 7);   //change this to a random static IP, make sure the last 3 parts of your gateway and IP should be same, if gateway is 192.168.100.###; IP need to be 192.168.100.... as well
IPAddress subnet(255, 255, 255, 0);   
IPAddress dns(8, 8, 8, 8); 

// Variable to store the HTTP request
String header;

// Variables to store the current output state
String LED_pin_state = "off";
String heat_pin_state = "off";
String fan_pin_state = "off";
String auto_pin_state = "off";

// Assign output variables to GPIO pins
const int LED_pin = 38;
const int heat_pin = 9;
const int fan_pin = 10;
const int tmp_SDA = 1;
const int tmp_SCL = 2;
const int fan_PWM = 48;
const int fan_RPM = 47;

// temperature constants
const float IDEAL_TEMP = 25.0;
const float LOWER_LIM = 23.0;
const float UPPER_LIM = 26.0; 

//fan speed constants (PWM values: 0-255)
const int FAN_LOW = 1;
const int FAN_MEDIUM = 170; 
const int FAN_HIGH = 255;

// Led Setup
Adafruit_NeoPixel pixels (1, LED_pin, NEO_GRB + NEO_KHZ800);

// Timing variables
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // Timeout in milliseconds

// Create an instance of the TMP117 sensor
Adafruit_TMP117 tmp117;

// Temperature variable
float currentTemperature = 0.0;

// Function to send the web page content to the client
void sendWebPage(WiFiClient& client) {

    //Update RPM and Temperature readings on the server
    client.println("<script>");
    client.println("function updateData() {");
    client.println("  var xhttp = new XMLHttpRequest();");
    client.println("  xhttp.onreadystatechange = function() {");
    client.println("    if (this.readyState == 4 && this.status == 200) {");
    client.println("      var response = this.responseText.split(',');");
    client.println("      document.getElementById('temperature').innerHTML = response[0] + ' &deg;C';");
    client.println("      document.getElementById('rpm').innerHTML = response[1] + ' RPM';");
    client.println("    }");
    client.println("  };");
    client.println("  xhttp.open('GET', '/data', true);");
    client.println("  xhttp.send();");
    client.println("}");
    client.println("setInterval(updateData, 1000);"); // Update every 100 ms
    client.println("</script>");
    
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    client.println("<style>");
    client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} ");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;} ");
    client.println(".button2 { background-color: #F52718; }");
    client.println("</style>");

    client.println("</head>");
    client.println("<body><h1>MIE480 NEST SMART HOME PROJECT</h1>");

    //Temperature box
    client.println("<div style='border: 2px solid #000; padding: 10px; display: inline-block; border-radius: 8px;'>");
    client.println("<p>Current Temperature: <span id='temperature'>" + String(currentTemperature) + "&deg;C</span></p>"); 
    client.println("</div>");

    //RPM box
    client.println("<div style='border: 2px solid #000; padding: 10px; display: inline-block; border-radius: 8px;'>");
    client.println("<p>Fan RPM: <span id='rpm'>" + String(fanRPM()) + " RPM</span></p>"); 
    client.println("</div>");

    //Auto Button
    client.println("<p>TURN ON AUTO MODE</p>");
    client.println("<p><a href=\"/AUTO/on\"><button class=\"button\">ON</button></a></p>");

    // Display buttons for LED, HEAT, and FAN
    sendButton(client, "LED", LED_pin_state);
    sendButton(client, "HEAT", heat_pin_state);
    sendButton(client, "FAN", fan_pin_state);

    client.println("</body></html>");
}

void sendWebPageAuto(WiFiClient& client) {

    client.println("<script>");
    client.println("function updateTemperature() {");
    client.println("  var xhttp = new XMLHttpRequest();");
    client.println("  xhttp.onreadystatechange = function() {");
    client.println("    if (this.readyState == 4 && this.status == 200) {");
    client.println("      document.getElementById('temperature').innerHTML = this.responseText + ' &deg;C';");
    client.println("    }");
    client.println("  };");
    client.println("  xhttp.open('GET', '/temperature', true);");
    client.println("  xhttp.send();");
    client.println("}");
    client.println("setInterval(updateTemperature, 100);"); // Update every second
    client.println("</script>");
    
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    client.println("<style>");
    client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} ");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;} ");
    client.println(".button2 { background-color: #F52718; }");
    client.println("</style>");

    client.println("</head>");
    client.println("<body><h1>MIE480 NEST SMART HOME PROJECT</h1>");
    client.println("<div style='border: 2px solid #000; padding: 10px; display: inline-block; border-radius: 8px;'>");
    client.println("<p>Current Temperature: <span id='temperature'>" + String(currentTemperature) + "&deg;C</span></p>"); 
    client.println("</div>");

    client.println("<p>TURN OFF AUTO MODE</p>");
    client.println("<p><a href=\"/AUTO/off\"><button class=\"button\">OFF</button></a></p>");
}

// Function to display the current state and buttons
void sendButton(WiFiClient& client, const String& pin_name, String& pin_state) {
    client.println("<p>" + pin_name + "</p>");
    if (pin_state == "off") {
        client.println("<p><a href=\"/" + pin_name + "/on\"><button class=\"button\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/" + pin_name + "/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
}

void autoControl(){
    //automatic temp control logic
  if (currentTemperature < LOWER_LIM) {
    Serial.println("exceeded lower limit, turning on heater and off fan");
    digitalWrite(heat_pin, LOW); // Turn on heater
    digitalWrite(fan_pin, HIGH); //turn off fan
  }


  else if (currentTemperature > UPPER_LIM) {
    Serial.println("exceeded upper limit, turning on fan and off heater");
    digitalWrite(heat_pin, HIGH); //turn off heater
    //maximum cooling
    digitalWrite(fan_pin, LOW); //turn on fan to max speed 
    analogWrite(fan_PWM, FAN_HIGH);
  }

  delay(2000);  // Check every 10 sec 
}

int fanRPM() {
    // Read fan RPM from the appropriate pin (RPM sensor or PWM measurement)
    // For now, returning a placeholder value (you can replace it with actual RPM reading logic)
    return analogRead(fan_RPM); // Replace with actual RPM reading method
}

void turnOff(){
  digitalWrite(fan_pin, HIGH); //turn off fan
  digitalWrite(heat_pin, HIGH); //turn off heater
}


void setup() {
    Serial.begin(115200);
    pixels.begin();
    pixels.clear();

    // Initialize GPIO pins as outputs
    pinMode(LED_pin, OUTPUT);
    pinMode(heat_pin, OUTPUT);
    pinMode(fan_pin, OUTPUT);

    // Set outputs to LOW initially
    digitalWrite(LED_pin, LOW);
    digitalWrite(heat_pin, HIGH);
    digitalWrite(fan_pin, HIGH);

    //Set fan PWM and RPM
    pinMode(fan_PWM, OUTPUT);
    pinMode(fan_RPM, INPUT);

    WiFi.config(local_ip, gateway, subnet, dns);

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
    Wire.begin(tmp_SDA, tmp_SCL); // SDA = GPIO 1, SCL = GPIO 2
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
        sendWebPage(client);
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

                          if (header.indexOf("GET /data") >= 0) {
                            Serial.println("Sending current temperature and RPM data...");
                            client.println(String(currentTemperature) + ", " + String(fanRPM())); // Send only the temperature value     
                        } if (auto_pin_state == "on") {
                            autoControl();
                        } else if (header.indexOf("GET /LED/on") >= 0) {
                            Serial.println("Turning LED ON");
                            LED_pin_state = "on";
                            pixels.setPixelColor(0, pixels.Color(100, 50, 200));
                            pixels.show();
                            sendWebPage(client);
                        } else if (header.indexOf("GET /LED/off") >= 0) {
                            Serial.println("Turning LED OFF");
                            LED_pin_state = "off";
                            pixels.setPixelColor(0, pixels.Color(0, 0, 0));
                            pixels.show();
                            sendWebPage(client);
                        } else if (header.indexOf("GET /HEAT/on") >= 0) {
                            Serial.println("Turning HEAT ON");
                            heat_pin_state = "on";
                            digitalWrite(heat_pin, LOW);
                            sendWebPage(client);
                        } else if (header.indexOf("GET /HEAT/off") >= 0) {
                            Serial.println("Turning HEAT OFF");
                            heat_pin_state = "off";
                            digitalWrite(heat_pin, HIGH);
                            sendWebPage(client);
                        } else if (header.indexOf("GET /FAN/on") >= 0) {
                            Serial.println("Turning FAN ON");
                            fan_pin_state = "on";
                            digitalWrite(fan_pin, LOW);
                            sendWebPage(client);
                        } else if (header.indexOf("GET /FAN/off") >= 0) {
                            Serial.println("Turning FAN OFF");
                            fan_pin_state = "off";
                            digitalWrite(fan_pin, HIGH);
                            sendWebPage(client);
                        }
                          else if (header.indexOf("GET /AUTO/on") >= 0) {
                            auto_pin_state = "on";
                            sendWebPageAuto(client);
                        } else if (header.indexOf("GET /AUTO/off") >= 0) {
                            auto_pin_state = "off";
                            turnOff();
                            sendWebPage(client);
                        }
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
}