// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "ESP32";       // Wi-Fi SSID
const char* password = "1234";    // Wi-Fi password

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variables to store the current output state
String LED_pin_state = "off";
String heat_pin_state = "off";
String fan_pin_state = "off";

// Assign output variables to GPIO pins
const int LED_pin = 8;
const int heat_pin = 9;
const int fan_pin = 10;

// Timing variables
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // Timeout in milliseconds

// Function to send the web page content to the client
void sendWebPage(WiFiClient& client) {
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    client.println("<style>");
    client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 { background-color: #F52718; }");
    client.println("</style></head>");
    client.println("<body><h1>APS480 NEST SMART HOME PROJECT</h1>");

    // Display buttons for LED, HEAT, and FAN
    sendButton(client, "LED", LED_pin_state);
    sendButton(client, "HEAT", heat_pin_state);
    sendButton(client, "FAN", fan_pin_state);

    client.println("</body></html>");
}

// Function to display the current state and buttons
void sendButton(WiFiClient& client, const String& pin_name, String& pin_state) {
    client.println("<p>" + pin_name + " - State: " + pin_state + "</p>");
    if (pin_state == "off") {
        client.println("<p><a href=\"/" + pin_name + "/on\"><button class=\"button\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/" + pin_name + "/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize GPIO pins as outputs
    pinMode(LED_pin, OUTPUT);
    pinMode(heat_pin, OUTPUT);
    pinMode(fan_pin, OUTPUT);

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
}

void loop() {
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

                        // Process the GET request
                        if (header.indexOf("GET /LED/on") >= 0) {
                            Serial.println("Turning LED ON");
                            LED_pin_state = "on";
                            digitalWrite(LED_pin, HIGH);
                        } else if (header.indexOf("GET /LED/off") >= 0) {
                            Serial.println("Turning LED OFF");
                            LED_pin_state = "off";
                            digitalWrite(LED_pin, LOW);
                        } else if (header.indexOf("GET /HEAT/on") >= 0) {
                            Serial.println("Turning HEAT ON");
                            heat_pin_state = "on";
                            digitalWrite(heat_pin, HIGH);
                        } else if (header.indexOf("GET /HEAT/off") >= 0) {
                            Serial.println("Turning HEAT OFF");
                            heat_pin_state = "off";
                            digitalWrite(heat_pin, LOW);
                        } else if (header.indexOf("GET /FAN/on") >= 0) {
                            Serial.println("Turning FAN ON");
                            fan_pin_state = "on";
                            digitalWrite(fan_pin, HIGH);
                        } else if (header.indexOf("GET /FAN/off") >= 0) {
                            Serial.println("Turning FAN OFF");
                            fan_pin_state = "off";
                            digitalWrite(fan_pin, LOW);
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
}