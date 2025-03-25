// Load Wi-Fi library 
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "ESP32"; // will have to search for this name under the wifi (Eduroam wifi or UofT wifi)
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
const int LED_pin = 8;
const int heat_pin = 9;
const int fan_pin = 10;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(LED_pin, OUTPUT);
  pinMode(heat_pin, OUTPUT);
  pinMode(fan_pin, OUTPUT);
  // Set outputs to LOW
  digitalWrite(LED_pin, LOW);
  digitalWrite(heat_pin, LOW);
  digitalWrite(fan_pin, LOW);

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

// Function to display current state & on/off button for output pins 
void sendButton(WiFiClient& client, const String& pin_name, String& pin_state) {
    client.println("<p>" + pin_name + " - State " + pin_state + "</p>");
    if (pin_state == "off") {
        client.println("<p><a href=\"/" + pin_name + "/on\"><button class=\"button\">ON</button></a></p>");
    } else {
        client.println("<p><a href=\"/" + pin_name + "/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
}

void loop(){
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
            
            // turns the GPIOs on and off depending on webpage inputs
            if (header.indexOf("GET /LED/on") >= 0) {
              Serial.println("LED_pin on");
              LED_pin_state = "on";
              digitalWrite(LED_pin, HIGH);
            } else if (header.indexOf("GET /LED/off") >= 0) {
              Serial.println("LED_pin off");
              LED_pin_state = "off";
              digitalWrite(LED_pin, LOW);
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
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            //Define web page fonts
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            //Style first button, for ON
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            // Style second button, for OFF
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #F52718;}</style></head>"); 
            
            // Web Page Heading
            client.println("<body><h1>APS480 NEST SMART HOME PROJECT</h1>");

            //Call functions to display the buttons for LED, HEAT and FAN
            sendButton(client, "LED", LED_pin_state);
            sendButton(client, "HEAT", heat_pin_state);
            sendButton(client, "FAN", fan_pin_state);

            //client.println("</body></html>");
          
          }
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}
