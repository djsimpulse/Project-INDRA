#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

/* PIN definitions */
#define PYRO1      6
#define PYRO2      7
#define LED        4
#define SD_CS      21
#define RFM_CS     10
#define RFM_RST    5
#define RFM_INT    3

Adafruit_BMP3XX bmp;
Adafruit_MPU6050 mpu;

/* Constants */
#define SEALEVELPRESSURE_HPA 1017.25

// ---- Launch detect ----
const float LAUNCH_ACCEL_G = 2.5;
const uint16_t LAUNCH_CONFIRM_MS = 50;
const uint32_t IGNORE_TIME_MS = 2000;

// ---- Altitude filter ----
const float ALT_ALPHA = 0.12;

// ---- Apogee detect ----
const uint8_t APOGEE_CONFIRM_COUNT = 4;
const uint32_t MIN_ASCENT_TIME_MS = 2500;
const uint32_t BACKUP_APOGEE_TIME_MS = 20500;

/* Flight state */
bool launched = false;
bool apogeeDetected = false;
bool pyroFired = false;

uint32_t bootTime = 0;
uint32_t launchTime = 0;
uint32_t lastSampleTime = 0;

float altitudeMax = 0;
uint8_t apogeeCounter = 0;

float altitudeRaw  = 0.0;
float altitudeFilt = 0.0;

/* SD card */
File logFile;

struct FlightLog {
  uint32_t time_ms;
  float altitude;
  float accelG;
  uint8_t launch;
  uint8_t apogee;
  uint8_t pyro;
};

/* RFM */
void rfInit() {
  pinMode(RFM_RST, OUTPUT);
  digitalWrite(RFM_RST, HIGH);
  delay(10);
  digitalWrite(RFM_RST, LOW);
  delay(10);
  digitalWrite(RFM_RST, HIGH);

  Serial.println("RF Init (stub)");
}

void rfSend(FlightLog &log) {
  Serial.println("RF TX:");
  Serial.print(log.time_ms); Serial.print(",");
  Serial.print(log.altitude); Serial.print(",");
  Serial.println(log.accelG);
}

/* Event trigger */
void firePyro2() {
  digitalWrite(PYRO2, HIGH);
  delay(200);
  digitalWrite(PYRO2, LOW);
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  bootTime = millis();

  pinMode(LED, OUTPUT);
  pinMode(PYRO1, OUTPUT);
  pinMode(PYRO2, OUTPUT);

  digitalWrite(PYRO1, LOW);
  digitalWrite(PYRO2, LOW);

  Wire.setSDA(8);
  Wire.setSCL(9);
  Wire.begin();

  Wire1.setSDA(14);
  Wire1.setSCL(15);
  Wire1.begin();

  /* bmp390 */
  bmp.begin_I2C(0x77, &Wire);
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_7);

  /* mpu6050 */
  mpu.begin(0x68, &Wire1);
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  SPI.setSCK(18);
  SPI.setTX(19);
  SPI.setRX(16);
  SPI.begin();

  /* sd */
  if (!SD.begin(SD_CS)) {
    Serial.println("SD FAIL");
    while (1);
  }

  logFile = SD.open("flight.csv", FILE_WRITE);
  logFile.println("time,altitude,accel,launch,apogee,pyro");

  /* rfm */
  rfInit();

  Serial.println("SYSTEM READY");
}

void loop() {
  uint32_t now = millis();
  if (now - lastSampleTime < 20) return;
  lastSampleTime = now;

  bmp.performReading();
  altitudeRaw = bmp.readAltitude(SEALEVELPRESSURE_HPA);

  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  float accelG = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  ) / 9.80665;

  /* filter */
  if (!launched) {
    altitudeFilt = altitudeRaw;
    altitudeMax  = altitudeRaw;
  } else {
    altitudeFilt = ALT_ALPHA * altitudeRaw +
                   (1.0 - ALT_ALPHA) * altitudeFilt;
  }

  /* launch detect */
  static uint32_t accelStart = 0;

  if (!launched &&
      now - bootTime > IGNORE_TIME_MS &&
      accelG > LAUNCH_ACCEL_G) {

    if (accelStart == 0) accelStart = now;

    if (now - accelStart >= LAUNCH_CONFIRM_MS) {
      launched = true;
      launchTime = now;
      altitudeMax = altitudeFilt;
      Serial.println("LAUNCH DETECTED");
    }
  } else {
    accelStart = 0;
  }

  /* apogee detect */
  if (launched && !apogeeDetected) {

    if (altitudeFilt > altitudeMax) {
      altitudeMax = altitudeFilt;
      apogeeCounter = 0;
    } else if ((altitudeFilt - altitudeMax) < -0.65) {
      apogeeCounter++;
    } else {
      apogeeCounter = 0;
    }

    bool altitudeBased =
      (apogeeCounter >= APOGEE_CONFIRM_COUNT) &&
      (now - launchTime > MIN_ASCENT_TIME_MS);

    bool timeBased =
      (now - launchTime > BACKUP_APOGEE_TIME_MS);

    if (altitudeBased || timeBased) {
      apogeeDetected = true;
      Serial.println("APOGEE DETECTED");
    }
  }

  /* event trigger */
  if (apogeeDetected && !pyroFired) {
    pyroFired = true;
    firePyro2();
    Serial.println("PYRO FIRED");
  }

  FlightLog log;
  log.time_ms = now;
  log.altitude = altitudeFilt;
  log.accelG = accelG;
  log.launch = launched;
  log.apogee = apogeeDetected;
  log.pyro   = pyroFired;

  logFile.print(log.time_ms); logFile.print(",");
  logFile.print(log.altitude); logFile.print(",");
  logFile.print(log.accelG); logFile.print(",");
  logFile.print(log.launch); logFile.print(",");
  logFile.print(log.apogee); logFile.print(",");
  logFile.println(log.pyro);
  logFile.flush();
  rfSend(log);
}