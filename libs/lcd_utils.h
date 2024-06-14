#ifndef lcd_utils_h
#define lcd_utils_h

#include <stdint.h>
#include <stdbool.h>

/* Enum definitions */
typedef enum {
  NONE,
  UP,
  DOWN,
  LEFT,
  RIGHT
} lcdButtonsE;

typedef enum {
  LCD_WELCOME,
  LCD_IDLE,
  LCD_SETTINGS
} lcdStateE;

typedef enum {
  LCD_SETTINGS_START,
  LCD_SETTINGS_SCHEDULE,
  LCD_SETTINGS_SCHEDULE_ENTRY_OPTIONS,
  LCD_SETTINGS_SCHEDULE_ENTRY_EDIT,
  LCD_SETTINGS_SCHEDULE_ADD,
  LCD_SETTINGS_WHEEL_EDIT
} lcdSettingsStateE;

typedef struct lcdAddS {
  bool isScheduleAdded; // whether or not same time schedule is already added
  bool isScheduleFull; // whether or not all schedules are already added
  uint8_t hour;
  uint8_t minute;
  uint8_t portions;
} lcdAddS;

typedef struct lcdEntryModeS {
  bool isEntryMode; // whether or not user is currently editing an entry
  uint8_t valueBeforeEdit; // value of entry before editing
  uint8_t currentlySelectedValue; // value of entry currently selected
  uint8_t valueBeforeEdit2; // value of entry before editing, used only for minutes in feeding schedule
  uint8_t currentlySelectedValue2; // value of entry currently selected, used only for minutes in feeding schedule
  bool isMinutesEdited; // whether or not user is currently editing minutes or hours
  bool isPortionsEdited; // whether or not user is currently editing portions
  bool isNewTimeTaken; // whether or not modified time is not the same as any other schedule
  lcdAddS add; // parameters for adding a new schedule
} lcdEntryModeS;

typedef struct lcdStateMachineS {
  lcdStateE state; // category that screen should display
  lcdSettingsStateE settingsState; // settings category that screen should display
  uint32_t lastIdleUpdate; // last time screen was updated in idle mode
  uint32_t lastButtonPressTime; // last time button was pressed (used to return to idle mode if no button is pressed for a while)
  lcdButtonsE lastPressedButton; // last button that was pressed
  bool isUpdateNeeded; // whether or not screen needs to be updated
  bool selectedRow; // row which user is currently selecting
  uint8_t selectedFeedSchedule; // schedule which user is currently selecting
  lcdEntryModeS entryMode; // parameters for entry mode
} lcdStateMachineS;

void handleLCD();
void lcdWelcomeScreen();
void lcdIdleScreen();
void lcdSettingsScreen();
void processButtonPress(lcdButtonsE button);
void lcdDrawPointingArrow();

#endif // lcd_utils_h
