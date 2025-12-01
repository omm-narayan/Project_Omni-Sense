#include <Arduino.h>

#define CO2_PIN 34   // Analog pin for MG811

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("MG811 CO2 Sensor Started...");
}

void loop() {
  // Raw ADC read
  int raw = analogRead(CO2_PIN);

  // Convert to voltage (ESP32 ADC = 0–4095)
  float voltage = raw * (3.3 / 4095.0);

  // Simple smoothing for clean graph
  static float smooth = 0;
  smooth = (smooth * 0.90) + (voltage * 0.10);

  // Serial Plotter friendly format
  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("   Voltage: ");
  Serial.println(smooth);

  delay(30);   // Good for Serial Plotter
}
