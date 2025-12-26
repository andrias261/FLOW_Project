// ===================================
// DEFINISI STATE
// ===================================
enum Gerakan {M1_ON, JEDA_M1_M2, M2_ON, M3_ON, EMERGENCY_STOP};
Gerakan stateSekarang = M1_ON;

// ===================================
// INISIALISASI SENSOR
// ===================================
//INA219
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;
//A02YYUW 
#define A02_BAUDRATE 9600
//JSN-SRT04
const int trigPin_SR04 = 7; 
const int echoPin_SR04 = 6; 

// ===================================
// INISIALISASI AKTUATOR
// ===================================
//MOTOR DRIVER
#define RPWM 17
#define LPWM 16
#define REN 22
#define LEN 24
//RELAY
#define POMPA_PIN 26

// ===================================
// VARIABEL KONVERSI
// ===================================
//Global Variabel
long duration_SR04;
int distance_cm_SR04 = 0; // Pembacaan Sensor (cm) untuk Ketinggian Air
int distance_mm_A02 = 0;
float distance_cm_A02 = 0.0; // Pembacaan Sensor (cm) untuk Volume Sampah
float busVoltage = 0.0;
float current_mA = 0.0;
float power_mW = 0.0;
bool kondisiAman = true;

// Variabel Hasil Konversi
float volume_sampah_liter = 0.0;
float ketinggian_air_cm = 0.0;

// Parameter Konversi
const float RUMUS_SAMP_A = 76.8;
const float RUMUS_SAMP_B = 0.96;
const float RUMUS_AIR_MAKS = 240.0; // Tinggi maksimal tangki air (240 cm)

// ===================================
// SETUP MILLIS
// ===================================
unsigned long waktuDulu_bacaSensor = 0;
unsigned long waktuDulu_interlock = 0;
unsigned long waktuDulu_state = 0;

const long sensorInterval = 100;
const long interlockInterval = 100;

