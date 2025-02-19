/*
    CDAC project -: Fire and Hazarad detection using CAN and IOT 
    
      Author -:
      1) Amit Kagade (248500130007)  //Group leader
      2) Ajay Sawant (248500130003)
      3) Aniket Mahajan (240850130008) 
      4) Atul Lamkhade (248500130013) 
      5) Omkar Salunkhe (248500130023)
   
  
    Fire and Hazard Detection System - Receiver Node
    ---------------------------------------------------
    
    Overview:
    This program is the receiver node for a fire and hazard detection system using 
    CAN and IoT. It receives sensor data over the CAN bus from the transmitter node, 
    processes it, and uploads the data to the Blynk cloud platform for remote monitoring.

    Hardware Components:
    - ESP32 (for IoT connectivity)
    - MCP2515 CAN transceiver (for CAN communication)
    - Blynk cloud platform (for remote monitoring)
    - WiFi module (ESP32 built-in)

    Functional Description:
    - Connects to a WiFi network and initializes the Blynk platform.
    - Configures MCP2515 to receive data over the CAN bus.
    - Reads incoming CAN messages, extracts sensor values, and displays them on the serial monitor.
    - Processes data for:
      - Temperature (in Celsius)
      - Humidity (in percentage)
      - Flame detection (1 = Detected, 0 = No Flame)
      - Gas level (0 = Safe, 1 = Low, 2 = Medium, 3 = High)
    - Uploads sensor data to the Blynk cloud for real-time monitoring.
    - Displays alerts on the serial monitor if hazardous conditions are detected.

    Communication:
    - Uses MCP2515 to receive sensor data from the transmitter node over the CAN bus.
    - Uses Blynk virtual pins:
      - V1: Temperature
      - V2: Humidity
      - V3: Gas Level
      - V4: Flame Status

    Delay:
    - A small delay (100ms) is added to prevent excessive CAN message handling.
*/


#define BLYNK_TEMPLATE_ID "TMPL3E33DZtsD"  // Your template ID from Blynk
#define BLYNK_TEMPLATE_NAME "Sensor Networking"  // Your template name from Blynk

#define BLYNK_PRINT Serial
#include <SPI.h>
#include <mcp_can.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// Blynk credentials
char auth[] = "YrnqKjixTSVKuKMFNoxyr6YspbtumR4d"; // Replace with your Auth token
char ssid[] = "Amit";                             // Replace with your WiFi SSID
char pass[] = "12345678";                         // Replace with your WiFi password


const int SPI_CS_PIN = 5;  // Chip Select pin for MCP2515
MCP_CAN CAN0(SPI_CS_PIN);  // CAN interface instance

void setup() {
  Serial.begin(115200);
  while (!Serial);

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS

  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);

  Serial.println("Initializing MCP2515...");

  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  } else {
    Serial.println("MCP2515 Initialization Failed!");
    while (1);
  }

  CAN0.setMode(MCP_NORMAL); // Set to normal mode

  // Initialize Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  Blynk.run();

  if (CAN0.checkReceive() == CAN_MSGAVAIL) {
    long unsigned int rxId;
    unsigned char len = 0;
    unsigned char rxBuf[8];

    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    Serial.print("Received data from ID: 0x");
    Serial.println(rxId, HEX);

    if (len >= 6) {
      int tempInt = (rxBuf[0] << 8) | rxBuf[1];  // Temperature value
      int humInt = (rxBuf[2] << 8) | rxBuf[3];   // Humidity value
      int flameStatus = rxBuf[4];               // Flame status
      int gasLevel = rxBuf[5];                  // Gas level

      float temperature = tempInt / 100.0;
      float humidity = humInt / 100.0;

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.print(" %, Flame Status: ");
      Serial.print(flameStatus ? "Flame Detected!" : "No Flame");
      Serial.print(", Gas Level: ");

      switch (gasLevel) {
        case 0:
          Serial.println("Safe Gas Levels");
          break;
        case 1:
          Serial.println("Low Gas Concentration");
          break;
        case 2:
          Serial.println("Medium Gas Concentration");
          break;
        case 3:
          Serial.println("High Gas Concentration");
          break;
        default:
          Serial.println("Unknown Gas Level");
          break;
      }

      // Send data to Blynk
      Blynk.virtualWrite(V1, temperature);  // Temperature on Virtual Pin V1
      Blynk.virtualWrite(V2, humidity);     // Humidity on Virtual Pin V2
      Blynk.virtualWrite(V3, gasLevel);     // Gas Level on Virtual Pin V3
      Blynk.virtualWrite(V4, flameStatus);  // Flame Status on Virtual Pin V4
    }
  }

  delay(100); // Small delay to avoid spamming
}
