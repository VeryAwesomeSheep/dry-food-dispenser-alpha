#ifndef interrupts_h
#define interrupts_h

#include <stdint.h>

uint8_t initMotor();
void driveMotor(int32_t speed);
void rotateMotor(int32_t degrees);
void encoderAISR();
void encoderBISR();

#endif // interrupts_h