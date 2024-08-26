//board 3 mac e0:5a:1b:54:d8:44

//#################################### IMPORT LIBS###################################
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <esp_now.h>
#include <WiFi.h>

//#################################### PUBLIC DECLARATION ###################################
Adafruit_SHT31 sht31 = Adafruit_SHT31();

typedef struct struct_message {
  float temperature;
  float humidity;
} struct_message;

// Create a struct_message to hold the data to send
struct_message dataToSend;

// MAC address of the receiver (replace with your receiver's MAC address)
uint8_t broadcastAddress[] = { 0x80, 0x7d, 0x3a, 0xfc, 0xe5, 0xd4 };

//#################################### FUNCTION DECLARATION ###################################

// Callback function when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//#################################### INIT ###################################

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize the SHT31 sensor
if (!sht31.begin(0x45)) {
    Serial.println("Failed to Initialize the chip, please confirm the wire connection");
    delay(500);
  }

  // Set ESP32 to Wi-Fi Station mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(onDataSent);

  // Register the peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

//#################################### LOOP ###################################

void loop() {
  // Read temperature and humidity values
  dataToSend.temperature = sht31.readTemperature();
  dataToSend.humidity = sht31.readHumidity();

  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  
  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }

  delay(1000);

  // Send the data via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));

  // Wait for a second before sending the next data
  delay(1000);
}
