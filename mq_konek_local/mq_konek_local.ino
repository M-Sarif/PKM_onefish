#include <WiFi.h>

const char* ssid = "wifi ono";       // Ganti dengan SSID WiFi Anda
const char* password = "wifi kencang gak bikin kembung"; // Ganti dengan password WiFi Anda
const char* serverIP = "192.168.162.213"; // Ganti dengan IP localhost Anda
const int serverPort = 80;             // Port server (default HTTP port adalah 80)
const char* serverPath = "/pkm/koneksi/koneksi_mq.php";

const int sensorPin = A0; // Pin analog tempat MQ-137 terhubung
const float VREF = 5.0;   // Tegangan referensi ADC
const int ADC_RESOLUTION = 1025; // Resolusi ADC

// Konstanta berdasarkan datasheet MQ-137
const float RL = 10.0;    // Resistor beban dalam kilo-ohm
const float RO = 10.0;    // Nilai RO setelah kalibrasi
const float a = -2.4;     // Gradient dari kurva logaritmik (dari datasheet)
const float b = 0.7;      // Intercept dari kurva logaritmik (dari datasheet)

WiFiClient client;

void setup() {
  Serial.begin(9600);
  Serial.println("MQ-137 Sensor - NH3 Concentration");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  // Membaca nilai analog dari sensor
  int sensorValue = analogRead(sensorPin);

  // Mengonversi nilai analog ke tegangan
  float sensorVoltage = sensorValue * (VREF / ADC_RESOLUTION);

  // Menghitung rasio Rs/Ro
  float Rs = ((VREF * RL) / sensorVoltage) - RL; // Menghitung Rs
  float ratio = Rs / RO;

  // Menghitung konsentrasi NH₃ dalam ppm menggunakan persamaan logaritmik
  float ppm = pow(10, ((log10(ratio) - b) / a));

  // Menampilkan hasil di Serial Monitor
  Serial.print("Nilai Analog: ");
  Serial.print(sensorValue);
  Serial.print(" | Tegangan: ");
  Serial.print(sensorVoltage, 2);
  Serial.print(" V | NH3: ");
  Serial.print(ppm, 2);
  Serial.println(" ppm");

  // Kirim data ke server
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");

    // Membuat data POST
    String postData = "sensor_value=" + String(sensorValue) + "&voltage=" + String(sensorVoltage, 2) + "&ppm=" + String(ppm, 2);

    // Membuat permintaan HTTP POST
    client.println("POST " + String(serverPath) + " HTTP/1.1");
    client.println("Host: " + String(serverIP));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println("Content-Length: " + String(postData.length()));
    client.println();
    client.print(postData);

    // Menunggu respons dari server
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout!");
        client.stop();
        return;
      }
    }

    // Membaca respons dari server
    Serial.println("Server response:");
    while (client.available()) {
      String response = client.readStringUntil('\n');
      Serial.println(response);
    }

    // Menutup koneksi
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection to server failed");
  }

  delay(1000); // Menunggu 1 detik sebelum pembacaan berikutnya
}
