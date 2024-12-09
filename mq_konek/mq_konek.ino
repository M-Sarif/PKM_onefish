#include <WiFi.h>

// WiFi credentials
const char* ssid = "wifi ono";
const char* password = "wifi kencang gak bikin kembung";

// Server endpoint (ubah menjadi alamat lokal)
const char* serverName = "http://192.168.162.213/pkm/koneksi/koneksi_mq.php";

// Definisikan pin analog untuk MQ137
const int mq137Pin = A0;

// Konstanta untuk kalibrasi
const float RLOAD = 10.0;  // Nilai resistor load (dalam kilo-ohm)
const float RZERO = 76.63; // Nilai RZERO (dalam kilo-ohm) berdasarkan kalibrasi
const float PARA = 116.6020682; // Konstanta untuk kurva MQ
const float PARB = 2.769034857; // Konstanta untuk kurva MQ

void setup() {
  // Inisialisasi komunikasi serial pada baud rate 9600
  Serial.begin(9600);
  // Tunggu hingga Serial siap (khususnya penting untuk beberapa board)
  while (!Serial) {
    ;
  }
  Serial.println("MQ137 Sensor Initialization...");

  // Koneksi ke WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void loop() {
  // Baca nilai analog dari pin sensor
  int analogValue = analogRead(mq137Pin);
  // Konversi nilai analog ke tegangan
  float voltage = (analogValue / 1024.0) * 5.0;
  // Hitung nilai resistansi sensor
  float rs = (5.0 - voltage) / voltage * RLOAD;
  // Hitung rasio resistansi sensor terhadap resistansi awal
  float ratio = rs / RZERO;
  // Hitung konsentrasi amonia dalam ppm menggunakan persamaan kurva MQ
  float ppm = PARA * pow(ratio, -PARB);

  // Tampilkan nilai pembacaan di Serial Monitor
  Serial.print("NH3 Concentration: ");
  Serial.print(ppm);
  Serial.println(" ppm");

  // Kirim data ke server PHP
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    String url = String(serverName) + "?ppm=" + String(ppm);
    if (client.connect(serverName, 80)) {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + serverName + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          Serial.println(">>> Client Timeout !");
          client.stop();
          return;
        }
      }
      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
      Serial.println();
      Serial.println("Closing connection");
    } else {
      Serial.println("Failed to connect to server");
    }
    client.stop();
  }

  // Tunggu selama 1 detik sebelum pembacaan berikutnya
  delay(1000);
}