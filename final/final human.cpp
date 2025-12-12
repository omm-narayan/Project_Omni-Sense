#include <Arduino.h>

const int radarPin = 23;   // Digital radar input
const int ledPin = 2;      // LED to indicate human
const int windowSize = 100; // Number of samples in sliding window
const int sampleInterval = 100; // ms between samples

int samples[windowSize];
int indexSample = 0;

void setup() {
  Serial.begin(115200);
  pinMode(radarPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // initialize samples array
  for(int i = 0; i < windowSize; i++) samples[i] = 1;
}

bool checkBreathing() {
  int transitions = 0;
  int prev = samples[0];
  for (int i = 1; i < windowSize; i++) {
    if (samples[i] != prev) transitions++;
    prev = samples[i];
  }

  // breathing rhythm approx 3–10 transitions per window
  return (transitions >= 3 && transitions <= 10);
}

void loop() {
  // read new sample
  int val = digitalRead(radarPin);
  samples[indexSample] = val;
  indexSample = (indexSample + 1) % windowSize;

  // check every windowSize * sampleInterval ms
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > windowSize * sampleInterval) {
    lastCheck = millis();

    if (checkBreathing()) {
      Serial.println("HUMAN");
      digitalWrite(ledPin, HIGH);
    } else {
      Serial.println("NO HUMAN");
      digitalWrite(ledPin, LOW);
    }
  }

  delay(sampleInterval);
}
