#include <TinyGPS++.h>
#include <Adafruit_FONA.h>

// OBD-II Config
#define OBD_RX 16
#define OBD_TX 17
HardwareSerial obdSerial(1); // Use UART1 for OBD-II

// GPS Config
#define GPS_RX 34
#define GPS_TX 35
HardwareSerial gpsSerial(2); // Use UART2 for GPS
TinyGPSPlus gps;

// GSM Config
#define FONA_RST 4
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
HardwareSerial fonaSS(0); // Use UART0 for GSM (default Serial)

// Alert LED
#define ALERT_LED 2

void setup() {
  Serial.begin(115200); // Debugging on default Serial port
  
  pinMode(ALERT_LED, OUTPUT);

  // Initialize OBD-II
  obdSerial.begin(9600, SERIAL_8N1, OBD_RX, OBD_TX); // RX=16, TX=17
  sendOBDCommand("ATZ"); // Reset OBD-II adapter
  delay(1000);
  sendOBDCommand("ATSP0"); // Auto-protocol

  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX); // RX=34, TX=35

  // Initialize GSM
  fonaSS.begin(4800); // Default RX/TX pins for GSM module on Serial0
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

  char phoneNumber[] = "+1234567890"; // Replace with your number
  char smsMessage[160];
  message.toCharArray(smsMessage, sizeof(smsMessage));
  

  fona.sendSMS(phoneNumber, smsMessage);
  delay(1000);
  digitalWrite(ALERT_LED, LOW);
}


void sendOBDCommand(String cmd) {
  obdSerial.print(cmd + "\r");
  delay(100);
  obdSerial.readStringUntil('>');
}
