//demo board mac 80:7d:3a:fc:e5:d4

//#################################### IMPORT LIBS###################################
#include <esp_now.h>
#include <WiFi.h>

//#################################### PUBLIC DECLARATION ###################################
const int fanPin = 5;  // PWM pin connected to the fan's control input

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  float temp;
  float humidity;
} struct_message;

// Create a struct_message called myData
struct_message myData;

//#################################### FUNCTION DECLARATION ###################################

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temp: ");
  Serial.println(myData.temp);
  Serial.print("Humidity: ");
  Serial.println(myData.humidity);

  float temperature = myData.temp;

// PROCESS RCVV DATA
  if (temperature < 25.0) {
    // Low speed (25% duty cycle)
    analogWrite(fanPin, 64);
    Serial.println("Fan Speed: Low");
  } else if (temperature >= 25.0 && temperature < 30.0) {
    // Medium speed (50% duty cycle)
    analogWrite(fanPin, 128);
    Serial.println("Fan Speed: Medium");
  } else {
    // High speed (100% duty cycle)
    analogWrite(fanPin, 255);
    Serial.println("Fan Speed: High");
  }
}

//#################################### INIT ###################################

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the fan pin as an output
  pinMode(fanPin, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

//#################################### LOOP ###################################

void loop() {
}