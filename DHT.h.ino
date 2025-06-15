#include <DHT.h>

// === Konfigurasi Pin ===
#define DHTPIN1 27
#define DHTPIN2 26
#define DHTPIN3 25
#define DHTTYPE DHT22

#define SOIL1 33
#define SOIL2 32
#define SOIL3 35

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);

// === Fungsi Swap untuk Median ===
void swap(float &a, float &b) {
  float temp = a;
  a = b;
  b = temp;
}

// === Fungsi Median Voting dengan NaN Toleransi ===
float voter(float a, float b, float c) {
  float values[3];
  int count = 0;

  if (!isnan(a)) values[count++] = a;
  if (!isnan(b)) values[count++] = b;
  if (!isnan(c)) values[count++] = c;

  if (count == 3) {
    if (values[0] > values[1]) swap(values[0], values[1]);
    if (values[1] > values[2]) swap(values[1], values[2]);
    if (values[0] > values[1]) swap(values[0], values[1]);
    return values[1]; // Median
  } else if (count == 2) {
    return (values[0] + values[1]) / 2.0;
  } else if (count == 1) {
    return values[0];
  } else {
    return -1; // Semua error
  }
}

void setup() {
  Serial.begin(115200);
  dht1.begin();
  dht2.begin();
  dht3.begin();
  Serial.println("=== Sistem Monitoring Dimulai ===");
}

void loop() {
  // === Baca Sensor DHT ===
  float rawTemp1 = dht1.readTemperature();
  float rawTemp2 = dht2.readTemperature();
  float rawTemp3 = dht3.readTemperature();

  float rawHum1 = dht1.readHumidity();
  float rawHum2 = dht2.readHumidity();
  float rawHum3 = dht3.readHumidity();

  // Tampilkan warning jika ada error
  if (isnan(rawTemp1)) Serial.println("⚠ Sensor Suhu DHT1 error");
  if (isnan(rawTemp2)) Serial.println("⚠ Sensor Suhu DHT2 error");
  if (isnan(rawTemp3)) Serial.println("⚠ Sensor Suhu DHT3 error");
  if (isnan(rawHum1))  Serial.println("⚠ Sensor Humidity DHT1 error");
  if (isnan(rawHum2))  Serial.println("⚠ Sensor Humidity DHT2 error");
  if (isnan(rawHum3))  Serial.println("⚠ Sensor Humidity DHT3 error");

  // === Kalibrasi DHT ===
  float temp1 = rawTemp1 + 0.1;
  float temp2 = rawTemp2 + 0.6;
  float temp3 = rawTemp3 + 0.6;

  float hum1 = rawHum1 - 14.1;
  float hum2 = rawHum2 - 13.3;
  float hum3 = rawHum3 - 5.6;

  // === Baca Sensor Tanah ===
  int soil1_raw = analogRead(SOIL1);
  int soil2_raw = analogRead(SOIL2);
  int soil3_raw = analogRead(SOIL3);

  // Konversi analog soil ke persen kelembaban tanah (0-100%)
  float soil1 = (float)(4095 - soil1_raw) / 4095.0 * 100.0;
  float soil2 = (float)(4095 - soil2_raw) / 4095.0 * 100.0;
  float soil3 = (float)(4095 - soil3_raw) / 4095.0 * 100.0;

  // Bisa juga validasi kalau soil rusak (misalnya bacaannya terlalu kecil/aneh)
  if (soil1_raw < 100) soil1 = NAN;
  if (soil2_raw < 100) soil2 = NAN;
  if (soil3_raw < 100) soil3 = NAN;

  // === Voting ===
  float tempFinal = voter(temp1, temp2, temp3);
  float humFinal  = voter(hum1, hum2, hum3);
  float soilFinal = voter(soil1, soil2, soil3);

  Serial.println("\n=== DATA SENSOR ===");

  Serial.println("[Suhu (°C)]");
  Serial.printf("T1: %.2f\tT2: %.2f\tT3: %.2f\n", temp1, temp2, temp3);
  if (tempFinal == -1) Serial.println("-> Voting ERROR: Suhu tidak valid (minimal 1 harus terbaca)");
  else Serial.printf("-> Hasil Voting: %.2f°C ✅\n", tempFinal);

  Serial.println("\n[Kelembapan Udara (%)]");
  Serial.printf("H1: %.2f\tH2: %.2f\tH3: %.2f\n", hum1, hum2, hum3);
  if (humFinal == -1) Serial.println("-> Voting ERROR: Kelembapan tidak valid");
  else {
    humFinal = constrain(humFinal, 0, 100); // Batas logis kelembapan
    Serial.printf("-> Hasil Voting: %.2f%% ✅\n", humFinal);
  }

  Serial.println("\n[Kelembapan Tanah (%)]");
  Serial.printf("S1: %.2f\tS2: %.2f\tS3: %.2f\n", soil1, soil2, soil3);
  if (soilFinal == -1) Serial.println("-> Voting ERROR: Soil tidak valid");
  else Serial.printf("-> Hasil Voting: %.2f%% ✅\n", soilFinal);

  Serial.println("=============================");

  delay(3000); // Update tiap 12 detik
}