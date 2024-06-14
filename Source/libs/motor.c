#include "motor.h"
#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include "../config.h"
#include "tools.h"
#include "logger.h"
#include <stdbool.h>

volatile int32_t encoderPosition = 0;
uint32_t prevTime = 0;
float prevError = 0;
float integralError = 0;

/*******************************************************************************
* initMotor
*
* @brief Initializes the motor and encoder
*
* @return 0 on success, 1 on failure
*******************************************************************************/
uint8_t initMotor() {
  char logMessageBuffer[120];

  pinMode(MOTOR_ENCODER_A, INPUT);
  pinMode(MOTOR_ENCODER_B, INPUT);
  pinMode(MOTOR_M1A, PWM_OUTPUT);
  pinMode(MOTOR_M1B, PWM_OUTPUT);

  if (wiringPiISR(MOTOR_ENCODER_A, INT_EDGE_BOTH, &encoderAISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(MOTOR_ENCODER_A), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  if (wiringPiISR(MOTOR_ENCODER_B, INT_EDGE_BOTH, &encoderBISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(MOTOR_ENCODER_B), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  return 0;
}

/*******************************************************************************
* driveMotor
*
* @brief Drives the motor at a given speed and direction
*
* @param[in] speed The speed to drive the motor at
*******************************************************************************/
void driveMotor(int32_t speed) {
  if (speed > 0) {
    if (speed > 1024) speed = 1024;
    pwmWrite(MOTOR_M1A, 0);
    pwmWrite(MOTOR_M1B, speed);
  }
  else if (speed < 0) {
    if (speed < -1024) speed = -1024;
    pwmWrite(MOTOR_M1A, -speed);
    pwmWrite(MOTOR_M1B, 0);
  }
  else {
    pwmWrite(MOTOR_M1A, 0);
    pwmWrite(MOTOR_M1B, 0);
  }
}

/*******************************************************************************
* rotateMotor
*
* @brief Rotates the motor by a given number of degrees
*
* @param[in] degrees The number of degrees to rotate the motor by
*******************************************************************************/
void rotateMotor(int32_t degrees) {
  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Rotating motor by %d degrees", degrees);
  logMessage(INFO, logMessageBuffer);
  bool blockHappened = false;

  encoderPosition = 0;

  int32_t targetPosition = degrees * MOTOR_ENCODER_TICKS_PER_DEGREE;

  // PID constants
  float kp = 5; // Proportional
  float kd = 0; // Derivative
  float ki = 0; // Integral

  // PID variables
  uint32_t prevTime = 0;
  float prevError = 0;
  float integralError = 0;

  // Block detection
  uint32_t blockTicks = 0; // Number of encoder ticks without change

  // Control signal
  float u = -1;

  while(u != 0) {
    // Time difference
    uint32_t currTime = micros();

    float deltaT = ((float)(currTime - prevTime)) / 1.0e6;
    prevTime = currTime;

    // Error
    int16_t error = encoderPosition + targetPosition;

    // Check for block
    if (error == prevError) {
      blockTicks++;
    } else {
      blockTicks = 0;
    }

    // If block detected
    if (blockTicks >= 150) {
      // Stop the motor
      driveMotor(0);
      // Back off
      int32_t backOffDegrees = degrees / -2;
      rotateMotor(backOffDegrees);
      delay(1000); // Wait for the motor to back off
      // Continue with the original destination
      targetPosition += backOffDegrees* MOTOR_ENCODER_TICKS_PER_DEGREE;
      blockTicks = 0; // Reset blockTicks
      blockHappened = true;
    }

    // Derivative
    float derivative = (error - prevError) / deltaT;

    // Integral
    integralError += error * deltaT;

    // Control signal
    u = kp * error + kd * derivative + ki * integralError;

    // Drive the motor
    driveMotor(u);

    // Update previous error
    prevError = error;

    // Print debug info
    delayMicroseconds(100);
  }

  // Stop the motor
  driveMotor(0);

  if (blockHappened) {
    sprintf(logMessageBuffer, "Motor reached position. Motor rotated by %d degrees, but block happened", degrees);
    logMessage(WARNING, logMessageBuffer);
  } else {
  sprintf(logMessageBuffer, "Motor reached position. Rotated by %d degrees", degrees);
  logMessage(INFO, logMessageBuffer);
  }
}

/*******************************************************************************
* encoderAISR
*
* @brief ISR for the channel A of the motor encoder. Modifies the motor position
*        value
*******************************************************************************/
void encoderAISR() {
  if (digitalRead(MOTOR_ENCODER_B) != digitalRead(MOTOR_ENCODER_A)) {
    encoderPosition++;
  } else {
    encoderPosition--;
  }
}

/*******************************************************************************
* encoderBISR
*
* @brief ISR for the channel B of the motor encoder. Modifies the motor position
*        value
*******************************************************************************/
void encoderBISR() {
  if (digitalRead(MOTOR_ENCODER_A) == digitalRead(MOTOR_ENCODER_B)) {
    encoderPosition++;
  } else {
    encoderPosition--;
  }
}