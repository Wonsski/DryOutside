#include <PMserial.h>
SerialPM pms(PMSx003, 5, 18);  // PMSx003, RX, TX

void setup() {
  Serial.begin(9600);
  pms.init();                   // config serial port
}

void loop() {
  pms.read();                   // read the PM sensor
  Serial.print(F("PM1.0 "));Serial.print(pms.pm01);Serial.print(F(", "));
  Serial.print(F("PM2.5 "));Serial.print(pms.pm25);Serial.print(F(", "));
  Serial.print(F("PM10 ")) ;Serial.print(pms.pm10);Serial.println(F(" [ug/m3]"));
  delay(10000);                 // wait for 10 seconds
}