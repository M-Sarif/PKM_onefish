#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <WiFi.h>

#define PH_PIN A1
float voltage, phValue, temperature = 25;
DFRobot_PH ph;

// Definisikan pin analog untuk MQ137
const int mq137Pin = A0;

// Konstanta untuk kalibrasi MQ137
const float RLOAD = 10.0;  // Nilai resistor load (dalam kilo-ohm)
const float RZERO = 76.63; // Nilai RZERO (dalam kilo-ohm) berdasarkan kalibrasi
const float PARA = 116.6020682; // Konstanta untuk kurva MQ
const float PARB = 2.769034857; // Konstanta untuk kurva MQ

const char ssid[] = "wifi ono";          // your network SSID (name)
const char pass[] = "wifi kencang gak bikin kembung";      // your network password

const char server[] = "onefish.my.id";   // server IP address

WiFiClient client;

void setup() {
    Serial.begin(115200);
    ph.begin();
    
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
    }
    
    Serial.println("Connected to WiFi");
}

void loop() {
    static unsigned long timepoint = millis();
    if (millis() - timepoint > 1000U) {
        timepoint = millis();
        
        // Baca nilai pH
        voltage = analogRead(PH_PIN) / 1024.0 * 4440;
        phValue = ph.readPH(voltage, temperature);
        Serial.print("temperature: ");
        Serial.print(temperature, 1);
        Serial.print(" ^C  pH: ");
        Serial.println(phValue, 2);
        
        // Baca nilai dari sensor MQ137
        int analogValue = analogRead(mq137Pin);
        float mqVoltage = (analogValue / 1024.0) * 5.0;
        float rs = (5.0 - mqVoltage) / mqVoltage * RLOAD;
        float ratio = rs / RZERO;
        float ppm = PARA * pow(ratio, -PARB);
        Serial.print("Analog Value: ");
        Serial.print(analogValue);
        Serial.print(" | Voltage: ");
        Serial.print(mqVoltage);
        Serial.print(" V | Resistance: ");
        Serial.print(rs);
        Serial.print(" kΩ | NH3 Concentration: ");
        Serial.print(ppm);
        Serial.println(" ppm");

        // Kirim data pH ke server
        if (client.connect(server, 80)) {
            String postDataPH = "temperature=" + String(temperature, 1) + "&ph=" + String(phValue, 2);
            client.println("POST /pkm/koneksi/koneksi_ph.php HTTP/1.1");
            client.println("Host: " + String(server));
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(postDataPH.length());
            client.println();
            client.println(postDataPH);
            client.stop();
        }

        // Kirim data MQ137 ke server
        if (client.connect(server, 80)) {
            String postDataMQ = "analogValue=" + String(analogValue) +
                                "&voltage=" + String(mqVoltage) +
                                "&resistance=" + String(rs) +
                                "&ppm=" + String(ppm);
            client.println("POST /pkm/koneksi/koneksi_mq.php HTTP/1.1");
            client.println("Host: " + String(server));
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(postDataMQ.length());
            client.println();
            client.println(postDataMQ);
            client.stop();
        }
    }
    ph.calibration(voltage, temperature); // calibration process by Serial CMD
}
