#define BLYNK_TEMPLATE_ID "TMPL3Cvdu2oj4"
#define BLYNK_TEMPLATE_NAME "human presence"
#define BLYNK_AUTH_TOKEN "o6cBM12BTKM_Lt03gMM51vCqUDerB02X"

// Include necessary libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "driver/i2s.h"

// Pin Definitions
#define CO2_PIN    34
#define RADAR_PIN  23
#define I2S_WS     15
#define I2S_SCK    14
#define I2S_SD     32

// WiFi credentials
char ssid[] = "Omm's A35";
char pass[] = "Megamind";

// Blynk Virtual Pins
#define VIRTUAL_PIN_CO2_GRAPH      V0      // Real-time CO2 graph
#define VIRTUAL_PIN_VIBRATION_GRAPH V1     // Real-time vibration graph
#define VIRTUAL_PIN_HUMAN_STATUS   V2      // Boolean human detected (0/1)

// For additional features
#define VIRTUAL_PIN_CO2_VALUE      V3      // Current CO2 value display
#define VIRTUAL_PIN_VIBRATION_VALUE V4     // Current vibration value display
#define VIRTUAL_PIN_ALERT          V5      // Alert trigger
#define VIRTUAL_PIN_THRESHOLD      V6      // CO2 threshold control
#define VIRTUAL_PIN_STATUS_LED     V7      // System status LED

// Variables
float co2Value = 0;
float vibrationLevel = 0;
bool humanDetected = false;
float co2Threshold = 1000; // Default CO2 threshold in ppm
bool alertTriggered = false;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000; // Update every 1 second

// I2S Configuration for MEMS Microphone
const i2s_port_t I2S_PORT = I2S_NUM_0;
const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 1024;

// Function to setup I2S for MEMS microphone
void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };
    
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_MONO);
}

// Function to read vibration level from MEMS microphone
float readVibrationLevel() {
    int32_t samples[BUFFER_SIZE];
    size_t bytesRead;
    
    // Read audio samples from I2S
    i2s_read(I2S_PORT, samples, sizeof(samples), &bytesRead, portMAX_DELAY);
    
    int samplesRead = bytesRead / sizeof(int32_t);
    
    // Calculate RMS (Root Mean Square) for vibration level
    long sum = 0;
    for (int i = 0; i < samplesRead; i++) {
        sum += (samples[i] * samples[i]);
    }
    
    float rms = sqrt(sum / samplesRead);
    
    // Normalize the value (adjust scaling factor as needed)
    float vibration = rms / 1000.0;
    
    return vibration;
}

// Function to read CO2 sensor (MG811)
float readCO2Sensor() {
    // Read analog value from CO2 sensor
    int sensorValue = analogRead(CO2_PIN);
    
    // MG811 calibration - adjust these values based on your sensor calibration
    // This is a basic conversion, you should calibrate your sensor properly
    float voltage = sensorValue * (3.3 / 4095.0); // ESP32 ADC resolution
    
    // Convert voltage to CO2 concentration (ppm)
    // MG811 output: 400ppm = 0.4V, 5000ppm = 2.0V (approximately)
    // Formula: ppm = 2500 * (voltage - 0.4) / 1.6 for 400-5000ppm range
    float co2ppm = 2500 * (voltage - 0.4) / 1.6;
    
    // Clamp values to reasonable range
    if (co2ppm < 400) co2ppm = 400;
    if (co2ppm > 5000) co2ppm = 5000;
    
    return co2ppm;
}

// Function to check human presence from radar sensor
bool checkHumanPresence() {
    // Read radar sensor (assuming it gives HIGH when human detected)
    int radarValue = digitalRead(RADAR_PIN);
    
    // Some radar sensors might need debouncing
    static unsigned long lastDetectionTime = 0;
    static bool lastState = false;
    
    bool currentState = (radarValue == HIGH);
    
    // Simple debouncing
    if (currentState != lastState) {
        lastDetectionTime = millis();
    }
    
    lastState = currentState;
    
    // Return true if human is detected
    return currentState;
}

// Blynk Timer object
BlynkTimer timer;

// Function to send sensor data to Blynk
void sendSensorData() {
    // Read all sensors
    co2Value = readCO2Sensor();
    vibrationLevel = readVibrationLevel();
    humanDetected = checkHumanPresence();
    
    // Send CO2 data to V0 (graph)
    Blynk.virtualWrite(V0, co2Value);
    
    // Send vibration data to V1 (graph)
    Blynk.virtualWrite(V1, vibrationLevel);
    
    // Send human detection status to V2 (0/1)
    Blynk.virtualWrite(V2, humanDetected ? 1 : 0);
    
    // Optional: Send additional data to other virtual pins
    Blynk.virtualWrite(V3, co2Value);          // CO2 value display
    Blynk.virtualWrite(V4, vibrationLevel);    // Vibration value display
    
    // Check CO2 threshold and trigger alert if needed
    if (co2Value > co2Threshold && !alertTriggered) {
        alertTriggered = true;
        Blynk.virtualWrite(V5, 1);  // Trigger alert
        Blynk.logEvent("co2_alert", String("CO2 level exceeded: ") + co2Value + " ppm");
    } else if (co2Value <= co2Threshold && alertTriggered) {
        alertTriggered = false;
        Blynk.virtualWrite(V5, 0);  // Clear alert
    }
    
    // Update status LED
    Blynk.virtualWrite(V7, humanDetected ? 1023 : 0);  // Bright when human detected
}

// Blynk write handler for threshold control
BLYNK_WRITE(V6) {
    co2Threshold = param.asFloat();
    Blynk.virtualWrite(V3, String("Threshold: ") + co2Threshold + " ppm");
}

// Setup function
void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    Serial.println("Starting Sensor System...");
    
    // Initialize pins
    pinMode(CO2_PIN, INPUT);
    pinMode(RADAR_PIN, INPUT);
    pinMode(I2S_WS, OUTPUT);
    pinMode(I2S_SCK, OUTPUT);
    pinMode(I2S_SD, INPUT);
    
    // Setup I2S for MEMS microphone
    setupI2S();
    
    // Connect to WiFi and Blynk
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Connected to Blynk!");
    
    // Setup timer to send sensor data every second
    timer.setInterval(updateInterval, sendSensorData);
    
    // Send initial threshold value to Blynk
    Blynk.virtualWrite(V6, co2Threshold);
}

// Main loop
void loop() {
    Blynk.run();
    timer.run();
    
    // Print sensor values to Serial Monitor for debugging
    static unsigned long lastSerialPrint = 0;
    if (millis() - lastSerialPrint > 2000) {  // Print every 2 seconds
        Serial.print("CO2: ");
        Serial.print(co2Value);
        Serial.print(" ppm | Vibration: ");
        Serial.print(vibrationLevel);
        Serial.print(" | Human: ");
        Serial.println(humanDetected ? "YES" : "NO");
        lastSerialPrint = millis();
    }
}