
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_FONA.h>

// OBD-II Config
#define OBD_RX 16
#define OBD_TX 17
SoftwareSerial obdSerial(OBD_RX, OBD_TX); // ESP32 pins for OBD-II

// GPS Config
#define GPS_RX 34
#define GPS_TX 35
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;

// GSM Config
#define FONA_RST 4
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
SoftwareSerial fonaSS = SoftwareSerial(25, 26); // RX, TX

// Alert LED
#define ALERT_LED 2

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED, OUTPUT);

  // Initialize OBD-II
  obdSerial.begin(9600);
  sendOBDCommand("ATZ"); // Reset OBD-II adapter
  delay(1000);
  sendOBDCommand("ATSP0"); // Auto-protocol

  // Initialize GPS
  gpsSerial.begin(9600);

  // Initialize GSM
  fonaSS.begin(4800);
  if (!fona.begin(fonaSS)) {
    Serial.println("GSM init failed");
    while (1);
  }
}

void loop() {
  // Read OBD-II data (e.g., engine RPM)
  String rpm = getOBDData("010C");
  Serial.print("Engine RPM: ");
  Serial.println(rpm);

  // Read GPS data
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        Serial.print("Lat: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Lng: ");
        Serial.println(gps.location.lng(), 6);
      }
    }
  }

  // Check for overheating (example threshold: 100°C)
  String coolantTemp = getOBDData("0105");
  if (coolantTemp.toInt() > 100) {
    sendAlert("OVERHEAT: " + coolantTemp + "°C");
  }

  delay(5000); // Update every 5 seconds
}

String getOBDData(String pid) {
  obdSerial.print(pid + "\r");
  delay(100);
  return obdSerial.readStringUntil('>');
}

void sendAlert(String message) {
  digitalWrite(ALERT_LED, HIGH);
  fona.sendSMS("+1234567890", message.c_str()); // Replace with your number
  delay(1000);
  digitalWrite(ALERT_LED, LOW);
}

void sendOBDCommand(String cmd) {
  obdSerial.print(cmd + "\r");
  delay(100);
  obdSerial.readStringUntil('>');
}
