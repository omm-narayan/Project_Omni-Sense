#include <Arduino.h>
#include <driver/i2s.h>

// SAFE PINS - Avoid GPIO 15
#define CO2_PIN 34
#define RADAR_PIN 23
#define I2S_WS 13    // Changed from 15
#define I2S_SCK 14
#define I2S_SD 32

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("CO2,Presence,Vibration");
  
  pinMode(CO2_PIN, INPUT);
  pinMode(RADAR_PIN, INPUT_PULLDOWN);
  
  // Wait for I2S sensor to stabilize
  delay(1000);
}

void loop() {
  // --- CO2 SENSOR ---
  int raw_co2 = analogRead(CO2_PIN);
  float voltage = raw_co2 * (3.3 / 4095.0);
  
  static float smooth_co2 = 0;
  smooth_co2 = (smooth_co2 * 0.9) + (voltage * 0.1);
  float co2_output = smooth_co2 * 1000;
  
  // --- RADAR SENSOR ---
  int radar = digitalRead(RADAR_PIN);
  float presence = (radar == LOW) ? 100.0 : 0.0;
  
  // --- SIMULATED VIBRATION (for testing) ---
  static float fake_vibration = 0;
  static unsigned long last_vib_change = 0;
  static float vib_target = 0;
  
  unsigned long now = millis();
  
  // Change vibration target every 2 seconds
  if (now - last_vib_change > 2000) {
    vib_target = random(0, 80);
    last_vib_change = now;
  }
  
  // Smooth transition to target
  fake_vibration = fake_vibration * 0.95 + vib_target * 0.05;
  
  // Add some noise
  fake_vibration += random(-5, 5);
  fake_vibration = constrain(fake_vibration, 0, 100);
  
  // --- SERIAL OUTPUT ---
  Serial.print(co2_output);
  Serial.print(",");
  Serial.print(presence);
  Serial.print(",");
  Serial.println(fake_vibration);
  
  delay(50);
}