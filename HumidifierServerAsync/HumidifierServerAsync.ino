#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include "config.h"
const char* ssid = STASSID;
const char* password = STAPSK;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool ultrasonic_state = false;
bool led_state = LED_OFF;
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long ledChangeInterval = 50; // Interval for changing LED color
unsigned long ledLastTime;  // The last time LED color was changed
int r, g, b, rd, gd, bd;

#include "webpage.h"

void rgbInit()
{
  int x[3], y[3], i, j;
  for(i=0;i<3;i++) x[i] = random(i*85+1,(i+1)*85+1);
  for(i=0;i<3;i++) y[i] = 0;
  for(i=0;i<3;i++) {
    j = random(0,3);
    while(y[j]!=0) j = random(0,3);
    y[j] = x[i];
  }
  r = y[0];
  g = y[1];
  b = y[2]; 
  rd = gd = bd = -1;
  analogWrite(PIN_RED, r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE, b);
  ledLastTime = millis();
}

void rgbOff()
{
  digitalWrite(PIN_RED, LED_OFF);
  digitalWrite(PIN_GREEN, LED_OFF);
  digitalWrite(PIN_BLUE, LED_OFF);
}

void rgbNext()
{
  int step;
  step = random(1,5);
  if( (r+step*rd) > 255 || (r+step*rd) < 0 ) rd = rd * (-1);
  r = r+step*rd;
  step = random(1,5);
  if( (g+step*gd) > 255 || (g+step*gd) < 0 ) gd = gd * (-1);
  g = g+step*gd;
  step = random(1,5);
  if( (b+step*bd) > 255 || (b+step*bd) < 0 ) bd = bd * (-1);
  b = b+step*bd;
  analogWrite(PIN_RED, r);
  analogWrite(PIN_GREEN, g);
  analogWrite(PIN_BLUE, b);
  ledLastTime = millis();
}

void notifyClients() {
  ws.textAll(String(ultrasonic_state));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      ultrasonic_state = !ultrasonic_state;
      led_state = !led_state;
      //Serial.print("ultra_sonic state = ");
      //Serial.println(ultrasonic_state);
      digitalWrite(PIN_SWITCH, ultrasonic_state);
      if (led_state == LED_ON) rgbInit();
      else rgbOff();
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ultrasonic_state){
      return "checked";
    }
    else{
      return "";
    }
  }
  else if (var == "NAME") {
    return WEB_TITLE;
  }
  return String();
}

void handleRoot(AsyncWebServerRequest *request) {
  // LED Red light 
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_BLUE, HIGH);
  digitalWrite(PIN_GREEN, HIGH);
  Serial.print(index_html);
  request->send_P(200, "text/html", index_html, processor);
  // LED OFF
  digitalWrite(PIN_RED, HIGH);
  digitalWrite(PIN_BLUE, !ultrasonic_state);
}

void handleNotFound(AsyncWebServerRequest *request) {
  digitalWrite(PIN_RED, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
  digitalWrite(PIN_RED, HIGH);
}

void setup(void) {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_SWITCH, OUTPUT);
  digitalWrite(PIN_RED, LED_OFF);
  digitalWrite(PIN_GREEN, LED_OFF);
  digitalWrite(PIN_BLUE, LED_OFF);
  digitalWrite(PIN_SWITCH, LOW);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(PIN_BLUE, !digitalRead(PIN_BLUE));
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(PIN_BLUE, HIGH);

  if (MDNS.begin(HOSTNAME)) {
    Serial.println("MDNS responder started");
  }

  initWebSocket();

  // Route for URI
  server.on("/", HTTP_GET, handleRoot);

  server.begin();
  Serial.println("HTTP server started");
  //Serial.print(index_html);

}

void loop(void) {
  ws.cleanupClients();
  MDNS.update();
  /********************
   * Check Button 
   ********************/
  // read the state of the switch into a local variable:
  int reading = digitalRead(PIN_BUTTON);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      Serial.print("Button State = ");
      Serial.println(buttonState);
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        led_state = !led_state;
        // Set LED State
        if(led_state == LED_ON) rgbInit();
        else rgbOff();
        ultrasonic_state = !ultrasonic_state;
        // Set Ultrasonic switch
        digitalWrite(PIN_SWITCH, ultrasonic_state);
        Serial.print("ultrasonic_state = ");
        Serial.println(ultrasonic_state);
        notifyClients();
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  // Change LED color
  if(led_state == LED_ON) {
    unsigned long current;
    current = millis();
    if(current - ledLastTime >= ledChangeInterval) rgbNext();
  }
}
