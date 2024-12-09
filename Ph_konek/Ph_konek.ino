#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <WiFi.h>  

#define PH_PIN A1
float voltage, phValue, temperature = 25;
DFRobot_PH ph;

const char ssid[] = "wifi ono";          
const char pass[] = "wifi kencang gak bikin kembung";    

const char server[] = "192.168.162.213";  

WiFiClient client;

void setup()
{
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

void loop()
{
    static unsigned long timepoint = millis();
    if (millis() - timepoint > 1000U) {         
        timepoint = millis();
        
        voltage = analogRead(PH_PIN) / 1024.0 * 4440; 
        phValue = ph.readPH(voltage, temperature); 
        Serial.print("temperature:");
        Serial.print(temperature, 1);
        Serial.print("^C  pH:");
        Serial.println(phValue, 2);
        
        if (client.connect(server, 80)) {       
            String postData = "temperature=" + String(temperature, 1) + "&ph=" + String(phValue, 2);
            
            client.println("POST /pkm/koneksi/koneksi_ph.php HTTP/1.1");
            client.println("Host: " + String(server));
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.println(postData.length());
            client.println();
            client.println(postData);
            client.stop();
        }
    }
    ph.calibration(voltage, temperature);         
}

