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

  // Tunggu selama 1 detik sebelum pembacaan berikutnya
  delay(1000);
}
