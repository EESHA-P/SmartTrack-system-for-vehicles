#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 Configuration
#define ONE_WIRE_BUS 15       // GPIO15 for DS18B20 data
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Potentiometer Configuration
#define POT_PIN 34            // Analog pin for potentiometer

// Pin Definitions
#define ALERT_LED 2           // LED pin
#define BUZZER_PIN 4          // Buzzer pin

// Mock OBD-II parameters with initial values
String engineRPM = "1200";
String coolantTemp = "82";
String batteryVoltage = "12.6";
String throttlePosition = "15";
String fuelLevel = "75";
String timingAdvance = "12.5";
String engineLoad = "34";
String speed = "0";
bool engineCheck = false;

// Mock GPS parameters
float latitude = 12.971598; // Example: Bangalore, India
float longitude = 77.594566;

// DTC Management
String activeDTCs[3] = {"", "", ""};
bool hasDTCs = false;
int dtcCounter = 0;

// Buzzer test state
bool buzzerTestMode = true;
unsigned long buzzerTestStartTime = 0;
const unsigned long BUZZER_TEST_DURATION = 5000; // 5 seconds test duration

// Debug flag
bool debugMode = true;

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  
  sensors.begin(); // Initialize DS18B20
  
  Serial.println("\n============================================");
  Serial.println("Enhanced Vehicle Diagnostics System");
  Serial.println("============================================");
  
  // Buzzer test sequence
  Serial.println("Starting buzzer test sequence...");
  buzzerTestStartTime = millis();
}

void loop() {
  // First, handle buzzer test if active
  if (buzzerTestMode) {
    runBuzzerTest();
    return; // Skip regular monitoring during test
  }
  
  // Read actual temperature from DS18B20
  sensors.requestTemperatures();
  float currentTemp = sensors.getTempCByIndex(0);

  // Read potentiometer value and map it to battery voltage range (11.0V - 13.0V)
  int potValue = analogRead(POT_PIN);
  // Improved mapping for more intuitive control
  float voltage = map(potValue, 0, 4095, 110, 130) / 10.0;
  batteryVoltage = String(voltage, 1); // Format to 1 decimal place
  
  if (debugMode) {
    Serial.print("Potentiometer raw value: ");
    Serial.print(potValue);
    Serial.print(", Mapped battery voltage: ");
    Serial.println(batteryVoltage);
  }

  // Update simulated OBD-II parameters with realistic values
  updateOBDParameters(currentTemp);
  
  // Check and generate DTCs based on current parameters
  checkAndGenerateDTCs(currentTemp);
  
  // Display enhanced dashboard
  displayEnhancedDashboard(currentTemp);
  
  // If there are active DTCs, display them
  if (hasDTCs) {
    displayDTCs();
  }
  
  delay(5000); // Update every cycle (5 seconds)
}

void updateOBDParameters(float currentTemp) {
  int baseRPM = 800; // Idle RPM
  int throttle = throttlePosition.toInt();
  
  int rpm;
  
  if (engineCheck) {
    rpm = baseRPM + random(0, 200) + (throttle * random(10,20));
  } else {
    rpm = baseRPM + throttle * random(50,100); 
  }
  engineRPM = String(rpm);

  // Calculate new fuel level (ensure non-negative)
  int newFuel = fuelLevel.toInt() - random(0, 2);
  if (newFuel < 0) newFuel = 0;
  fuelLevel = String(newFuel);
  
  coolantTemp = String((int)currentTemp); 
  timingAdvance = String(8 + (random(0, 10) / 2.0));
  engineLoad = String(20 + (throttle / 2) + random(0, 15));

  // Fixed speed calculation to ensure non-negative values
  int currentSpeed = speed.toInt();

  if (throttle > 10) {
    currentSpeed = currentSpeed + random(-2, 5);
  } else {
    currentSpeed = currentSpeed - random(1, 4);
  }
  // Ensure speed stays within valid range
  if (currentSpeed < 0) currentSpeed = 0;
  if (currentSpeed > 120) currentSpeed = 120;
  
  speed = String(currentSpeed);

  if (currentSpeed > 0) {
    latitude += (random(-10, 10) / 10000.0);
    longitude += (random(-10, 10) / 10000.0);
  }
}

void checkAndGenerateDTCs(float currentTemp){
  bool dtcStatusChanged = false;
  
  // Temperature-based DTC
  if (currentTemp > 40.0) {
    if (addDTC("P0118", "Engine Coolant Temperature Circuit High")) {
      dtcStatusChanged = true;
    }
    // Also trigger an alert when temperature is critical
    sendAlert("ENGINE OVERHEATING: " + String(currentTemp, 1) + "°C");
  } else {
    if (removeDTC("P0118")) {
      dtcStatusChanged = true;
    }
  }
  
  // Battery voltage DTC
  if (batteryVoltage.toFloat() < 11.8) {
    if (addDTC("P0562", "System Voltage Low")) {
      dtcStatusChanged = true;
    }
    // Also trigger an alert when battery voltage is low
    sendAlert("LOW BATTERY VOLTAGE: " + batteryVoltage + "V");
  } else {
    if (removeDTC("P0562")) {
      dtcStatusChanged = true;
    }
  }
  
  // Throttle position sensor issue
  if (throttlePosition.toInt() < 5 && speed.toInt() > 30) {
    if (addDTC("P0123", "Throttle Position Sensor High Input")) {
      dtcStatusChanged = true;
    }
  } else {
    if (removeDTC("P0123")) {
      dtcStatusChanged = true;
    }
  }
  
  // Set engine check light based on DTC presence
  engineCheck = hasDTCs;
  
  // Only clear DTCs periodically if there's no indication to keep them
  dtcCounter++;
  if (dtcCounter >= 20 && !hasDTCs) {
    clearDTCs();
    dtcCounter = 0;
  }
}

