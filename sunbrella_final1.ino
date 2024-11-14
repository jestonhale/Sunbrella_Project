//ethanwashere
//#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <ArduinoBLE.h>

/*
#define SDA;
#define SCL;
*/

BLEService motorService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic motorControlCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLEWriteWithoutResponse, 20);

//LiquidCrystal_I2C lcd (0x27, 16, 2);

const int stepsPerRevolution = 5000; //Number of steps per full rotation
int currentStep1 = 0; //variable for tracking current step position of tilting motor
int currentStep2 = 0; //variable for tracking current step position of base motor

// Constants for the light sensor range
const int minLightValue = 0;     // Minimum possible light value
const int maxLightValue = 1023;  // Maximum possible light value (for 10-bit ADC)

// Define pins for the stepper motors
const int stepPin1 = 2;  // Stepper 1 step pin
const int dirPin1 = 3;   // Stepper 1 direction pin
const int stepPin2 = 4;  // Stepper 2 step pin
const int dirPin2 = 5;   // Stepper 2 direction pin

Stepper stepper1(stepsPerRevolution, stepPin1, dirPin1);
Stepper stepper2(stepsPerRevolution, stepPin2, dirPin2);

// Define analog pins for the photoresistors
const int photoResistorTopRight = A0;
const int photoResistorTopLeft = A1;
const int photoResistorBottomRight = A2;
const int photoResistorBottomLeft = A3;

// Initialize motor moving flags
bool motor1Moving = false;
bool motor2Moving = false;

// Control mode flag: true when controlled by the app, false when controlled by photoresistors
bool appControlMode = false;

// Motor speed control variables for app mode
int motor1Speed = 1000;
int motor2Speed = 1000;
int motor1Direction = 1;  // 1 for CW, -1 for CCW, 0 for stop
int motor2Direction = 1;  // 1 for CW, -1 for CCW, 0 for stop

// Timer for checking BLE updates
unsigned long lastBLECheck = 0;
const unsigned long BLECheckInterval = 50; // Check BLE every 50ms
bool motor1CommandArrived = false;
bool motor2CommandArrived = false;

// New flag to control the automatic mode
bool autoModeEnabled = false;  

void setup() {
  //stepper2.setSpeed(100);
  Serial.begin(115200);  // Start serial communication
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Failed to start BLE!");
    while (1);
  }

  BLE.setLocalName("ArduinoMotorController");
  BLE.setAdvertisedService(motorService);

  motorService.addCharacteristic(motorControlCharacteristic);
  BLE.addService(motorService);

  BLE.advertise();
  Serial.println("BLE device is now advertising...");

    // Set stepper motor pins as outputs
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
}

void controlMotorsWithPhotoresistors() {
 
 
  // Read the analog values from the photoresistors
  int lightTopRight = analogRead(photoResistorTopRight);
  int lightTopLeft = analogRead(photoResistorTopLeft);
  int lightBottomRight = analogRead(photoResistorBottomRight);
  int lightBottomLeft = analogRead(photoResistorBottomLeft);

  // Mapping analog values to a more compact range from 0 to 100
  int NorthEast = map(lightTopRight, minLightValue, maxLightValue, 100, 0);
  int SouthEast = map(lightBottomRight, minLightValue, maxLightValue, 100, 0);
  int NorthWest = map(lightTopLeft, minLightValue, maxLightValue, 100, 0);
  int SouthWest = map(lightBottomLeft, minLightValue, maxLightValue, 100, 0);

  // Print the light values to the Serial Monitor
  Serial.print("TR: ");
  Serial.print(NorthEast);
  Serial.print(", TL: ");
  Serial.print(NorthWest);
  Serial.print(", BR: ");
  Serial.print(SouthEast);
  Serial.print(", BL: ");
  Serial.print(SouthWest);
  Serial.print("  -  Position1 #: ");
  Serial.println(currentStep1);
  Serial.print("  -  Position2 #: ");
  Serial.println(currentStep2);


  // Step 1: Handle Motor 1 first
  if (abs(NorthEast - NorthWest) >= 3 && (NorthEast > NorthWest)) {
    moveMotor1CL();    // Rotate motor toward NorthEast when NorthEast is greater
  } else if (abs(NorthEast - NorthWest) >= 3 && (NorthEast < NorthWest)) {
    moveMotor1CCL();   // Rotate motor toward NorthWest when NorthWest is greater
  } else {
    motor1Moving = false;  // Stop Motor 1
  }

  // Step 2: Handle Motor 2 only if TR and TL are within the range of 6
  if (abs(NorthEast - NorthWest) <= 10) {  // Check if TR and TL are within the range of 6
    if (abs(NorthEast - SouthEast) >= 10 && (NorthEast > SouthEast)) {
      moveMotor2CCL();  // Rotate motor toward NorthEast when NorthEast is greater
    } else if (abs(NorthEast - SouthEast) >= 10 && (NorthEast < SouthEast)) {
      moveMotor2CL(); // Rotate motor toward SouthEast when SouthEast is greater
    } else {
      motor2Moving = false; // Stop Motor 2
    }
  } else {
    motor2Moving = false;  // Ensure Motor 2 does not move if TR and TL are not within range
  }

}

