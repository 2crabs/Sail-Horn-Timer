int ThreeOrFiveMinuteSwitch = 3;
const int GoButton = 1;
int HornButton = 5;


bool running = false;

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
}

void loop() {
  if(digitalRead(GoButton) == LOW) {
    running = true;
    millisLastChecked = millis();


  } else {
    updateTime();
    running = false;
  }

  if(running) {
    updateTime();
  }


  delay(100);
  Serial.println(millisToZero);

}

void updateTime(){
    millisCurrent = millis();
    if(millisCurrent > millisLastChecked) {
      millisToZero = millisToZero - (millisCurrent - millisLastChecked);
      millisLastChecked = millisCurrent;
    }
    write
}
