#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <ArduinoBLE.h>

BLEService motorService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLECharacteristic motorControlCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite | BLEWriteWithoutResponse, 20);

const int stepsPerRevolution = 5000; // Number of steps per full rotation
int currentStep1 = 0; // Variable for tracking current step position of tilting motor
int currentStep2 = 0; // Variable for tracking current step position of base motor

// Constants for light sensor range
const int minLightValue = 0;
const int maxLightValue = 1023;

// Define pins for the stepper motors
const int stepPin1 = 4;
const int dirPin1 = 5;
const int stepPin2 = 2;
const int dirPin2 = 3;

Stepper stepper1(stepsPerRevolution, stepPin1, dirPin1);
Stepper stepper2(stepsPerRevolution, stepPin2, dirPin2);

// Define analog pins for the photoresistors
const int photoResistorTopRight = A0;
const int photoResistorTopLeft = A1;
const int photoResistorBottomRight = A2;
const int photoResistorBottomLeft = A3;

bool motor1Moving = false;
bool motor2Moving = false;
bool appControlMode = true;
bool autoModeEnabled = false;  

int motor1Speed = 1000;
int motor2Speed = 1000;
int motor1Direction = 1;
int motor2Direction = 1;

unsigned long lastBLECheck = 0;
const unsigned long BLECheckInterval = 50;
bool motor1CommandArrived = false;
bool motor2CommandArrived = false;

void setup() {
  Serial.begin(115200);
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
  motor1Moving = true;
  digitalWrite(dirPin1, LOW);
  if(currentStep1 > -1500){
    digitalWrite(stepPin1, HIGH);
    delay(1);
    digitalWrite(stepPin1, LOW);
    delay(1);
    currentStep1 -= 1;
  }
}
void moveMotor1CL(){
  motor1Moving = true;
  digitalWrite(dirPin1, HIGH);
  if(currentStep1 < 1500){
    digitalWrite(stepPin1, HIGH);
    delay(1);
    digitalWrite(stepPin1, LOW);
    delay(1);
    currentStep1 += 1;
  }
}

void moveMotor2CCL(){
  motor2Moving = true;
  digitalWrite(dirPin2, LOW);
  if(currentStep2 > -1500){
    digitalWrite(stepPin2, HIGH);
    delay(1);
    digitalWrite(stepPin2, LOW);
    delay(1);
    currentStep2 -= 1;
  }
}

void moveMotor2CL(){
  if(currentStep2 < 1500){
    motor2Moving = true;
    digitalWrite(dirPin2, HIGH);
    digitalWrite(stepPin2, HIGH);
    delay(1);
    digitalWrite(stepPin2, LOW);
    delay(1);
    currentStep2 += 1;    
  }
}

void resetMotorPosition() {
   while (currentStep1 != 0 || currentStep2 != 0){
    if currentStep1 < 0{
    digitalWrite(dirPin2, HIGH)
    digitalWrite(stepPin2, HIGH);
    delay(1);
    digitalWrite(stepPin2, LOW);
    delay(1);
    currentStep2 -= 1;
    }
    if currentStep1 < 0{
    
    digitalWrite(stepPin2, HIGH);
    delay(1);
    digitalWrite(stepPin2, LOW);
    delay(1);
    currentStep2 -= 1;
    }

   }
  }


void controlMotorsWithApp() {
  if (millis() - lastBLECheck >= BLECheckInterval) {
    lastBLECheck = millis();
    checkBLECommands();
  }

  if (motor1CommandArrived) {
    pulseMotor(stepPin1, dirPin1, motor1Direction, motor1Speed);
  }
  if (motor2CommandArrived) {
    pulseMotor(stepPin2, dirPin2, motor2Direction, motor2Speed);
  } else if (!motor2CommandArrived, !motor1CommandArrived) {
    motor1CommandArrived = false;
    motor2CommandArrived = false;
  }

}

void parseAppCommands(String command) {
  if (command.startsWith("MOTOR1:")) {
    motor1Direction = command.charAt(7) == '-' ? -1 : 1;
    motor1Speed = max(2000, 2000 / command.substring(8).toInt());
    Serial.print("MOTOR1 set to direction ");
    Serial.print(motor1Direction);
    Serial.print(" with delay ");
    Serial.println(motor1Speed);
    motor1CommandArrived = true;
  } else if (command.startsWith("MOTOR2:")) {
    motor2Direction = command.charAt(7) == '-' ? -1 : 1;
    motor2Speed = max(2000, 2000 / command.substring(8).toInt());
    Serial.print("MOTOR2 set to direction ");
    Serial.print(motor2Direction);
    Serial.print(" with delay ");
    Serial.println(motor2Speed);
    motor2CommandArrived = true;
    if (motor2Direction == 1){
      currentStep1 -= 1;
    }
  }
  else if (command.startsWith("STOP")) {
    motor1CommandArrived = false;
    motor2CommandArrived = false;
  }
  else if (command.startsWith("RESET")) {
      appControlMode = false;
      autoModeEnabled = false;
      Serial.println("Resetting position...");
        resetMotorPosition();
      Serial.println("Reset complete.");
    }
  else if (command.startsWith("AUTOMODE")) {
      appControlMode = false;
      autoModeEnabled = true;
      Serial.println("Automode Enabled");
    }
}

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

void pulseMotor(int stepPin, int dirPin, int direction, int speedDelay) {
  digitalWrite(dirPin, direction > 0 ? HIGH : LOW);

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(speedDelay);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(speedDelay);
}

void loop() {
  // Only check BLE at specified intervals
  if (millis() - lastBLECheck >= BLECheckInterval) {
    lastBLECheck = millis();
    checkBLECommands();
  }

    

  // Control motors with direct pin pulsing for faster response
 if (motor1CommandArrived){
    pulseMotor(stepPin1, dirPin1, motor1Direction, motor1Speed);
      if (motor1Direction == 1){
      currentStep1 += 1;
    }
    else if (motor1Direction == 0){
      currentStep2 -= 0;
    }
 }
  if(motor2CommandArrived){
    pulseMotor(stepPin2, dirPin2, motor2Direction, motor2Speed);
    if (motor2Direction){
      currentStep2 += 1;
    }
    else if (motor2Direction){
      currentStep2 -= 0;
    }
  }
  else if (!motor2CommandArrived,!motor1CommandArrived) {
    motor1CommandArrived = false;
    motor2CommandArrived = false;
  }

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    if (command == "APP_CONNECTED") {
      appControlMode = true;
      autoModeEnabled = false;
    } else if (command == "APP_DISCONNECTED") {
      appControlMode = false;
    }
  }
}