void moveMotor1CCL(){
  motor1Moving = true;         //Move Motor 1 if NE and NW are greater than 5 of each other
  digitalWrite(dirPin1, LOW);  // Set direction to one side
  if(currentStep1 > -1500){
    digitalWrite(stepPin1, HIGH);
    delay(1);  // Adjust delay for motor speed
    digitalWrite(stepPin1, LOW);
    delay(1);  // Adjust delay for motor speed
    currentStep1 -= 1;
  }
}
void moveMotor1CL(){
  motor1Moving = true;         //Move Motor 1 if NE and NW are greater than 5 of each other
  digitalWrite(dirPin1, HIGH);  // Set direction to other side
  if(currentStep1 < 1500){
    digitalWrite(stepPin1, HIGH);
    delay(1);  // Adjust delay for motor speed
    digitalWrite(stepPin1, LOW);
    delay(1);  // Adjust delay for motor speed
    currentStep1 += 1;
  }
}
//}
void moveMotor2CCL(){
  motor2Moving = true;         //Move Motor 1 if NE and NW are greater than 5 of each other
  digitalWrite(dirPin2, LOW);  // Set direction to one side
  if(currentStep2 > -1500){
    digitalWrite(stepPin2, HIGH);
    delay(1);  // Adjust delay for motor speed
    digitalWrite(stepPin2, LOW);
    delay(1);  // Adjust delay for motor speed
    currentStep2 -= 1;
  }
}
void moveMotor2CL(){
  if(currentStep2 < 1500){
  motor2Moving = true;         //Move Motor 1 if NE and NW are greater than 5 of each other
  digitalWrite(dirPin2, HIGH);  // Set direction to other side
    digitalWrite(stepPin2, HIGH);
    delay(1);  // Adjust delay for motor speed
    digitalWrite(stepPin2, LOW);
    delay(1);  // Adjust delay for motor speed
    currentStep2 += 1;    
  }
}

void resetMotorPosition() {
  // Reset motor 1
  if (currentStep1 != 0) {
    if (currentStep1 > 0) {
      // Move Motor 1 towards zero in a negative direction
      digitalWrite(dirPin1, LOW);
      digitalWrite(stepPin1, HIGH);
      delayMicroseconds(1000);  // Adjust speed as needed
      digitalWrite(stepPin1, LOW);
      delayMicroseconds(1000);
      currentStep1 -= 1;
    } else {
      // Move Motor 1 towards zero in a positive direction
      digitalWrite(dirPin1, HIGH);
      digitalWrite(stepPin1, HIGH);
      delayMicroseconds(1000);
      digitalWrite(stepPin1, LOW);
      delayMicroseconds(1000);
      currentStep1 += 1;
    }
  }

  // Reset motor 2
  if (currentStep2 != 0) {
    if (currentStep2 > 0) {
      // Move Motor 2 towards zero in a negative direction
      digitalWrite(dirPin2, LOW);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(1000);  // Adjust speed as needed
      digitalWrite(stepPin2, LOW);
      delayMicroseconds(1000);
      currentStep2 -= 1;
    } else {
      // Move Motor 2 towards zero in a positive direction
      digitalWrite(dirPin2, HIGH);
      digitalWrite(stepPin2, HIGH);
      delayMicroseconds(1000);
      digitalWrite(stepPin2, LOW);
      delayMicroseconds(1000);
      currentStep2 += 1;
    }
  }
}

//


