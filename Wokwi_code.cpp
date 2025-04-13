#include <Arduino.h>

// Mock OBD-II parameters
String engineRPM = "2500";
String coolantTemp = "95";
String batteryVoltage = "12.6";
bool engineCheck = false;

// Mock GPS parameters
float latitude = 12.971598; // Example: Bangalore, India
float longitude = 77.594566;
float speed = 45.0; // Speed in km/h

// Alert LED pin
#define ALERT_LED 2

void setup() {
  Serial.begin(115200); // Initialize Serial Monitor for debugging
  pinMode(ALERT_LED, OUTPUT); // Set GPIO2 as output for LED

  Serial.println("\n============================================");
  Serial.println("Integrated Vehicle Tracking & Diagnostics System");
  Serial.println("============================================\n");

  // Simulate initialization sequence
  Serial.println("Initializing OBD-II interface...");
  delay(1000);
  Serial.println("OBD-II connected successfully");

  Serial.println("Initializing GPS module...");
  delay(1000);
  Serial.println("GPS module ready");

  Serial.println("\nSystem ready! Starting diagnostics...\n");
}

void loop() {
  // Simulate random changes in OBD-II data
  engineRPM = String(2000 + random(0, 1000)); // Random RPM between 2000-3000
  coolantTemp = String(90 + random(0, 15));   // Random temp between 90-105°C
  batteryVoltage = String(12.0 + (random(0, 10) / 10.0)); // Random voltage between 12.0-13.0
  
  // Simulate random GPS movement
  latitude += (random(-10, 10) / 10000.0); // Simulate small latitude changes
  longitude += (random(-10, 10) / 10000.0); // Simulate small longitude changes
  speed = 40 + random(0, 20); // Random speed between 40-60 km/h

  // Check for critical conditions
  if (coolantTemp.toInt() > 100) {
    sendAlert("ENGINE OVERHEATING: " + coolantTemp + "°C");
    engineCheck = true;
  } else if (batteryVoltage.toFloat() < 11.8) {
    sendAlert("LOW BATTERY VOLTAGE: " + batteryVoltage + "V");
    engineCheck = true;
  } else {
    engineCheck = false;
    digitalWrite(ALERT_LED, LOW); // Turn off LED if no alert
  }

  // Display diagnostics and tracking data on Serial Monitor
  displayDashboard();

  delay(5000); // Update every 5 seconds
}

// Simulate sending an alert via GSM (mocked)
void sendAlert(String message) {
  digitalWrite(ALERT_LED, HIGH); // Turn on alert LED
  
  Serial.println("\n⚠️ ALERT ⚠️");
  Serial.println(message);

  // Simulate SMS alert (mocked)
  Serial.println("Sending SMS alert to fleet manager...");
  Serial.println("SMS: " + message + " at Lat:" + String(latitude,6) + ", Lng:" + String(longitude,6));
}

// Display diagnostics and tracking data on Serial Monitor
void displayDashboard() {
  Serial.println("\n===== VEHICLE DIAGNOSTICS & TRACKING =====");
  
  // OBD-II Data Section
  Serial.println("------ OBD-II DIAGNOSTICS ------");
  Serial.println("Engine RPM: " + engineRPM);
  Serial.println("Coolant Temp: " + coolantTemp + "°C");
  Serial.println("Battery Voltage: " + batteryVoltage + "V");
  
  // GPS Tracking Section
  Serial.println("------ GPS TRACKING ------");
  Serial.println("Latitude: " + String(latitude,6));
  Serial.println("Longitude: " + String(longitude,6));
  
}
