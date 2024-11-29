#include <DHT22.h>
#include <PMserial.h>

#define dhtPIN 22
#define pmTXPIN 17
#define pmRXPIN 16

DHT22 dht22(dhtPIN);
SerialPM pms(PMSx003, pmRXPIN, pmTXPIN);
 
void setup() {
  Serial.begin(9600);
  pms.init();

  Serial.println("\nSerial begin");
}

void loop() {

  // DHT22
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  if (dht22.getLastError() != dht22.OK) {
    Serial.print("last error :");
    Serial.println(dht22.getLastError());
  }
  
  Serial.print("DHT22:");Serial.print("\t\t");
  Serial.print("h=");Serial.print(h,1);Serial.print("\t");
  Serial.print("t=");Serial.println(t,1);

  // PMS3003
  pms.read();
  Serial.print("PMS3003:");Serial.print("\t");
  Serial.print(F("PM1.0 "));Serial.print(pms.pm01);Serial.print(F(", "));
  Serial.print(F("PM2.5 "));Serial.print(pms.pm25);Serial.print(F(", "));
  Serial.print(F("PM10 ")) ;Serial.print(pms.pm10);Serial.println(F(" [ug/m3]"));

  delay(5000);
}