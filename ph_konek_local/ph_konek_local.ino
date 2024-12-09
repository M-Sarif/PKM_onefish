#include <WiFi.h>
#include "DFRobot_PH.h"
#include <EEPROM.h>

// WiFi Configuration
const char* ssid = "wifi ono";
const char* password = "wifi kencang gak bikin kembung";
const char* serverIP = "192.168.162.213";
const int serverPort = 80;

// Sensor MQ-137 Configuration
const int mq137Pin = A0;
const float VREF = 5.0;
const int ADC_RESOLUTION = 1025;
const float RL = 10.0;  // Load resistor in kilo-ohms
const float RO = 10.0;  // RO calibration value
const float a = -2.4;   // Slope from datasheet
const float b = 0.7;    // Intercept from datasheet

// Sensor pH Configuration
#define PH_PIN A1
float voltage, phValue, temperature = 25;
DFRobot_PH ph;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  Serial.println("MQ-137 and pH Sensor Data Transmission");

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize pH sensor
  ph.begin();
}

void loop() {
  // MQ-137 sensor reading
  int mq137Value = analogRead(mq137Pin);
  float mq137Voltage = mq137Value * (VREF / ADC_RESOLUTION);
  float Rs = ((VREF * RL) / mq137Voltage) - RL;
  float ratio = Rs / RO;
  float nh3PPM = pow(10, ((log10(ratio) - b) / a));

  // pH sensor reading
  voltage = analogRead(PH_PIN) / 1024.0 * 4440;
  phValue = ph.readPH(voltage, temperature);

  // Display readings
  Serial.print("MQ-137: Analog=");
  Serial.print(mq137Value);
  Serial.print(" Voltage=");
  Serial.print(mq137Voltage, 2);
  Serial.print("V NH3=");
  Serial.print(nh3PPM, 2);
  Serial.println("ppm");

  Serial.print("pH Sensor: Voltage=");
  Serial.print(voltage, 2);
  Serial.print("mV pH=");
  Serial.println(phValue, 2);

  // Send MQ-137 data to server
  sendToServer("/pkm/koneksi/koneksi_mq.php",
               "mq137_value=" + String(mq137Value) +
               "&mq137_voltage=" + String(mq137Voltage, 2) +
               "&nh3_ppm=" + String(nh3PPM, 2));

  // Send pH data to server
  sendToServer("/pkm/koneksi/koneksi_ph.php",
               "temperature=" + String(temperature, 1) +
               "&ph=" + String(phValue, 2));

  // Calibrate pH sensor
  ph.calibration(voltage, temperature);

  delay(2000);  // Wait 2 seconds before the next loop
}

void sendToServer(String endpoint, String postData) {
  Serial.println("Sending data: " + postData);
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server: " + endpoint);

    // Send HTTP POST request
    client.println("POST " + endpoint + " HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.print(postData);

    // Read server response
    Serial.println("Server response:");
    while (client.available()) {
      String response = client.readStringUntil('\n');
      Serial.println(response);
    }

    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection to server failed: " + endpoint);
  }
}
