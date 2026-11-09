#define BLYNK_TEMPLATE_ID "TMPL6S4LVtq2z"
#define BLYNK_TEMPLATE_NAME "JDP"
#define BLYNK_AUTH_TOKEN "-3mpckmJpWrBoarIfflpKRrVXmTiqBN5"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <U8x8lib.h>
#include "Grove_Motor_Driver_TB6612FNG.h"

// Wi-Fi information
char ssid[] = "Bilal";
char pass[] = "0509489849";

// Existing motor driver
MotorDriver motor;

// Existing OLED screen
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

// Existing expansion board buzzer
const int pinBuzzer = A3;

int motorSpeed = 200;

// Tracks which direction is active
int currentDirection = 0;

// ===============================
// OLED DISPLAY
// ===============================

void showStatus(const char* status) {
  u8x8.clearDisplay();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  u8x8.drawString(0, 0, "JDP ROBOT");
  u8x8.drawString(0, 2, "STATUS:");
  u8x8.drawString(0, 4, status);
}

// ===============================
// BUZZER
// ===============================

void shortBeep() {
  tone(pinBuzzer, 1000, 100);
}

void doubleBeep() {
  tone(pinBuzzer, 1000, 100);
  delay(150);
  tone(pinBuzzer, 1000, 100);
}

// ===============================
// ROBOT MOVEMENT
// ===============================

void stopRobot() {
  motor.dcMotorStop(MOTOR_CHA);
  motor.dcMotorStop(MOTOR_CHB);

  currentDirection = 0;
  showStatus("STOP");
  Serial.println("STOP");
}

void forward() {
  motor.dcMotorRun(MOTOR_CHA, motorSpeed);
  motor.dcMotorRun(MOTOR_CHB, motorSpeed);

  currentDirection = 1;
  showStatus("FORWARD");
  shortBeep();
  Serial.println("FORWARD");
}

void backward() {
  motor.dcMotorRun(MOTOR_CHA, -motorSpeed);
  motor.dcMotorRun(MOTOR_CHB, -motorSpeed);

  currentDirection = 2;
  showStatus("BACKWARD");
  doubleBeep();
  Serial.println("BACKWARD");
}

void left() {
  motor.dcMotorRun(MOTOR_CHA, -motorSpeed);
  motor.dcMotorRun(MOTOR_CHB, motorSpeed);

  currentDirection = 3;
  showStatus("TURN LEFT");
  shortBeep();
  Serial.println("LEFT");
}

void right() {
  motor.dcMotorRun(MOTOR_CHA, motorSpeed);
  motor.dcMotorRun(MOTOR_CHB, -motorSpeed);

  currentDirection = 4;
  showStatus("TURN RIGHT");
  shortBeep();
  Serial.println("RIGHT");
}

// ===============================
// BLYNK BUTTONS
// ===============================

// Forward
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) {
    forward();
  } else if (currentDirection == 1) {
    stopRobot();
  }
}

// Backward
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) {
    backward();
  } else if (currentDirection == 2) {
    stopRobot();
  }
}

// Left
BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    left();
  } else if (currentDirection == 3) {
    stopRobot();
  }
}

// Right
BLYNK_WRITE(V3) {
  if (param.asInt() == 1) {
    right();
  } else if (currentDirection == 4) {
    stopRobot();
  }
}

// Stop
BLYNK_WRITE(V4) {
  if (param.asInt() == 1) {
    stopRobot();
  }
}

// Horn
BLYNK_WRITE(V5) {
  if (param.asInt() == 1) {
    tone(pinBuzzer, 1200);
    Serial.println("HORN");
  } else {
    noTone(pinBuzzer);
  }
}

// V6 = SPEED CONTROL
BLYNK_WRITE(V6)
{
  motorSpeed = constrain(param.asInt(), 0, 255);

  Serial.print("Speed: ");
  Serial.println(motorSpeed);

  // Update motor speed immediately if moving
  switch (currentDirection) {
    case 1:
      motor.dcMotorRun(MOTOR_CHA, motorSpeed);
      motor.dcMotorRun(MOTOR_CHB, motorSpeed);
      break;

    case 2:
      motor.dcMotorRun(MOTOR_CHA, -motorSpeed);
      motor.dcMotorRun(MOTOR_CHB, -motorSpeed);
      break;

    case 3:
      motor.dcMotorRun(MOTOR_CHA, -motorSpeed);
      motor.dcMotorRun(MOTOR_CHB, motorSpeed);
      break;

    case 4:
      motor.dcMotorRun(MOTOR_CHA, motorSpeed);
      motor.dcMotorRun(MOTOR_CHB, -motorSpeed);
      break;
  }

  if (motorSpeed == 0) {
    motor.dcMotorStop(MOTOR_CHA);
    motor.dcMotorStop(MOTOR_CHB);
  }
}

// ===============================
// CONNECTION SAFETY
// ===============================

BLYNK_CONNECTED() {
  Serial.println("Blynk connected!");
  showStatus("CONNECTED");
  Blynk.syncVirtual(V6);
}

void checkConnection() {
  if (!Blynk.connected()) {
    if (currentDirection != 0) {
      stopRobot();
    }
    showStatus("OFFLINE");
  }
}

BlynkTimer timer;

// ===============================
// SETUP
// ===============================

void setup() {
  Serial.begin(115200);

  pinMode(pinBuzzer, OUTPUT);
  noTone(pinBuzzer);

  Wire.begin();

  motor.init();
  motor.dcMotorStop(MOTOR_CHA);
  motor.dcMotorStop(MOTOR_CHB);

  u8x8.begin();
  u8x8.setPowerSave(0);
  showStatus("STARTING");

  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  showStatus("READY");
  timer.setInterval(500L, checkConnection);
}

// ===============================
// LOOP
// ===============================

void loop() {
  Blynk.run();
  timer.run();
}