const int radarPin = 32; // Connect radar digital OUT here
bool lastState = LOW;
unsigned long lastChange = 0;
unsigned long pulseWidth = 0;
unsigned long lowDuration = 0;

// Calibration values for ~6 meters (adjust slightly after testing)
const unsigned long min_pw = 5000;      // µs for closest distance (~0 m)
const unsigned long max_pw = 200000;    // µs for farthest distance (~6 m)
const float min_dist = 0.0;             // cm
const float max_dist = 600.0;           // 6 meters = 600 cm
const unsigned long humanThreshold = 15000; // µs, adjust after testing

float estimateDistance(unsigned long pw) {
  pw = constrain(pw, min_pw, max_pw);
  return min_dist + (pw - min_pw) * (max_dist - min_dist) / (max_pw - min_pw);
}

void setup() {
  Serial.begin(115200);
  pinMode(radarPin, INPUT);
  Serial.println("Radar Digital Serial Stream Ready (Inverted Logic, ~6 m)");
}

void loop() {
  bool state = digitalRead(radarPin);
  unsigned long now = micros();

  if (state != lastState) {
    unsigned long duration = now - lastChange;

    if (lastState) pulseWidth = duration;   // HIGH pulse
    else lowDuration = duration;            // LOW duration

    lastChange = now;
    lastState = state;

    if (pulseWidth > 0 && lowDuration > 0) {
      float distance = estimateDistance(pulseWidth);

      // Inverted logic: LOW pulse = Human YES
      bool human = pulseWidth < humanThreshold;

      Serial.print("Human: "); Serial.print(human ? "YES" : "NO");
      Serial.print(", Distance: "); Serial.println(distance);

      pulseWidth = 0;
      lowDuration = 0;
    }
  }
}
