$ JOYSTICK CODE :
#include "Arduino.h"
#include <SPI.h>
#include <RF24.h>

// This is just the way the RF24 library works:
// Hardware configuration: Set up nRF24L01 radio on SPI bus (pins 10, 11, 12, 13) plus pins 9 & 10 for CE and CSN
RF24 radio(9, 10);

int joystickX1Pin = A3;
int joystickY1Pin = A2;
int joystickX2Pin = A1;
int joystickY2Pin = A0;

byte addresses[][6] = {"nil", "suki2948"};

// -----------------------------------------------------------------------------
// SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP   SETUP
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.println("THIS IS THE TRANSMITTER CODE - YOU NEED THE OTHER ARDUINO TO SEND BACK A RESPONSE");

  // Initiate the radio object
  radio.begin();

  // Set the transmit power to lowest available to prevent power supply related issues
  radio.setPALevel(RF24_PA_MIN);

  // Set the speed of the transmission to the quickest available
  radio.setDataRate(RF24_250KBPS);

  // Use a channel unlikely to be used by Wifi, Microwave ovens, etc.
  radio.setChannel(124);

  // Open a writing pipe on the radio, with the specified address
  radio.openWritingPipe(addresses[1]);
}

// -----------------------------------------------------------------------------
// LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// -----------------------------------------------------------------------------
void loop() {
  int joystickX1Value = analogRead(joystickX1Pin);
  int joystickY1Value = analogRead(joystickY1Pin);
  int joystickX2Value = analogRead(joystickX2Pin);
  int joystickY2Value = analogRead(joystickY2Pin);

  // Create a struct to hold joystick values
  struct JoystickData {
    int x1;
    int y1;
    int x2;
    int y2;
  };

  JoystickData joystickData;
  joystickData.x1 = joystickX1Value;
  joystickData.y1 = joystickY1Value;
  joystickData.x2 = joystickX2Value;
  joystickData.y2 = joystickY2Value;

  // Send joystick data to the receiver
  radio.write(&joystickData, sizeof(joystickData));

  delay(10); // Add a delay to prevent flooding the receiver
}




$ RECIVER CODE: 

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pins

// Define the struct to hold joystick values
struct JoystickValues {
  int x1; // Joystick 1 x-axis for left/right turns
  int y1; // Joystick 1 y-axis (not used for now)
  int x2; // Joystick 2 x-axis (not used for now)
  int y2; // Joystick 2 y-axis for forward/backward
};

const int motor1Dir1 = 3; // Connect motor 1 direction pin 1 to Arduino pin 3
const int motor1Dir2 = 4; // Connect motor 1 direction pin 2 to Arduino pin 4

const int motor2Dir1 = A3; // Connect motor 2 direction pin 1 to Arduino pin A3
const int motor2Dir2 = A4; // Connect motor 2 direction pin 2 to Arduino pin A4

const int motor3Dir1 = A1; // Connect motor 3 direction pin 1 to Arduino pin A1
const int motor3Dir2 = A2; // Connect motor 3 direction pin 2 to Arduino pin A2

const int motor4Dir1 = 5; // Connect motor 4 direction pin 1 to Arduino pin 5
const int motor4Dir2 = 6; // Connect motor 4 direction pin 2 to Arduino pin 6

const int y2MidValue = 512;
const int y2Tolerance = 20;

const int x1MidValue = 512;
const int x1Tolerance = 20;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
  radio.startListening();

  pinMode(motor1Dir1, OUTPUT);
  pinMode(motor1Dir2, OUTPUT);
  pinMode(motor2Dir1, OUTPUT);
  pinMode(motor2Dir2, OUTPUT);
  pinMode(motor3Dir1, OUTPUT);
  pinMode(motor3Dir2, OUTPUT);
  pinMode(motor4Dir1, OUTPUT);
  pinMode(motor4Dir2, OUTPUT);
}

void loop() {
  if (radio.available()) {
    // Read the struct from NRF24
    JoystickValues joystickValues;
    radio.read(&joystickValues, sizeof(joystickValues));

    // Extract values from the struct
    int xValue1 = joystickValues.x1;
    int yValue2 = joystickValues.y2;

    Serial.print("xValue1: ");
    Serial.print(xValue1);
    Serial.print("\t");

    Serial.print("yValue2: ");
    Serial.println(yValue2);

    // Control motor directions based on joystick x-value for left/right turns
    digitalWrite(motor1Dir1, xValue1 < x1MidValue - x1Tolerance ? HIGH : LOW);
    digitalWrite(motor1Dir2, xValue1 > x1MidValue + x1Tolerance ? HIGH : LOW);

    digitalWrite(motor2Dir1, xValue1 < x1MidValue - x1Tolerance ? HIGH : LOW);
    digitalWrite(motor2Dir2, xValue1 > x1MidValue + x1Tolerance ? HIGH : LOW);

    digitalWrite(motor3Dir1, xValue1 < x1MidValue - x1Tolerance ? LOW : HIGH);
    digitalWrite(motor3Dir2, xValue1 > x1MidValue + x1Tolerance ? LOW : HIGH);

    digitalWrite(motor4Dir1, xValue1 < x1MidValue - x1Tolerance ? LOW : HIGH);
    digitalWrite(motor4Dir2, xValue1 > x1MidValue + x1Tolerance ? LOW : HIGH);

    // Control forward/backward based on joystick y2-value with tolerance
    if (yValue2 < y2MidValue - y2Tolerance) {
      // Move backward
      Serial.println("Moving backward");
      digitalWrite(motor1Dir1, HIGH);
      digitalWrite(motor1Dir2, LOW);
      digitalWrite(motor2Dir1, HIGH);
      digitalWrite(motor2Dir2, LOW);
      digitalWrite(motor3Dir1, HIGH);
      digitalWrite(motor3Dir2, LOW);
      digitalWrite(motor4Dir1, HIGH);
      digitalWrite(motor4Dir2, LOW);
    } else if (yValue2 > y2MidValue + y2Tolerance) {
      // Move forward
      Serial.println("Moving forward");
      digitalWrite(motor1Dir1, LOW);
      digitalWrite(motor1Dir2, HIGH);
      digitalWrite(motor2Dir1, LOW);
      digitalWrite(motor2Dir2, HIGH);
      digitalWrite(motor3Dir1, LOW);
      digitalWrite(motor3Dir2, HIGH);
      digitalWrite(motor4Dir1, LOW);
      digitalWrite(motor4Dir2, HIGH);
    } else {
      // Stop or maintain current state
      Serial.println("Stopped or maintaining current state");
      // Add your logic here if needed
    }
     
    if (xValue1 < x1MidValue - x1Tolerance) {
      Serial.println("Turning left");
      // Add your turning left motor control logic
    } else if (xValue1 > x1MidValue + x1Tolerance) {
      Serial.println("Turning right");
      // Add your turning right motor control logic
    }
  }
}
