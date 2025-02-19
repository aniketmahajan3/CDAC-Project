/*
    CDAC project -: Fire and Hazarad detection using CAN and IOT 
    
      Author -:
      1) Amit Kagade (248500130007)  //Group leader
      2) Ajay Sawant (248500130003)
      3) Aniket Mahajan (240850130008) 
      4) Atul Lamkhade (248500130013) 
      5) Omkar Salunkhe (248500130023)
  

    Fire and Hazard Detection System - Transmitter Node
    ---------------------------------------------------
    
    Overview:
    This program is the transmitter node for a fire and hazard detection system using 
    CAN and IoT. It collects sensor data, processes it, and transmits it over the 
    CAN bus to the receiver node.

    Hardware Components:
    - ESP32 (for data acquisition)
    - MCP2515 CAN transceiver (for CAN communication)
    - DHT11 sensor (for temperature and humidity)
    - Flame sensor (for fire detection)
    - MQ2 gas sensor (for gas concentration measurement)

    Functional Description:
    - Reads temperature and humidity from a DHT11 sensor.
    - Reads flame status from a digital flame sensor.
    - Reads gas concentration levels from an MQ2 gas sensor.
    - Determines gas concentration category:
      - 0 = Safe
      - 1 = Low concentration
      - 2 = Medium concentration
      - 3 = High concentration
    - Converts the sensor data to an appropriate format for CAN transmission.
    - Sends the processed data to the receiver node over the CAN bus.

    Communication:
    - Uses MCP2515 to transmit sensor data via the CAN bus.
    - Sends 6 bytes of data:
      - Byte 0-1: Temperature (scaled by 100)
      - Byte 2-3: Humidity (scaled by 100)
      - Byte 4: Flame status (1 = Detected, 0 = No Flame)
      - Byte 5: Gas level (0 = Safe, 1 = Low, 2 = Medium, 3 = High)

    Delay:
    - A delay of 2 seconds is added to allow sensor readings to stabilize.
*/

#include <SPI.h>
#include <mcp_can.h>
#include "DHT.h"

#define DHTPIN 4           // DHT11 Data pin
#define DHTTYPE DHT11      // Define the DHT sensor type
#define FLAME_PIN 27       // Digital pin for Flame sensor
#define MQ2_PIN 34         // Analog pin for MQ2 gas sensor

const int SPI_CS_PIN = 5;  // Chip Select pin for MCP2515
MCP_CAN CAN0(SPI_CS_PIN);  // CAN interface instance

DHT dht(DHTPIN, DHTTYPE);  // DHT11 instance

// Gas concentration thresholds
const int LOW_GAS_THRESHOLD = 1000;
const int MEDIUM_GAS_THRESHOLD = 2000;
const int HIGH_GAS_THRESHOLD = 3000;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS

    pinMode(SPI_CS_PIN, OUTPUT);
    digitalWrite(SPI_CS_PIN, HIGH);

    dht.begin();  // Initialize DHT11 sensor
    pinMode(FLAME_PIN, INPUT);  // Set Flame sensor pin as input

    Serial.println("Initializing MCP2515...");

    if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("MCP2515 Initialization Failed!");
        while (1);
    }

    CAN0.setMode(MCP_NORMAL); // Set to normal mode
}

void loop() {
    float temperature = dht.readTemperature();  // Read temperature in Celsius
    float humidity = dht.readHumidity();        // Read humidity
    int flameStatus = digitalRead(FLAME_PIN);   // Read Flame sensor (HIGH for Flame)
    int gasValue = analogRead(MQ2_PIN);         // Read MQ2 sensor value

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT11 sensor!");
        return;
    }

    // Determine gas concentration level
    int gasLevel = 0;
    if (gasValue >= HIGH_GAS_THRESHOLD) {
        gasLevel = 3;  // High gas concentration
        Serial.println("High Gas Concentration Detected!");
    } else if (gasValue >= MEDIUM_GAS_THRESHOLD) {
        gasLevel = 2;  // Medium gas concentration
        Serial.println("Medium Gas Concentration Detected!");
    } else if (gasValue >= LOW_GAS_THRESHOLD) {
        gasLevel = 1;  // Low gas concentration
        Serial.println("Low Gas Concentration Detected!");
    } else {
        gasLevel = 0;  // Safe levels
        Serial.println("Safe Gas Levels");
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Flame Status: ");
    Serial.print(flameStatus ? "Flame Detected!" : "No Flame");
    Serial.print(", Gas Level: ");
    Serial.println(gasLevel);

    // Convert sensor readings to integers for CAN transmission
    int tempInt = (int)(temperature * 100);  // Scale to maintain decimal precision
    int humInt = (int)(humidity * 100);

    byte data[8];
    data[0] = (tempInt >> 8) & 0xFF;  // Temperature high byte
    data[1] = tempInt & 0xFF;         // Temperature low byte
    data[2] = (humInt >> 8) & 0xFF;   // Humidity high byte
    data[3] = humInt & 0xFF;          // Humidity low byte
    data[4] = flameStatus;            // Flame status (1 = Flame detected, 0 = No Flame)
    data[5] = gasLevel;               // Gas level (0 = Safe, 1 = Low, 2 = Medium, 3 = High)

    byte status = CAN0.sendMsgBuf(0x100, 0, 6, data); // Send 6 bytes of sensor data

    if (status == CAN_OK) {
        Serial.println("Message Sent Successfully!");
    } else {
        Serial.print("Send Error: ");
        Serial.println(status);
    }

    delay(2000);  // Delay for sensor update
}