// ===================================
// FUNGSI SETUP
// ===================================
void setup() {
  Serial.begin(9600);
  //Inisialisasi JSN-SRT04
  pinMode(trigPin_SR04, OUTPUT);
  pinMode(echoPin_SR04, INPUT);
  //Inisialisasi INA219
  Wire.begin(); 
  ina219.begin();
  // Inisialisasi A02YYUW (Serial3)
  Serial3.begin(A02_BAUDRATE);

  // Inisialisasi Aktuator
  pinMode(POMPA_PIN, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(REN, OUTPUT);
  pinMode(LEN, OUTPUT);

  digitalWrite(REN, HIGH);
  digitalWrite(LEN, HIGH);
}

// ===================================
// MAIN LOOP
// ===================================
void loop() {
  unsigned long currentMillis = millis();

  //TASK 1: Baca dan Kirim Data Sensor
  if (currentMillis - waktuDulu_bacaSensor >= sensorInterval) {
    //Upadate Waktu Skrng
    waktuDulu_bacaSensor = currentMillis;
    //Baca Sensor
    baca_sensor();
    //Kirim data
    //Kirim Data ke ESP32
  }

  //TASK 2: Interlock dan Notif Peringatan
  if (currentMillis - waktuDulu_interlock >= interlockInterval) {
    //Update waktu skrng
    waktuDulu_interlock = currentMillis;
    if (KONDISI EMERGENCY) {
      kondisiAman = false;
      //Kirim Notif Peringatan
      

      // Jika kritis dan sedang tidak di fase emergency, masuk ke emergency
      if(stateSekarang != EMERGENCY_STOP) {
        pindahState(EMERGENCY_STOP);
      } else {
        kondisiAman = true;
      }
    }
  }  
  //Mulai State
  startState(currentMillis);
}

// ===================================
// FUNGSI STATE
// ===================================
void startState(unsigned long current){
  switch(stateSekarang){

    case M1_ON:
      ln_turun(); //Linear Aktutaor Turun
      pompa_off(); //Pompa Mati
      if (current - waktuDulu_state >= 7000) { // 7 detik
        ln_stop(); //Linear Mati
        pindahState(JEDA_M1_M2);
      }
    break;

    //Delay dalam Air
    case JEDA_M1_M2:
      if (current - waktuDulu_state >= 5000) { // 5 detik
        pindahState(M2_ON);
      }
    break;

    case M2_ON:
      ln_naik(); //Linear Aktuator Naik
      pompa_off();
      if (current - waktuDulu_state >= 7000) { // 7 detik
        ln_stop(); //Linear Aktuator Mati
        pindahState(M3_ON);
      }
    break;

    case M3_ON:
      pompa_on(); //Pompa Menyala
      ln_stop();
      if (current - waktuDulu_state >= 60000) { // 1 menit
        pompa_off(); //Pompa Berhenti
        pindahState(M1_ON); // Kembali ke awal
      }
    break;

    case EMERGENCY_STOP:
      pompa_off(); 
      ln_naik();
      if (current - waktuDulu_state >= 7000) {
        ln_stop();
        pompa_off();
        if(kondisiAman == true){
          pindahState(M1_ON);
        }
      }
    break;
  }
}

void pindahState(Gerakan stateBaru) {
  stateSekarang = stateBaru;
  waktuDulu_state = millis(); // Reset timer setiap pindah state
}

// ===================================
// FUNGSI BACA SENSOR
// ===================================
void baca_sensor(){
  baca_ina219();
  baca_jsn04();
  baca_a02();
}

void baca_ina219(){
  busVoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
}

void baca_jsn04(){
  digitalWrite(trigPin_SR04, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin_SR04, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin_SR04, LOW);
  
  duration_SR04 = pulseIn(echoPin_SR04, HIGH, 30000); // Timeout 30ms
  // Jika pulseIn timeout, durasi akan 0
  if (duration_SR04 > 0) {
      distance_cm_SR04 = duration_SR04 / 58; 
  } else {
      distance_cm_SR04 = 0; // Data tidak valid
  }

  //Konversi ke ketinggian air
  if (distance_cm_SR04 > 0) {
        ketinggian_air_cm = RUMUS_AIR_MAKS - distance_cm_SR04;
        
        // Batasi nilai agar tidak melebihi tinggi tangki
        if (ketinggian_air_cm > RUMUS_AIR_MAKS) {
            ketinggian_air_cm = RUMUS_AIR_MAKS;
        }
        // Pastikan ketinggian tidak negatif (air di bawah sensor)
        if (ketinggian_air_cm < 0) {
            ketinggian_air_cm = 0.0;
        }
  } else {
        ketinggian_air_cm = 0.0; // Jika pembacaan error/0
  }
}

void baca_a02(){
  int distance_mm_temp = 0;
  distance_cm_A02 = 0.0;
  // Variabel untuk menyimpan 4 byte data dari sensor
  byte data_buffer[4]; 
  while (Serial3.available() >= 4) {
    if (Serial3.read() == 0xFF) {
      data_buffer[1] = Serial3.read(); 
      data_buffer[2] = Serial3.read(); 
      data_buffer[3] = Serial3.read(); 
      byte checksum = (0xFF + data_buffer[1] + data_buffer[2]) & 0xFF;
      if (checksum == data_buffer[3]) {
        distance_mm_temp = (data_buffer[1] << 8) | data_buffer[2];        
        // Konversi ke cm: mm / 10
        distance_cm_A02 = (float)distance_mm_temp / 10.0; 
      }
    }
  }

  //konversi ke liter sampah
  if (distance_cm_A02 > 0) {
        volume_sampah_liter = RUMUS_SAMP_A - (RUMUS_SAMP_B * distance_cm_A02);
        // Pastikan volume tidak negatif
        if (volume_sampah_liter < 0) {
            volume_sampah_liter = 0.0;
        }
  } else {
        volume_sampah_liter = 0.0; // Jika pembacaan error/0
  }
}

// ===================================
// FUNGSI GERAK AKTUATOR
// ===================================
void ln_naik(){
  analogWrite(RPWM, 255);
  analogWrite(LPWM, 0);
}

void ln_turun(){
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 255);
}

void ln_stop(){
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void pompa_on(){
  digitalWrite(POMPA_PIN, HIGH);
}

void pompa_off(){
  digitalWrite(POMPA_PIN, LOW);
}