#include <Arduino.h>

// SIMPLE TEST - CO2 and Radar only
#define CO2_PIN 34
#define RADAR_PIN 23

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait for serial
  Serial.println("CO2,Presence");
  pinMode(CO2_PIN, INPUT);
  pinMode(RADAR_PIN, INPUT_PULLDOWN);
}

void loop() {
  // CO2 Reading
  int raw_co2 = analogRead(CO2_PIN);
  float voltage = raw_co2 * (3.3 / 4095.0);
  
  // Smoothing
  static float smooth_co2 = 0;
  smooth_co2 = (smooth_co2 * 0.9) + (voltage * 0.1);
  
  // Radar (0 = detection, 1 = no detection)
  int radar = digitalRead(RADAR_PIN);
  float presence = (radar == LOW) ? 100.0 : 0.0;
  
  // Serial Plotter output
  Serial.print(smooth_co2 * 1000);  // Scale CO2
  Serial.print(",");
  Serial.println(presence);
  
  delay(50);
}