void controlMotorsWithApp() {
  // Only check BLE at specified intervals
  if (millis() - lastBLECheck >= BLECheckInterval) {
    lastBLECheck = millis();
    checkBLECommands();
  }

    

  // Control motors with direct pin pulsing for faster response
 if (motor1CommandArrived){
    pulseMotor(stepPin1, dirPin1, motor1Direction, motor1Speed);
 }
  if(motor2CommandArrived){
    pulseMotor(stepPin2, dirPin2, motor2Direction, motor2Speed);
 }
  else if (!motor2CommandArrived,!motor1CommandArrived) {
    motor1CommandArrived = false;
    motor2CommandArrived = false;
  }
 
}

// Parse the command to set motor direction and speed
void parseAppCommands(String command) {
  if (command.startsWith("MOTOR1:")) {
    motor1Direction = command.charAt(7) == '-' ? -1 : 1;
    motor1Speed = max(2000, 2000 / command.substring(8).toInt());  // Map speed
    Serial.print("MOTOR1 set to direction ");
    Serial.print(motor1Direction);
    Serial.print(" with delay ");
    Serial.println(motor1Speed);
    motor1CommandArrived = true;
  } else if (command.startsWith("MOTOR2:")) {
    motor2Direction = command.charAt(7) == '-' ? -1 : 1;
    motor2Speed = max(2000, 2000 / command.substring(8).toInt());  // Map speed
    Serial.print("MOTOR2 set to direction ");
    Serial.print(motor2Direction);
    Serial.print(" with delay ");
    Serial.println(motor2Speed);
    motor2CommandArrived = true;
  }
 else if (command.startsWith("STOP")) {
    motor1CommandArrived = false;
    motor2CommandArrived = false;
    
  }
}

// Function to check for and parse BLE commands
void checkBLECommands() {
  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    if (motorControlCharacteristic.written()) {
      String command = String((char*)motorControlCharacteristic.value());
      Serial.print("Received command: ");
      Serial.println(command);
      parseAppCommands(command);
    }
  }
}

// Function to pulse the step pin for faster motor control
void pulseMotor(int stepPin, int dirPin, int direction, int speedDelay) {
  digitalWrite(dirPin, direction > 0 ? HIGH : LOW); // Set direction

  // Generate a pulse on the step pin
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(speedDelay);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(speedDelay);
}
/*
bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}



Wire.begin ();
  if (!i2CAddrTest (0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
    lcd.init();
    lcd.backlight ();
    lcd.setCursor (0,0);
    lcd.print ("Sunbrella Script");
   // delay(2000);
  }
*/


void loop() {
  // Check for BLE updates at specified intervals
  if (millis() - lastBLECheck >= BLECheckInterval) {
    lastBLECheck = millis();
    checkBLECommands();
  }

  // Control motors based on received commands
  if (appControlMode) {
    // Control motors via app commands
    if (motor1CommandArrived) {
      pulseMotor(stepPin1, dirPin1, motor1Direction, motor1Speed);
    }
    if (motor2CommandArrived) {
      pulseMotor(stepPin2, dirPin2, motor2Direction, motor2Speed);
    }
  } else if (autoModeEnabled) {
    // Control motors based on photoresistor values in automatic mode
    controlMotorsWithPhotoresistors();
  } else {
    // Stop motors if no commands have arrived
    motor1CommandArrived = false;
    motor2CommandArrived = false;
  }

  // Check for serial communication from the app
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');

    if (command == "APP_CONNECTED") {
      appControlMode = true;        // Enable app control
      autoModeEnabled = false;      // Disable auto mode if app control is active
    } else if (command == "Failed to start BLE!" || command == "APP_DISCONNECTED") {
      appControlMode = false;       // Switch back to other control modes
    } else if (command == "RESET") {
      appControlMode = false;       // Disable app control during reset
      autoModeEnabled = false;      // Disable auto mode during reset
      Serial.println("Resetting position...");

      // Continue resetting until both currentStep1 and currentStep2 reach zero
      while (currentStep1 != 0 || currentStep2 != 0) {
        resetMotorPosition();
      }

      Serial.println("Reset complete, awaiting new commands.");
    } else if (command == "AUTOMODE") {
      appControlMode = false;       // Ensure app control is off in auto mode
      autoModeEnabled = true;       // Enable automatic mode with photoresistors
      Serial.println("Automatic mode activated with photoresistor control.");
    }
  }

  //}

if (appControlMode) {
    // Control the motors based on app commands
    controlMotorsWithApp();
}
  if (autoModeEnabled) {
    // Control the motors based on photoresistor readings in automatic mode
    controlMotorsWithPhotoresistors();
 } else {
    motor1Moving = false;
    motor2Moving = false;
    }
  }