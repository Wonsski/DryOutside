#define YL83_ANALOG_PIN 0
#define YL83_DIGITAL_PIN 2

void setup() {
  Serial.begin(9600);
  pinMode(YL83_ANALOG_PIN, INPUT);
  pinMode(YL83_DIGITAL_PIN, INPUT);
}

void loop() {
  int analogValue = analogRead(YL83_ANALOG_PIN);
  int digitalValue = digitalRead(YL83_DIGITAL_PIN);

  Serial.print(F("YL-83 Analog Value: "));
  Serial.println(analogValue);
  Serial.print(F("YL-83 Digital Value: "));
  Serial.println(digitalValue);

  delay(2000); // Collecting period
}