#include <esp_now.h>
#include <WiFi.h>

// Structure must match transmitter exactly
typedef struct struct_message {
  float temperature;
  float humidity;
  int mq6;
  int mq7;
  int mq135;
  char rain[5];  // Wet or Dry
} struct_message;

struct_message receivedData;

// Callback when data is received
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  // Print all sensor readings neatly
  Serial.print("Temp: "); Serial.print(receivedData.temperature);
  Serial.print(" °C | Hum: "); Serial.print(receivedData.humidity);
  Serial.print(" % | MQ6: "); Serial.print(receivedData.mq6);
  Serial.print(" | MQ7: "); Serial.print(receivedData.mq7);
  Serial.print(" | MQ135: "); Serial.print(receivedData.mq135);
  Serial.print(" | Rain: "); Serial.println(receivedData.rain);
}

void setup() {
  Serial.begin(115200);

  // Set device as Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(onDataReceive);

  Serial.println("Receiver ready...");
}

void loop() {
  // Nothing needed here; data handled in callback
}
