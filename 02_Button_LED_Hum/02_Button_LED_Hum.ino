#include "config.h"
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
}

void loop(void) {
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
        if(led_state == LED_ON) {
          rgbInit();
          Serial.println("LED ON");
        }
        else {
          rgbOff();
          Serial.println("LED OFF");
        }
        ultrasonic_state = !ultrasonic_state;
        // Set Ultrasonic switch
        digitalWrite(PIN_SWITCH, ultrasonic_state);
        Serial.print("ultrasonic_state = ");
        Serial.println(ultrasonic_state);
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
