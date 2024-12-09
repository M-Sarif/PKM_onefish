#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>

#define PH_PIN A1
float voltage, phValue, temperature = 25;
DFRobot_PH ph;

const int mq137Pin = A0;

const float RLOAD = 10.0;
const float RZERO = 76.63;
const float PARA = 116.6020682;
const float PARB = 2.769034857;

const char ssid[] = "MyHome";
const char pass[] = "Ailarisa672";

const char server[] = "onefish.my.id";
const int port = 443;

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, server, port);

void setup() {
    Serial.begin(115200);
    ph.begin();
    
    connectWiFi();
}

void loop() {
    static unsigned long timepoint = millis();
    if (millis() - timepoint > 1000U) {
        timepoint = millis();
        
        if (WiFi.status() != WL_CONNECTED) {
            connectWiFi();
        }
        
        voltage = analogRead(PH_PIN) / 1024.0 * 4440;
        phValue = ph.readPH(voltage, temperature);
        
        int analogValue = analogRead(mq137Pin);
        float mqVoltage = (analogValue / 1024.0) * 3.0;
        float rs = (5.0 - mqVoltage) / mqVoltage * RLOAD;
        float ratio = rs / RZERO;
        float ppm = PARA * pow(ratio, -PARB);

        sendDataToServer(temperature, phValue, analogValue, mqVoltage, rs, ppm);
    }
    ph.calibration(voltage, temperature);
}

void connectWiFi() {
    Serial.println("Menghubungkan ke WiFi...");
    WiFi.begin(ssid, pass);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nTerhubung ke WiFi");
    } else {
        Serial.println("\nGagal terhubung ke WiFi. Mencoba lagi nanti.");
    }
}

void sendDataToServer(float temp, float ph, int analogVal, float voltage, float resistance, float ppm) {
    String postData = "temperature=" + String(temp, 1) + 
                      "&ph=" + String(ph, 2) + 
                      "&analogValue=" + String(analogVal) +
                      "&voltage=" + String(voltage) +
                      "&resistance=" + String(resistance) +
                      "&ppm=" + String(ppm);
    
    client.setTimeout(10000);  // Set timeout to 10 seconds
    
    int retries = 3;
    while (retries > 0) {
        client.beginRequest();
        client.post("/koneksi/koneksi.php");
        client.sendHeader("Content-Type", "application/x-www-form-urlencoded");
        client.sendHeader("Content-Length", postData.length());
        client.beginBody();
        client.print(postData);
        client.endRequest();
        
        int statusCode = client.responseStatusCode();
        String response = client.responseBody();
        
        if (statusCode == 200) {
            Serial.println("Data berhasil dikirim");
            Serial.println("Response: " + response);
            return;
        } else {
            Serial.println("Gagal mengirim data. Status code: " + String(statusCode));
            Serial.println("Response: " + response);
            retries--;
            if (retries > 0) {
                Serial.println("Mencoba lagi dalam 5 detik...");
                delay(5000);
            }
        }
    }
    
    Serial.println("Gagal mengirim data setelah 3 percobaan.");
}