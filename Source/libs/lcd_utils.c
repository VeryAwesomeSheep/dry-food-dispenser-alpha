#include "lcd_utils.h"
#include "lcd.h"
#include "feeding.h"
#include <stdint.h>
#include <time.h>

#include <stdio.h>

lcdStateMachineS lcdState = {0};

/*******************************************************************************
* handleLCD
*
* @brief Handles the main LCD state machine
*******************************************************************************/
void handleLCD() {
  switch (lcdState.state) {
    case LCD_WELCOME:
      lcdWelcomeScreen();
      lcdState.state = LCD_IDLE;
      break;
    case LCD_IDLE: {
      if (lcdState.lastPressedButton != NONE) {
        lcdState.lastPressedButton = NONE;
        lcdState.state = LCD_SETTINGS;
        lcdState.settingsState = LCD_SETTINGS_START;
        lcdState.isUpdateNeeded = true;
        break;
      }

      uint32_t currentTime = millis();
      if (currentTime - lcdState.lastIdleUpdate > 20000) {
        lcdState.lastIdleUpdate = currentTime;
        lcdIdleScreen();
      } else if (lcdState.isUpdateNeeded) {
        lcdIdleScreen();
        lcdState.isUpdateNeeded = false;
      }
      break;
    }
    case LCD_SETTINGS:
      lcdSettingsScreen();
      break;
  }
}

/*******************************************************************************
* lcdWelcomeScreen
*
* @brief Displays the welcome screen
*******************************************************************************/
void lcdWelcomeScreen() {
  lcd_clear_i2c();
  lcd_setCursor_i2c(3, 0);
  lcd_writeString_i2c("Pet Feeder");
  lcd_setCursor_i2c(4, 1);
  lcd_writeString_i2c("Welcome!");
}

/*******************************************************************************
* lcdIdleScreen
*
* @brief Displays the idle screen with the current time and the time until the
*        next feeding
*******************************************************************************/
void lcdIdleScreen() {
  time_t rawTime;
  struct tm* timeInfo;
  char timeBuffer[17];

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  // Locale dependent date and time format
  strftime(timeBuffer, sizeof(timeBuffer), "%H:%M - %d.%m.%y", timeInfo);

  char feedingTimeBuffer[17];
  uint16_t minutesUntilFeeding = minutesToNextFeeding();
  if (minutesUntilFeeding != UINT16_MAX) {
    uint8_t hours = minutesUntilFeeding / 60;
    uint8_t minutes = minutesUntilFeeding % 60;
    sprintf(feedingTimeBuffer, "Feeding: %hhuh %hhum", hours, minutes);
  }
  else {
    sprintf(feedingTimeBuffer, "Schedule empty!");
  }

  lcd_clear_i2c();
  lcd_writeString_i2c(timeBuffer);
  lcd_setCursor_i2c(0, 1);
  lcd_writeString_i2c(feedingTimeBuffer);
}

