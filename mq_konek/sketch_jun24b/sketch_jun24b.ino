#include <WiFi.h>  // Menggunakan library WiFi yang kompatibel dengan Arduino R4 WiFi

// Definisikan pin analog untuk MQ137
const int mq137Pin = A0;

// Konstanta untuk kalibrasi
const float RLOAD = 10.0;  // Nilai resistor load (dalam kilo-ohm)
const float RZERO = 76.63; // Nilai RZERO (dalam kilo-ohm) berdasarkan kalibrasi
const float PARA = 116.6020682; // Konstanta untuk kurva MQ
const float PARB = 2.769034857; // Konstanta untuk kurva MQ

const char ssid[] = "wifi ono";          // your network SSID (name)
const char pass[] = "wifi kencang gak bikin kembung";      // your network password

const char server[] = "http://onefish.my.id/";   // server IP address

WiFiClient client;

void setup() {
  // Inisialisasi komunikasi serial pada baud rate 9600
  Serial.begin(9600);
  // Tunggu hingga Serial siap (khususnya penting untuk beberapa board)
  while (!Serial) {
    ;
  }
  Serial.println("MQ137 Sensor Initialization...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
  }

  Serial.println("Connected to WiFi");
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
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.print(" V | Resistance: ");
  Serial.print(rs);
  Serial.print(" kΩ | NH3 Concentration: ");
  Serial.print(ppm);
  Serial.println(" ppm");

  // Kirim data ke server
  if (client.connect(server, 80)) {           // Connect to the server
    String postData = "analogValue=" + String(analogValue) +
                      "&voltage=" + String(voltage) +
                      "&resistance=" + String(rs) +
                      "&ppm=" + String(ppm);

    client.println("POST /pkm/koneksi/koneksi_mq.php HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    client.stop();
  }

  // Tunggu selama 1 detik sebelum pembacaan berikutnya
  delay(1000);
}
