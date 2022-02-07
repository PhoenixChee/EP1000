#define RED 6
#define YELLOW 7
#define GREEN 8

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

void loop() {
  digitalWrite(RED, HIGH);
  delay(300); // Wait for 300 millisecond(s)
  digitalWrite(RED, LOW);
  delay(300); // Wait for 300 millisecond(s)
  digitalWrite(YELLOW, HIGH);
  delay(300);
  digitalWrite(YELLOW, LOW);
  delay(300);
  digitalWrite(GREEN, HIGH);
  delay(300);
  digitalWrite(GREEN, LOW);
  delay(300);
}