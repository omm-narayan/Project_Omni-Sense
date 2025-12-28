// ESP32 automatic scanner + CO2 reporter (distance in meters, 2 decimals)
// Pins: RADAR_PIN digital OUT, CO2_PIN analog IN (ADC1).
// Upload to your ESP32 with Arduino IDE or PlatformIO.

#include <Arduino.h>

const int RADAR_PIN = 32;     // digital radar output pin
const int CO2_PIN   = 34;     // analog CO2 input (ADC1_CH6)

const unsigned long CYCLE_MS = 15000UL;      // run cycle every 15s
const unsigned long SCAN_DURATION_MS = 10000UL; // scanning window 10s
const unsigned long CO2_INTERVAL_MS = 500UL; // send CO2 every 500ms during normal op

// Radar pulse timing (microseconds)
unsigned long lastChange = 0;
bool lastState = LOW;
unsigned long lastPulseWidth = 0;


// Distance calibration (microseconds -> meters)
const unsigned long MIN_PW = 5000UL;      // µs for closest
const unsigned long MAX_PW = 200000UL;    // µs for farthest (~6m)
const float MIN_DIST_M = 0.0f;
const float MAX_DIST_M = 6.0f;

// Detection threshold (µs) — pulseWidth < threshold => detection
const unsigned long HUMAN_PW_THRESHOLD = 15000UL; // tune after testing

// state
unsigned long lastCycleStart = 0;
unsigned long lastCo2Sent = 0;

float estimateMeters(unsigned long pw_us) {
  unsigned long pw = pw_us;
  if (pw < MIN_PW) pw = MIN_PW;
  if (pw > MAX_PW) pw = MAX_PW;
  float t = float(pw - MIN_PW) / float(MAX_PW - MIN_PW);
  return MIN_DIST_M + t * (MAX_DIST_M - MIN_DIST_M);
}

void sendCo2Once() {
  int raw = analogRead(CO2_PIN); // 0..4095 on ESP32 ADC
  float voltage = raw * (3.3f / 4095.0f);
  // Preserve same scaling that worked earlier: scale*1000
  float out = voltage * 1000.0f;
  // send as: CO2,<value>
  Serial.print("CO2,");
  Serial.println(out, 2);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // allow serial to settle
  pinMode(RADAR_PIN, INPUT);
  analogReadResolution(12); // 12-bit ADC (0-4095)
  lastCycleStart = millis();
  lastCo2Sent = 0;
  Serial.println("ESP32 AUTO SCANNER READY");
}

void loop() {
  unsigned long nowMs = millis();

  // Send periodic CO2 reading (during idle & scans)
  if ((nowMs - lastCo2Sent) >= CO2_INTERVAL_MS) {
    sendCo2Once();
    lastCo2Sent = nowMs;
  }

  // Manage automatic cycle
  if ((nowMs - lastCycleStart) >= CYCLE_MS) {
    // 1) announce initiation
    Serial.println("SCAN_INITIATING");
    delay(1000); // short pause

    // 2) hold still
    Serial.println("HOLD_STILL");
    delay(2000);

    // 3) scanning
    Serial.println("SCANNING");

    // scanning window: collect detections and distances
    unsigned long scanStart = millis();
    unsigned long scanEnd = scanStart + SCAN_DURATION_MS;
    int detectionCount = 0;
    unsigned long sumDetectedPw = 0;
    int pwSamples = 0;
    lastPulseWidth = 0;
    lastChange = micros();
    lastState = digitalRead(RADAR_PIN);

    while (millis() < scanEnd) {
      bool state = digitalRead(RADAR_PIN);
      unsigned long nowUs = micros();

      if (state != lastState) {
        unsigned long duration = nowUs - lastChange;
        // if lastState was HIGH, duration is pulse width of HIGH
        if (lastState == HIGH) {
          lastPulseWidth = duration; // pulse width in µs
          // detection if pulseWidth < threshold
          if (lastPulseWidth > 0 && lastPulseWidth < HUMAN_PW_THRESHOLD) {
            detectionCount++;
            sumDetectedPw += lastPulseWidth;
            pwSamples++;
          }
        }
        lastState = state;
        lastChange = nowUs;
      }

      // also send occasional CO2 during scanning
      if ((millis() - lastCo2Sent) >= CO2_INTERVAL_MS) {
        sendCo2Once();
        lastCo2Sent = millis();
      }
    } // end scanning window

    // Compute result
    int human = (detectionCount >= 5) ? 1 : 0;
    float meters = 0.0f;
    if (human && pwSamples > 0) {
      float avgPw = float(sumDetectedPw) / float(pwSamples);
      meters = estimateMeters((unsigned long)avgPw);
    }

    // Send final CSV result (meters with two decimals)
    Serial.print("HUMAN,");
    Serial.print(human);
    Serial.print(",");
    Serial.println(meters, 2);

    // prepare next cycle
    lastCycleStart = millis();
    // small sleep to avoid tight loop
    delay(20);
  }

  // Small idle yield
  delay(10);
}
