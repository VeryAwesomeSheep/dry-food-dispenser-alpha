// TODO: Clean up includes
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <wiringPi.h>
#include "config.h"
#include "libs/tools.h"
#include "libs/lcd.h" // TODO: replace with dynamic library
#include "libs/buttons.h"
#include "libs/motor.h"
#include "libs/lcd_utils.h"
#include "libs/logger.h"
#include "libs/feeding.h"

int main(void) {
  // Initialize logger
  if (initLogger() != 0) {
    fprintf(stderr, "Error during logger initialization: %s", strerror(errno));
    return 1;
  }

  char logMessageBuffer[120];

  // Initialize wiringPi
  if (wiringPiSetupGpio() != 0) {
    sprintf(logMessageBuffer, "Error during wiringPi initialization: %s", strerror(errno));
    logMessage(ERROR, logMessageBuffer);
  }

  // Initialize GPIO pins/devices
  lcd_init_i2c(LCD_ADDRESS, 16, 2, LCD_5x8DOTS);
  lcd_blinkOff_i2c();
  lcd_cursorOff_i2c();

  // Initialize motor
  if (initMotor() != 0) {
    sprintf(logMessageBuffer, "Error during motor initialization: %s", strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  // Initialize buttons
  if (initButtons() != 0) {
    sprintf(logMessageBuffer, "Error during buttons initialization: %s", strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  // Load feeding schedule
  if (loadFeedingSchedule() != 0) {
    sprintf(logMessageBuffer, "No feeding schedule loaded: %s", strerror(errno));
    logMessage(WARNING, logMessageBuffer);
  }

  // Feeder initialization complete
  //lcdWelcomeScreen();
  sprintf(logMessageBuffer, "Feeder initialization complete");
  logMessage(INFO, logMessageBuffer);

  // Operation loop
  while(1) {
    debounceButtons();
    handleFeeding();
    handleLCD();
    delayMicroseconds(10000);
  }

  return 0;
}
