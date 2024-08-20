#define uS_TO_S_FACTOR 1000000 
#define TIME_TO_SLEEP  2

#include <esp_now.h>
#include <WiFi.h>
#include <DFRobot_SHT3x.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xEC, 0x62, 0x60, 0x94, 0xFB, 0xB4};
DFRobot_SHT3x   sht3x;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temp;
  float humidity;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void hibernate() {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
    
    esp_deep_sleep_start();
}

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  
  //Initialize the chip
  while (sht3x.begin() != 0) {
    Serial.println("Failed to Initialize the chip, please confirm the wire connection");
    delay(1000);
  }

  /**
   * softReset Send command resets via IIC, enter the chip's default mode single-measure mode, 
   * turn off the heater, and clear the alert of the ALERT pin.
   * @return Read the register status to determine whether the command was executed successfully, 
   * and return true indicates success.
   */
  if(!sht3x.softReset()){
    Serial.println("Failed to Initialize the chip....");
  }

  Serial.print("Ambient Temperature(°C):");
  float temp = sht3x.getTemperatureC();
  Serial.print(temp);
  Serial.print(" C/");

  Serial.print("Relative Humidity(%RH):");
  float humidity = sht3x.getHumidityRH();
  Serial.print(humidity);
  Serial.println(" %RH");

  /* SETTING UP ESPNOW*/
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Set values to send
  myData.temp = temp;
  myData.humidity = humidity;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  /* PUTTING ESP32 TO SLEEP (HIBERNATION MODE)*/
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  hibernate();
  Serial.println("This will never be printed");
}

void loop() {
  // put your main code here, to run repeatedly:

}