// Improved DTC management with return value indicating whether a change occurred
bool addDTC(String code, String description) {
  // Check if DTC already exists
  for (int i = 0; i < 3; i++) {
    if (activeDTCs[i].startsWith(code)) {
      return false; // DTC already exists, no change
    }
  }
  
  // Find an empty slot for the DTC
  for (int i = 0; i < 3; i++) {
    if (activeDTCs[i] == "") {
      activeDTCs[i] = code + ": " + description;
      hasDTCs = true;
      return true; // DTC added, change occurred
    }
  }
  
  return false; // No slot available, no change
}

// Improved removeDTC function with return value
bool removeDTC(String code) {
  for (int i = 0; i < 3; i++) {
    if (activeDTCs[i].startsWith(code)) {
      // Clear the DTC slot
      activeDTCs[i] = "";
      
      // Shift remaining DTCs up to fill the gap
      for (int j = i; j < 2; j++) {
        activeDTCs[j] = activeDTCs[j + 1];
      }
      activeDTCs[2] = ""; // Clear the last slot
      
      // Check if any DTCs are still active
      hasDTCs = false;
      for (int k = 0; k < 3; k++) {
        if (activeDTCs[k] != "") {
          hasDTCs = true;
          break;
        }
      }
      
      return true; // DTC was removed, change occurred
    }
  }
  
  return false; // DTC wasn't found, no change
}

void clearDTCs() {
  for (int i = 0; i < 3; i++) {
    activeDTCs[i] = "";
  }
  hasDTCs = false;
  engineCheck = false;
}

void runBuzzerTest() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - buzzerTestStartTime;
  
  if (elapsedTime < BUZZER_TEST_DURATION) {
    // Alternate buzzer on/off every 500ms during test period
    if ((elapsedTime / 500) % 2 == 0) {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("Buzzer Test: ON");
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("Buzzer Test: OFF");
    }
    delay(500);
  } else {
    // End test mode after duration expires
    buzzerTestMode = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Buzzer test completed. Starting regular monitoring...");
  }
}

void sendAlert(String message) {
  digitalWrite(ALERT_LED, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  
  Serial.println("\n⚠️ ALERT ⚠️");
  Serial.println(message);
  Serial.println("Location: " + String(latitude, 6) + ", " + String(longitude, 6));
  
  delay(2000);
  
  digitalWrite(BUZZER_PIN, LOW);
  // Keep LED on as long as there are active DTCs
}

void displayEnhancedDashboard(float temp) {
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.println("│      VEHICLE DIAGNOSTICS        │");
  Serial.println("├─────────────────────────────────┤");
  
  Serial.print("│ RPM: ");
  Serial.print(engineRPM);
  printSpaces(13 - engineRPM.length());
  Serial.print("│ Coolant: ");
  String tempStr = String(temp, 1) + "°C";
  Serial.print(tempStr);
  printSpaces(11 - tempStr.length());
  Serial.println("│");
  
  Serial.print("│ Throttle: ");
  Serial.print(throttlePosition + "%");
  printSpaces(8 - throttlePosition.length());
  Serial.print("│ Battery: ");
  Serial.print(batteryVoltage + "V");
  printSpaces(11 - batteryVoltage.length());
  Serial.println("│");
  
  Serial.print("│ Fuel: ");
  Serial.print(fuelLevel + "%");
  printSpaces(11 - fuelLevel.length());
  Serial.print("│ Speed: ");
  Serial.print(speed + " km/h");
  printSpaces(11 - speed.length());
  Serial.println("│");
  
  Serial.println("├────────────────┴────────────────┤");
  Serial.println("│ DIAGNOSTIC STATUS               │");
  Serial.println("├─────────────────────────────────┤");
  Serial.print("│ Check Engine: ");
  Serial.print(engineCheck ? "ON " : "OFF");
  printSpaces(17);
  Serial.println("│");
  
  Serial.print("│ Temp Status: ");
  if (temp > 40.0) {
    Serial.print("CRITICAL");
    printSpaces(11);
  } else if (temp > 30.0) {
    Serial.print("WARNING");
    printSpaces(13);
  } else if (temp > 20.0) {
    Serial.print("NORMAL");
    printSpaces(14);
  } else {
    Serial.print("COLD");
    printSpaces(16);
  }
  Serial.println("│");
  
  Serial.println("└─────────────────────────────────┘");
}

void displayDTCs() {
  Serial.println("\n┌─────────────────────────────────┐");
  Serial.println("│ DIAGNOSTIC TROUBLE CODES        │");
  Serial.println("├─────────────────────────────────┤");
  
  bool hasPrinted = false;
  
  for (int i = 0; i < 3; i++) {
    if (activeDTCs[i] != "") {
      hasPrinted = true;
      Serial.print("│ ");
      Serial.print(activeDTCs[i]);
      printSpaces(31 - activeDTCs[i].length());
      Serial.println("│");
    }
  }
  
  if (!hasPrinted) {
    Serial.println("│ No active DTCs                   │");
  }
  
  Serial.println("└─────────────────────────────────┘");
}

void printSpaces(int count) {
  if (count < 0) count = 0; // Safety check
  for (int i = 0; i < count; i++) {
    Serial.print(" ");
  }
}

