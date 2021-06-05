



#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
  
int ThreeOrFiveMinuteSwitch = 2;
const int GoButton = 1;
int HornButton = 2;
bool running = false;

Adafruit_7segment display = Adafruit_7segment();

unsigned long millisToZero;
unsigned long millisLastChecked;
unsigned long millisCurrent;



void setup() {
  // put your setup code here, to run once:
  pinMode(ThreeOrFiveMinuteSwitch, INPUT);
  pinMode(GoButton, INPUT_PULLUP);
  pinMode(HornButton, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  if(digitalRead(ThreeOrFiveMinuteSwitch) == HIGH) {
    millisToZero = 330 * 1000 ; // five and a half minutes
  } else {
    millisToZero = 210 * 1000; // three and a half minutes
  }
  
  display.begin(0x70);
}

void loop() {
  if(digitalRead(GoButton) == LOW) {
    toggleState();
  }

  if(running) {
    updateTime();
  }


  delay(100);
  Serial.println(millisToZero);

}

void toggleState(){
    if(running) {
      //stop
      running = false;
    } else {
      running = true;
      millisLastChecked = millis();
  }
}
void updateTime(){
    millisCurrent = millis();
    if(millisCurrent > millisLastChecked) {
      millisToZero = millisToZero - (millisCurrent - millisLastChecked);
      millisLastChecked = millisCurrent;
    }
    writeTime();
}
void writeTime(){
    display.writeDigitNum(0, (millisToZero / 1000000), false);
    display.writeDigitNum(1, (millisToZero / 100000) % 10, false);
    display.drawColon(true);
    display.writeDigitNum(3, (millisToZero / 10000) % 10, false);
    display.writeDigitNum(4, (millisToZero / 1000) % 10, false);
    display.writeDisplay();
}

