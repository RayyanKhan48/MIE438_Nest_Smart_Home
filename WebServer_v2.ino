#include <WiFi.h>
#include "Adafruit_TMP117.h"
#include <Adafruit_NeoPixel.h> 

// Replace with your network credentials
const char* ssid = "ESP32"; 
const char* password = "1234";

// Set web server port number to 80
WiFiServer server(80);

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

//LED SETUP
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
void sendHTMLPage(WiFiClient& client) {
  // Display the HTML web page
client.println("<!DOCTYPE html><html>");
client.println("<head>");
client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
client.println("<link rel=\"icon\" href=\"data:,\">");

// Define web page fonts and button styles
client.println("<style>");
client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }");
client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");
client.println(".button2 { background-color: #F52718; }");
client.println("</style>");

client.println("<title>ESP32 Temperature Sensor</title>");
client.println("<script>");

// JavaScript to fetch temperature data
client.println("function updateTemperature() {");
client.println("  var xhr = new XMLHttpRequest();");
client.println("  xhr.onreadystatechange = function() {");
client.println("    if (xhr.readyState == 4 && xhr.status == 200) {");
client.println("      document.getElementById('tempValue').innerHTML = xhr.responseText + ' °C';");
client.println("    }");
client.println("  };");
client.println("  xhr.open('GET', '/temperature', true);");
client.println("  xhr.send();");
client.println("}");
client.println("setInterval(updateTemperature, 1000);");  // Call updateTemperature every second

client.println("</script>");
client.println("</head>");

client.println("<body>");
client.println("<h1>APS480 NEST SMART HOME PROJECT</h1>");
client.println("<p>Temperature: <span id='tempValue'>Loading...</span></p>");

// Function to display the buttons for LED, HEAT, and FAN
sendButton(client, "LED", LED_pin_state);
sendButton(client, "HEAT", heat_pin_state);
sendButton(client, "FAN", fan_pin_state);

client.println("</body>");
client.println("</html>");

}

// Function to display the current state & on/off button for output pins
void sendButton(WiFiClient& client, const String& pin_name, String& pin_state) {
    client.println("<p>" + pin_name + " - State " + pin_state + "</p>");
    if (pin_state == "off") {
        client.println("<p><a href=\"/" + pin_name + "/on\"><button class=\"button\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/" + pin_name + "/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Process request to turn GPIO pins on/off
            processRequest();
            
            // Send the HTML page to the client
            sendHTMLPage(client);
          }
        }
      }
    }
    header = "";  // Clear header after processing the request
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
