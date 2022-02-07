#define RED 13
#define GREEN 12
#define WHITE 8
#define BTN 7

int LEDState = 0;
int buttonState = 0;           
int lastButtonState = 0;

unsigned long startPressed = 0;
unsigned long debounceDelay = 50;
unsigned long pressDelay = 3000;

void setup() {
  pinMode(BTN, INPUT);
  pinMode(WHITE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Check change in button state 
  int reading = digitalRead(BTN);
  if (reading != lastButtonState) {
    // Start Timer
    startPressed = millis();
  }
  // If Button is Held for more than 50ms, LED is OFF
  if (millis() - startPressed > debounceDelay) {
    // Check change in button state 
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
      	LEDState = (LEDState + 1) % 5;
      }
    }
  }
  // If Button is Held for more than 3000ms, LED is OFF
  if (millis() - startPressed > pressDelay){
    buttonState = reading;
    if (buttonState == LOW){
      LEDState = 0;
    }
  }
  
  lastButtonState = reading;
  decode(LEDState);
  
  Serial.print("  ");
  Serial.print(millis() - startPressed);
  Serial.print("  ");
  Serial.println(LEDState);
}

void decode(int s) {
  digitalWrite(RED,LOW);
  digitalWrite(GREEN,LOW);
  digitalWrite(WHITE,LOW);
  switch(s) {
    case 1:
      digitalWrite(RED,(millis() / 100) % 2);
      break;
    case 2:
      digitalWrite(GREEN,(millis() / 100) % 2);
      break;
    case 3:
      digitalWrite(WHITE,(millis() / 100) % 2);
      break;
    case 4:
      digitalWrite(RED,(millis() / 100) % 2);
      digitalWrite(GREEN,(millis() / 100) % 2);
      digitalWrite(WHITE,(millis() / 100) % 2);
      break;
  }
}