/*******************************************************************************
* handleLCD
*
* @brief Handles the settings LCD state machine
*******************************************************************************/
void lcdSettingsScreen() {
  if (millis() - lcdState.lastButtonPressTime > 30000) {
    lcdState.state = LCD_IDLE;
    lcdState.settingsState = LCD_SETTINGS_START;
    lcdState.selectedRow = 0;
    lcdState.selectedFeedSchedule = 0;
    lcdState.entryMode = (lcdEntryModeS){0};
    lcdState.isUpdateNeeded = true;
    lcd_blinkOff_i2c();
    return;
  }

  switch (lcdState.settingsState) {
    case LCD_SETTINGS_START:
      if (lcdState.isUpdateNeeded) {
        lcd_clear_i2c();
        lcd_writeString_i2c("Schedule");
        lcd_setCursor_i2c(0, 1);
        lcd_writeString_i2c("Feeder wheel");
        lcdDrawPointingArrow();

        lcdState.isUpdateNeeded = false;
      }

      switch (lcdState.lastPressedButton) {
        case UP:
        case DOWN:
          lcdState.lastPressedButton = NONE;
          lcdState.selectedRow = !lcdState.selectedRow;
          lcdDrawPointingArrow();
          break;
        case LEFT:
          lcdState.lastPressedButton = NONE;
          lcdState.selectedRow = 0;
          lcdState.state = LCD_IDLE;
          lcdState.isUpdateNeeded = true;
          break;
        case RIGHT:
          lcdState.lastPressedButton = NONE;
          switch (lcdState.selectedRow) {
            case 0:
              lcdState.settingsState = LCD_SETTINGS_SCHEDULE;
              break;
            case 1:
              lcdState.settingsState = LCD_SETTINGS_WHEEL_EDIT;
              break;
          }
          lcdState.selectedRow = 0;
          lcdState.isUpdateNeeded = true;
        default:
          break;
      }

      break;
    case LCD_SETTINGS_SCHEDULE:
      if (lcdState.isUpdateNeeded) {
        char line1Buffer[17];
        char line2Buffer[17];

        // Calculate the schedule indexes to display on the screen
        uint8_t scheduleIndex1 = lcdState.selectedFeedSchedule;
        uint8_t scheduleIndex2 = scheduleIndex1 + 1;

        if (lcdState.selectedRow == 1) {
          scheduleIndex1 = scheduleIndex1 > 0 ? scheduleIndex1 - 1 : 0;
          scheduleIndex2 = lcdState.selectedFeedSchedule;
        }

        if (getActiveFeedingTimes() == 0) {
          sprintf(line1Buffer, "Schedule empty!");
        }
        else if (scheduleIndex1 < getActiveFeedingTimes()) {
          uint8_t hour = getFeedingTimeHour(scheduleIndex1);
          uint8_t minute = getFeedingTimeMinute(scheduleIndex1);

          if (hour < 10 && minute < 10) {
            sprintf(line1Buffer, "%hhu. 0%hhu:0%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1));
          }
          else if (hour < 10 && minute >= 10) {
            sprintf(line1Buffer, "%hhu. 0%hhu:%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1));
          }
          else if (hour >= 10 && minute < 10) {
            sprintf(line1Buffer, "%hhu. %hhu:0%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1));
          }
          else {
            sprintf(line1Buffer, "%hhu. %hhu:%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1));
          }
        }
        else {
          uint8_t hour = getFeedingTimeHour(scheduleIndex1 - 1);
          uint8_t minute = getFeedingTimeMinute(scheduleIndex1 - 1);

          if (hour < 10 && minute < 10) {
            sprintf(line1Buffer, "%hhu. 0%hhu:0%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1 - 1));
          }
          else if (hour < 10 && minute >= 10) {
            sprintf(line1Buffer, "%hhu. 0%hhu:%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1 - 1));
          }
          else if (hour >= 10 && minute < 10) {
            sprintf(line1Buffer, "%hhu. %hhu:0%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1 - 1));
          }
          else {
            sprintf(line1Buffer, "%hhu. %hhu:%hhu - %hhu", scheduleIndex1,
                    hour, minute, getFeedingTimePortions(scheduleIndex1 - 1));
          }
        }

        if (scheduleIndex2 < getActiveFeedingTimes()) {
          uint8_t hour = getFeedingTimeHour(scheduleIndex2);
          uint8_t minute = getFeedingTimeMinute(scheduleIndex2);

          if (hour < 10 && minute < 10) {
            sprintf(line2Buffer, "%hhu. 0%hhu:0%hhu - %hhu", scheduleIndex2,
                    hour, minute, getFeedingTimePortions(scheduleIndex2));
          }
          else if (hour < 10 && minute >= 10) {
            sprintf(line2Buffer, "%hhu. 0%hhu:%hhu - %hhu", scheduleIndex2,
                    hour, minute, getFeedingTimePortions(scheduleIndex2));
          }
          else if (hour >= 10 && minute < 10) {
            sprintf(line2Buffer, "%hhu. %hhu:0%hhu - %hhu", scheduleIndex2,
                    hour, minute, getFeedingTimePortions(scheduleIndex2));
          }
          else {
            sprintf(line2Buffer, "%hhu. %hhu:%hhu - %hhu", scheduleIndex2,
                    hour, minute, getFeedingTimePortions(scheduleIndex2));
          }
        }
        else {
          sprintf(line2Buffer, "Add new");
        }

        lcd_clear_i2c();
        lcd_writeString_i2c(line1Buffer);
        lcd_setCursor_i2c(0, 1);
        lcd_writeString_i2c(line2Buffer);
        lcdDrawPointingArrow();

        lcdState.isUpdateNeeded = false;
      }

      switch (lcdState.lastPressedButton) {
        case UP:
          lcdState.lastPressedButton = NONE;
          if (lcdState.selectedFeedSchedule > 0) {
            lcdState.selectedFeedSchedule--;
          }

          if (lcdState.selectedRow == 0) {
              lcdState.isUpdateNeeded = true;
          }
          else {
            lcdState.selectedRow = 0;
            lcdDrawPointingArrow();
          }
          break;
        case DOWN:
          lcdState.lastPressedButton = NONE;
          if (lcdState.selectedFeedSchedule < getActiveFeedingTimes()) { // allows to go 1 over for add new
            lcdState.selectedFeedSchedule++;
          }

          if (lcdState.selectedRow == 1) {
            lcdState.isUpdateNeeded = true;
          }
          else {
            lcdState.selectedRow = 1;
            lcdDrawPointingArrow();
          }
          break;
        case LEFT:
          lcdState.lastPressedButton = NONE;
          lcdState.settingsState = LCD_SETTINGS_START;
          lcdState.isUpdateNeeded = true;
          lcdState.selectedRow = 0;
          lcdState.selectedFeedSchedule = 0;
          break;
        case RIGHT:
          lcdState.lastPressedButton = NONE;
          if (lcdState.selectedFeedSchedule < getActiveFeedingTimes()) {
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE_ENTRY_OPTIONS;
          }
          else {
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE_ADD;
          }
          lcdState.selectedRow = 0;
          lcdState.isUpdateNeeded = true;
          break;
        default:
          break;
      }

      break;
    case LCD_SETTINGS_SCHEDULE_ENTRY_OPTIONS:
      if (lcdState.isUpdateNeeded) {
        lcd_clear_i2c();
        lcd_writeString_i2c("Modify");
        lcd_setCursor_i2c(0, 1);
        lcd_writeString_i2c("Remove");
        lcdDrawPointingArrow();

        lcdState.isUpdateNeeded = false;
      }

      switch (lcdState.lastPressedButton) {
        case UP:
        case DOWN:
          lcdState.lastPressedButton = NONE;
          lcdState.selectedRow = !lcdState.selectedRow;
          lcdDrawPointingArrow();
          break;
        case LEFT:
          lcdState.lastPressedButton = NONE;
          lcdState.settingsState = LCD_SETTINGS_SCHEDULE;
          lcdState.selectedRow = 0;
          lcdState.selectedFeedSchedule = 0;
          lcdState.isUpdateNeeded = true;
          break;
        case RIGHT:
          lcdState.lastPressedButton = NONE;
          lcdState.isUpdateNeeded = true;

          if (lcdState.selectedRow == 0) {
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE_ENTRY_EDIT;
          }
          else {
            removeFeedingTime(lcdState.selectedFeedSchedule);
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE;
            lcdState.selectedRow = 0;
            lcdState.selectedFeedSchedule = 0;
          }
          break;
        default:
          break;
      }

      break;
    case LCD_SETTINGS_SCHEDULE_ENTRY_EDIT:
      if (lcdState.isUpdateNeeded) {
        char line1Buffer[17];
        char line2Buffer[17];

        if (lcdState.entryMode.isNewTimeTaken) {
          sprintf(line1Buffer, "Time in schedule");
          sprintf(line2Buffer, " ");
        }
        else {
          uint8_t hour = getFeedingTimeHour(lcdState.selectedFeedSchedule);
          uint8_t minute = getFeedingTimeMinute(lcdState.selectedFeedSchedule);

          if (hour < 10 && minute < 10) {
            sprintf(line1Buffer, "Time: 0%hhu:0%hhu", hour, minute);
          }
          else if (hour < 10 && minute >= 10) {
            sprintf(line1Buffer, "Time: 0%hhu:%hhu", hour, minute);
          }
          else if (hour >= 10 && minute < 10) {
            sprintf(line1Buffer, "Time: %hhu:0%hhu", hour, minute);
          }
          else {
            sprintf(line1Buffer, "Time: %hhu:%hhu", hour, minute);
          }

          sprintf(line2Buffer, "Portions: %hhu",
            getFeedingTimePortions(lcdState.selectedFeedSchedule));
        }

        lcd_clear_i2c();
        lcd_writeString_i2c(line1Buffer);
        lcd_setCursor_i2c(0, 1);
        lcd_writeString_i2c(line2Buffer);
        if (!lcdState.entryMode.isNewTimeTaken) lcdDrawPointingArrow();

        lcdState.isUpdateNeeded = false;
      }

      if (lcdState.entryMode.isEntryMode) {
        lcd_blinkOn_i2c();

        // Editing time of feeding time
        if (lcdState.selectedRow == 0) {
          char numBuffer[3];

          // Editing hour of feeding time
          if (!lcdState.entryMode.isMinutesEdited) {
            lcd_setCursor_i2c(6, 0);

            switch (lcdState.lastPressedButton) {
              case UP:
                lcdState.lastPressedButton = NONE;
                if (lcdState.entryMode.currentlySelectedValue < 23) {
                  lcdState.entryMode.currentlySelectedValue++;
                  if (lcdState.entryMode.currentlySelectedValue < 10) {
                    sprintf(numBuffer, "0%hhu", lcdState.entryMode.currentlySelectedValue);
                  }
                  else {
                    sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
                  }
                  lcd_setCursor_i2c(6, 0);
                  lcd_writeString_i2c(numBuffer);
                }
                break;
              case DOWN:
                lcdState.lastPressedButton = NONE;
                if (lcdState.entryMode.currentlySelectedValue > 0) {
                  lcdState.entryMode.currentlySelectedValue--;
                  if (lcdState.entryMode.currentlySelectedValue < 10) {
                    sprintf(numBuffer, "0%hhu", lcdState.entryMode.currentlySelectedValue);
                  }
                  else {
                    sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
                  }
                  lcd_setCursor_i2c(6, 0);
                  lcd_writeString_i2c(numBuffer);
                }
                break;
              case LEFT:
                lcdState.lastPressedButton = NONE;
                lcdState.entryMode.isEntryMode = false;
                if (lcdState.entryMode.valueBeforeEdit < 10)
                  sprintf(numBuffer, "0%hhu", lcdState.entryMode.valueBeforeEdit);
                else
                  sprintf(numBuffer, "%hhu", lcdState.entryMode.valueBeforeEdit);
                lcd_setCursor_i2c(6, 0);
                lcd_writeString_i2c(numBuffer);
                lcd_blinkOff_i2c();
                break;
              case RIGHT:
                lcdState.lastPressedButton = NONE;
                lcdState.entryMode.isMinutesEdited = true;
                break;
              default:
                break;
            }
          }
          // Editing minute of feeding time
          else {
            lcd_setCursor_i2c(9, 0);

            switch (lcdState.lastPressedButton) {
              case UP:
                lcdState.lastPressedButton = NONE;
                if (lcdState.entryMode.currentlySelectedValue2 < 59) {
                  lcdState.entryMode.currentlySelectedValue2++;
                  if (lcdState.entryMode.currentlySelectedValue2 < 10) {
                    sprintf(numBuffer, "0%hhu", lcdState.entryMode.currentlySelectedValue2);
                  }
                  else {
                    sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue2);
                  }
                  lcd_setCursor_i2c(9, 0);
                  lcd_writeString_i2c(numBuffer);
                }
                break;
              case DOWN:
                lcdState.lastPressedButton = NONE;
                if (lcdState.entryMode.currentlySelectedValue2 > 0) {
                  lcdState.entryMode.currentlySelectedValue2--;
                  if (lcdState.entryMode.currentlySelectedValue2 < 10) {
                    sprintf(numBuffer, "0%hhu", lcdState.entryMode.currentlySelectedValue2);
                  }
                  else {
                    sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue2);
                  }
                  lcd_setCursor_i2c(9, 0);
                  lcd_writeString_i2c(numBuffer);
                }
                break;
              case LEFT:
                lcdState.lastPressedButton = NONE;
                lcdState.entryMode.isMinutesEdited = false;
                sprintf(numBuffer, "%hhu", lcdState.entryMode.valueBeforeEdit2);
                lcd_setCursor_i2c(9, 0);
                lcd_writeString_i2c(numBuffer);
                break;
              case RIGHT:
                lcdState.lastPressedButton = NONE;
                lcd_blinkOff_i2c();
                if (isFeedingTimeDuplicate(lcdState.entryMode.currentlySelectedValue, lcdState.entryMode.currentlySelectedValue2)) {
                  lcdState.isUpdateNeeded = true;
                  lcdState.entryMode.isEntryMode = false;
                  lcdState.entryMode.isMinutesEdited = false;
                  lcdState.entryMode.isNewTimeTaken = true;
                  break;
                } else {
                  saveModifiedFeedingTime(lcdState.selectedFeedSchedule,
                                          lcdState.entryMode.currentlySelectedValue,
                                          lcdState.entryMode.currentlySelectedValue2);
                  lcdState.entryMode.isEntryMode = false;
                }
                break;
              default:
                break;
            }
          }
        }
        // Editing portions of feeding time
        else if (lcdState.selectedRow == 1) {
          lcd_setCursor_i2c(10, 1);
          char numBuffer[3];

          switch (lcdState.lastPressedButton) {
            case UP:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.currentlySelectedValue < 10) {
                lcdState.entryMode.currentlySelectedValue++;
                sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
                lcd_setCursor_i2c(10, 1);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case DOWN:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.currentlySelectedValue > 1) {
                lcdState.entryMode.currentlySelectedValue--;
                sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
                lcd_setCursor_i2c(10, 1);
                lcd_writeString_i2c(numBuffer);
                lcd_removeChar_i2c(11, 1); // remove second digit so there is no left over from value 10
              }
              break;
            case LEFT:
              lcdState.lastPressedButton = NONE;
              lcdState.entryMode.isEntryMode = false;
              sprintf(numBuffer, "%hhu", lcdState.entryMode.valueBeforeEdit);
              lcd_setCursor_i2c(10, 1);
              lcd_writeString_i2c(numBuffer);
              lcd_blinkOff_i2c();
              break;
            case RIGHT:
              lcdState.lastPressedButton = NONE;
              setFeedingTimePortions(lcdState.selectedFeedSchedule, lcdState.entryMode.currentlySelectedValue);
              lcdState.entryMode.isEntryMode = false;
              lcd_blinkOff_i2c();
              break;
            default:
              break;
          }
        }
      }
      else {
        switch (lcdState.lastPressedButton) {
          case UP:
          case DOWN:
            if (lcdState.entryMode.isNewTimeTaken) break;
            lcdState.lastPressedButton = NONE;
            lcdState.selectedRow = !lcdState.selectedRow;
            lcdDrawPointingArrow();
            break;
          case LEFT:
            lcdState.lastPressedButton = NONE;
            if (lcdState.entryMode.isNewTimeTaken) break;
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE_ENTRY_OPTIONS;
            lcdState.selectedRow = 0;
            lcdState.selectedFeedSchedule = 0;
            lcdState.entryMode = (lcdEntryModeS){0};
            lcdState.isUpdateNeeded = true;
            break;
          case RIGHT:
            lcdState.lastPressedButton = NONE;
            if (lcdState.entryMode.isNewTimeTaken) {
              lcdState.entryMode.isNewTimeTaken = false;
              lcdState.isUpdateNeeded = true;
              break;
            }
            lcdState.entryMode.isEntryMode = true;
            if (lcdState.selectedRow == 0) {
              lcdState.entryMode.valueBeforeEdit = getFeedingTimeHour(lcdState.selectedFeedSchedule);
              lcdState.entryMode.valueBeforeEdit2 = getFeedingTimeMinute(lcdState.selectedFeedSchedule);
            }
            else {
              lcdState.entryMode.valueBeforeEdit = getFeedingTimePortions(lcdState.selectedFeedSchedule);
            }

            lcdState.entryMode.currentlySelectedValue = lcdState.entryMode.valueBeforeEdit;
            lcdState.entryMode.currentlySelectedValue2 = lcdState.entryMode.valueBeforeEdit2;
            break;
          default:
            break;
        }
      }

      break;
    case LCD_SETTINGS_SCHEDULE_ADD:
      if (lcdState.isUpdateNeeded) {
        char line1Buffer[17];

        if (getActiveFeedingTimes() == 10) {
          sprintf(line1Buffer, "Schedule full!");
          lcdState.entryMode.add.isScheduleFull = true;
        }
        else if (lcdState.entryMode.add.isScheduleAdded) {
          sprintf(line1Buffer, "Time in schedule");
        }
        else if (!lcdState.entryMode.isPortionsEdited) {
          uint8_t hour = lcdState.entryMode.add.hour;
          uint8_t minute = lcdState.entryMode.add.minute;

          if (hour < 10 && minute < 10) {
          sprintf(line1Buffer, "Time: 0%hhu:0%hhu", hour, minute);
          }
          else if (hour < 10 && minute >= 10) {
            sprintf(line1Buffer, "Time: 0%hhu:%hhu", hour, minute);
          }
          else if (hour >= 10 && minute < 10) {
            sprintf(line1Buffer, "Time: %hhu:0%hhu", hour, minute);
          }
          else {
            sprintf(line1Buffer, "Time: %hhu:%hhu", hour, minute);
          }
        }
        else {
          sprintf(line1Buffer, "Portions: %hhu",
            lcdState.entryMode.add.portions);
        }

        lcd_clear_i2c();
        lcd_writeString_i2c(line1Buffer);

        lcdState.isUpdateNeeded = false;
      }

      if (lcdState.entryMode.isEntryMode) {
        lcd_blinkOn_i2c();
        char numBuffer[3];

        // Editing hour of feeding
        if (!lcdState.entryMode.isMinutesEdited && !lcdState.entryMode.isPortionsEdited) {
          lcd_setCursor_i2c(6, 0);
          switch (lcdState.lastPressedButton) {
            case UP:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.hour < 23) {
                lcdState.entryMode.add.hour++;
                if (lcdState.entryMode.add.hour < 10) {
                  sprintf(numBuffer, "0%hhu", lcdState.entryMode.add.hour);
                }
                else {
                  sprintf(numBuffer, "%hhu", lcdState.entryMode.add.hour);
                }
                lcd_setCursor_i2c(6, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case DOWN:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.hour > 0) {
                lcdState.entryMode.add.hour--;
                if (lcdState.entryMode.add.hour < 10) {
                  sprintf(numBuffer, "0%hhu", lcdState.entryMode.add.hour);
                }
                else {
                  sprintf(numBuffer, "%hhu", lcdState.entryMode.add.hour);
                }
                lcd_setCursor_i2c(6, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case LEFT:
              lcdState.lastPressedButton = NONE;
              lcdState.entryMode.isEntryMode = false;
              lcdState.entryMode.add.hour = 0;
              lcdState.entryMode.add.minute = 0;
              lcdState.entryMode.add.portions = 1;
              lcdState.isUpdateNeeded = true;
              lcd_blinkOff_i2c();
              break;
            case RIGHT:
              lcdState.lastPressedButton = NONE;
              lcdState.entryMode.isMinutesEdited = true;
              lcdState.isUpdateNeeded = true;
              break;
            default:
              break;
            }
        }
        // Editing minute of feeding
        else if (lcdState.entryMode.isMinutesEdited && !lcdState.entryMode.isPortionsEdited) {
          lcd_setCursor_i2c(9, 0);
          switch (lcdState.lastPressedButton) {
            case UP:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.minute < 59) {
                lcdState.entryMode.add.minute++;
                if (lcdState.entryMode.add.minute < 10) {
                  sprintf(numBuffer, "0%hhu", lcdState.entryMode.add.minute);
                }
                else {
                  sprintf(numBuffer, "%hhu", lcdState.entryMode.add.minute);
                }
                lcd_setCursor_i2c(9, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case DOWN:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.minute > 0) {
                lcdState.entryMode.add.minute--;
                if (lcdState.entryMode.add.minute < 10) {
                  sprintf(numBuffer, "0%hhu", lcdState.entryMode.add.minute);
                }
                else {
                  sprintf(numBuffer, "%hhu", lcdState.entryMode.add.minute);
                }
                lcd_setCursor_i2c(9, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case LEFT:
              lcdState.lastPressedButton = NONE;
              lcdState.entryMode.isMinutesEdited = false;
              lcdState.isUpdateNeeded = true;
              break;
            case RIGHT:
              lcdState.lastPressedButton = NONE;
              lcdState.isUpdateNeeded = true;
              // Same time already exists in schedule
              if (isFeedingTimeDuplicate(lcdState.entryMode.add.hour, lcdState.entryMode.add.minute)) {
                lcd_blinkOff_i2c();
                lcdState.entryMode.isEntryMode = false;
                lcdState.entryMode.isMinutesEdited = false;
                lcdState.entryMode.isPortionsEdited = false;
                lcdState.entryMode.add.isScheduleAdded = true;
                break;
              }
              lcdState.entryMode.isMinutesEdited = false;
              lcdState.entryMode.isPortionsEdited = true;
              break;
            default:
              break;
            }
        }
        // Editing portions of feeding
        else if (!lcdState.entryMode.isMinutesEdited && lcdState.entryMode.isPortionsEdited) {
          lcd_setCursor_i2c(10, 0);
          switch (lcdState.lastPressedButton) {
            case UP:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.portions < 10) {
                lcdState.entryMode.add.portions++;
                sprintf(numBuffer, "%hhu", lcdState.entryMode.add.portions);
                lcd_setCursor_i2c(10, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case DOWN:
              lcdState.lastPressedButton = NONE;
              if (lcdState.entryMode.add.portions > 1) {
                lcdState.entryMode.add.portions--;
                sprintf(numBuffer, "%hhu ", lcdState.entryMode.add.portions);
                lcd_setCursor_i2c(10, 0);
                lcd_writeString_i2c(numBuffer);
              }
              break;
            case LEFT:
              lcdState.lastPressedButton = NONE;
              lcdState.entryMode.isMinutesEdited = true;
              lcdState.entryMode.isPortionsEdited = false;
              lcdState.isUpdateNeeded = true;
              break;
            case RIGHT:
              lcdState.lastPressedButton = NONE;
              addFeedingTime(lcdState.entryMode.add.hour, lcdState.entryMode.add.minute, lcdState.entryMode.add.portions);
              lcdState.entryMode = (lcdEntryModeS){0};
              lcdState.settingsState = LCD_SETTINGS_SCHEDULE;
              lcdState.selectedRow = 0;
              lcdState.selectedFeedSchedule = 0;
              lcdState.isUpdateNeeded = true;
              lcd_blinkOff_i2c();
              break;
            default:
              break;
            }
        }
      }
      else {
        switch (lcdState.lastPressedButton) {
          case UP:
          case DOWN:
            break;
          case LEFT:
            lcdState.lastPressedButton = NONE;
            lcdState.settingsState = LCD_SETTINGS_SCHEDULE;
            lcdState.selectedRow = 0;
            lcdState.selectedFeedSchedule = 0;
            lcdState.entryMode = (lcdEntryModeS){0};
            lcdState.isUpdateNeeded = true;
            break;
          case RIGHT:
            lcdState.lastPressedButton = NONE;
            if (lcdState.entryMode.add.isScheduleFull) break;
            if (lcdState.entryMode.add.isScheduleAdded) {
              lcdState.entryMode.add.isScheduleAdded = false;
              lcdState.entryMode.add.hour = 0;
              lcdState.entryMode.add.minute = 0;
              lcdState.entryMode.add.portions = 1;
              lcdState.isUpdateNeeded = true;
              break;
            }
            lcdState.entryMode.isEntryMode = true;
            lcdState.entryMode.add.hour = 0;
            lcdState.entryMode.add.minute = 0;
            lcdState.entryMode.add.portions = 1;
            break;
          default:
            break;
        }
      }

      break;
    case LCD_SETTINGS_WHEEL_EDIT:
      if (lcdState.isUpdateNeeded) {
        char buffer[8];
        sprintf(buffer, "Arms: %hhu", getFeedingWheelArms());

        lcd_clear_i2c();
        lcd_writeString_i2c(buffer);
        lcdDrawPointingArrow();

        lcdState.isUpdateNeeded = false;
      }

      if (lcdState.entryMode.isEntryMode) {
        lcd_blinkOn_i2c();
        lcd_setCursor_i2c(6, 0);
        char numBuffer[2];

        switch (lcdState.lastPressedButton) {
          case UP:
            lcdState.lastPressedButton = NONE;
            if (lcdState.entryMode.currentlySelectedValue < 8) {
              lcdState.entryMode.currentlySelectedValue += 2;
              sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
              lcd_setCursor_i2c(6, 0);
              lcd_writeString_i2c(numBuffer);
            }
            break;
          case DOWN:
            lcdState.lastPressedButton = NONE;
            if (lcdState.entryMode.currentlySelectedValue > 4) {
              lcdState.entryMode.currentlySelectedValue -= 2;
              sprintf(numBuffer, "%hhu", lcdState.entryMode.currentlySelectedValue);
              lcd_setCursor_i2c(6, 0);
              lcd_writeString_i2c(numBuffer);
            }
            break;
          case LEFT:
            lcdState.lastPressedButton = NONE;
            lcdState.entryMode.isEntryMode = false;
            sprintf(numBuffer, "%hhu", lcdState.entryMode.valueBeforeEdit);
            lcd_setCursor_i2c(6, 0);
            lcd_writeString_i2c(numBuffer);
            lcd_blinkOff_i2c();
            break;
          case RIGHT:
            lcdState.lastPressedButton = NONE;
            setFeedingWheelArms(lcdState.entryMode.currentlySelectedValue);
            lcdState.entryMode.isEntryMode = false;
            lcd_blinkOff_i2c();
            break;
          default:
            break;
        }
      }
      else {
        switch (lcdState.lastPressedButton) {
          case LEFT:
            lcdState.lastPressedButton = NONE;
            lcdState.settingsState = LCD_SETTINGS_START;
            lcdState.selectedRow = 0;
            lcdState.selectedFeedSchedule = 0;
            lcdState.entryMode = (lcdEntryModeS){0};
            lcdState.isUpdateNeeded = true;
            break;
          case RIGHT:
            lcdState.lastPressedButton = NONE;
            lcdState.entryMode.valueBeforeEdit = getFeedingWheelArms();
            lcdState.entryMode.currentlySelectedValue = lcdState.entryMode.valueBeforeEdit;
            lcdState.entryMode.isEntryMode = true;
            break;
          default:
            break;
        }
      }

      break;
  }
}

/*******************************************************************************
* processButtonPress
*
* @brief Processes the button press and updates the last pressed button and the
*        time of the press
*******************************************************************************/
void processButtonPress(lcdButtonsE button) {
  lcdState.lastPressedButton = button;
  lcdState.lastButtonPressTime = millis();
}

/*******************************************************************************
* lcdDrawPointingArrow
*
* @brief Prints the pointing arrow on the last position in the row to indicate
*        the row selected by the user
*******************************************************************************/
void lcdDrawPointingArrow() {
  switch (lcdState.selectedRow) {
    case 0:
      lcd_removeChar_i2c(15, 1);
    case 1:
      lcd_removeChar_i2c(15, 0);
  }

  lcd_setCursor_i2c(15, lcdState.selectedRow);
  lcd_writeString_i2c("<");
}
