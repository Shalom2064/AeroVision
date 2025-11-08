#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>

// Sensor Pins
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define MQ6_PIN 34
#define MQ7_PIN 36
#define MQ135_PIN 32
#define RAIN_PIN 35  // Keep analog pin but we will convert to Wet/Dry

DHT dht(DHT_PIN, DHT_TYPE);

// Receiver MAC
uint8_t receiverMAC[] = {0xC0, 0xCD, 0xD6, 0xCF, 0xC5, 0x1C};

// Structure for all sensors
typedef struct struct_message {
  float temperature;
  float humidity;
  int mq6;
  int mq7;
  int mq135;
  char rain[5];  // Wet or Dry
} struct_message;

struct_message sensorData;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Data send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

void setup() {
  Serial.begin(115200);

  // Initialize sensors
  dht.begin();
  pinMode(MQ6_PIN, INPUT);
  pinMode(MQ7_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);

  // Set WiFi mode
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW!");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    return;
  }

  Serial.println("Transmitter ready...");
}

void loop() {
  // Read sensors
  sensorData.temperature = dht.readTemperature();
  sensorData.humidity = dht.readHumidity();
  sensorData.mq6 = analogRead(MQ6_PIN);
  sensorData.mq7 = analogRead(MQ7_PIN);
  sensorData.mq135 = analogRead(MQ135_PIN);

  // Rain sensor: convert to Wet/Dry
  int rainReading = analogRead(RAIN_PIN);
  if (rainReading < 2000) {  // adjust threshold if needed
    strcpy(sensorData.rain, "Wet");
  } else {
    strcpy(sensorData.rain, "Dry");
  }

  // Print to Serial
  Serial.print("Temp: "); Serial.print(sensorData.temperature);
  Serial.print(" °C | Hum: "); Serial.print(sensorData.humidity);
  Serial.print(" % | MQ6: "); Serial.print(sensorData.mq6);
  Serial.print(" | MQ7: "); Serial.print(sensorData.mq7);
  Serial.print(" | MQ135: "); Serial.print(sensorData.mq135);
  Serial.print(" | Rain: "); Serial.println(sensorData.rain);

  // Send data via ESP-NOW
  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&sensorData, sizeof(sensorData));

  if (result != ESP_OK) {
    Serial.println("Error sending data!");
  }

  delay(2000);
}
