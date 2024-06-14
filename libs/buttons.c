#include "buttons.h"
#include "../config.h"
#include "tools.h"
#include <wiringPi.h>
#include <stdio.h>
#include "motor.h"
#include "lcd_utils.h"
#include "logger.h"
#include "feeding.h"

buttonS buttonUp = {BUTTON_UP, true, 0, false, DEBOUNCE_TIME};
buttonS buttonDown = {BUTTON_DOWN, true, 0, false, DEBOUNCE_TIME};
buttonS buttonLeft = {BUTTON_LEFT, true, 0, false, DEBOUNCE_TIME};
buttonS buttonRight = {BUTTON_RIGHT, true, 0, false, DEBOUNCE_TIME};
buttonS buttonFeed = {BUTTON_FEED, true, 0, false, DEBOUNCE_TIME};

/*******************************************************************************
* initButtons
*
* @brief Initializes button pinModes, pullUps and ISRs
*
* @return 0 on success, 1 on failure
*******************************************************************************/
uint8_t initButtons() {
  char logMessageBuffer[120];

  pinMode(BUTTON_UP, INPUT);
  pinMode(BUTTON_DOWN, INPUT);
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);
  pinMode(BUTTON_FEED, INPUT);

  pullUpDnControl(BUTTON_UP, PUD_UP);
  pullUpDnControl(BUTTON_DOWN, PUD_UP);
  pullUpDnControl(BUTTON_LEFT, PUD_UP);
  pullUpDnControl(BUTTON_RIGHT, PUD_UP);
  pullUpDnControl(BUTTON_FEED, PUD_UP);

  if (wiringPiISR(BUTTON_UP, INT_EDGE_BOTH, &buttonUpISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(BUTTON_UP), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  if (wiringPiISR(BUTTON_DOWN, INT_EDGE_BOTH, &buttonDownISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(BUTTON_DOWN), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  if (wiringPiISR(BUTTON_LEFT, INT_EDGE_BOTH, &buttonLeftISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(BUTTON_LEFT), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  if (wiringPiISR(BUTTON_RIGHT, INT_EDGE_BOTH, &buttonRightISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(BUTTON_RIGHT), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  if (wiringPiISR(BUTTON_FEED, INT_EDGE_BOTH, &buttonFeedISR) < 0) {
    sprintf(logMessageBuffer, "Error: Unable to setup ISR for %s: %s", getName(BUTTON_FEED), strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return 1;
  }

  return 0;
}

/*******************************************************************************
* debounceButtons
*
* @brief Runs debouncing for all buttons
******************************************************************************/
void debounceButtons() {
  if (buttonUp.state) debounceButton(&buttonUp);
  if (buttonDown.state) debounceButton(&buttonDown);
  if (buttonLeft.state) debounceButton(&buttonLeft);
  if (buttonRight.state) debounceButton(&buttonRight);
  if (buttonFeed.state) debounceButton(&buttonFeed);
}

/*******************************************************************************
* debounceButton
*
* @brief Debounces a button and calls button handling functions
*
* @param[in] button Button to debounce
******************************************************************************/
void debounceButton(buttonS *button) {
  uint32_t currentTime = millis();

  if (currentTime - button->lastDebounceTime > button->debounceTime && button->state) {
    button->lastDebounceTime = currentTime;
    if (!digitalRead(button->pin) && button->previousState) {
      switch (button->pin) {
      case BUTTON_UP:
        processButtonPress(UP);
        break;
      case BUTTON_DOWN:
        processButtonPress(DOWN);
        break;
      case BUTTON_LEFT:
        processButtonPress(LEFT);
        break;
      case BUTTON_RIGHT:
        processButtonPress(RIGHT);
        break;
      case BUTTON_FEED:
        rotateMotor(360 / getFeedingWheelArms());
        break;
      default:
        break;
      }

      button->previousState = false;
      delayMicroseconds(100);
    } else if (digitalRead(button->pin) && !button->previousState) {
      button->previousState = true;
    }

    button->state = false;
  }
}

/*******************************************************************************
* buttonUpISR
*
* @brief ISR for buttonUp
******************************************************************************/
void buttonUpISR() {
  buttonUp.state = true;
}

/*******************************************************************************
* buttonDownISR
*
* @brief ISR for buttonDown
******************************************************************************/
void buttonDownISR() {
  buttonDown.state = true;
}

/*******************************************************************************
* buttonLeftISR
*
* @brief ISR for buttonLeft
******************************************************************************/
void buttonLeftISR() {
  buttonLeft.state = true;
}

/*******************************************************************************
* buttonRightISR
*
* @brief ISR for buttonRight
******************************************************************************/
void buttonRightISR() {
  buttonRight.state = true;
}

/*******************************************************************************
* buttonFeedISR
*
* @brief ISR for buttonFeed
******************************************************************************/
void buttonFeedISR() {
  buttonFeed.state = true;
}
