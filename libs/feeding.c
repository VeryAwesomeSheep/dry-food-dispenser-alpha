#include "feeding.h"
#include <string.h>
#include "motor.h"
#include <wiringPi.h>
#include "logger.h"

feedingScheduleS feedingSchedule = {0};

/*******************************************************************************
* saveFeedingSchedule
*
* @brief Saves the feeding schedule to the feeding.cfg file
*
* @return 0 on success, -1 on failure
*******************************************************************************/
int8_t saveFeedingSchedule() {
  char logMessageBuffer[120];

  FILE *fp = fopen("feeding.cfg", "w");
  if (fp == NULL) {
    sprintf(logMessageBuffer, "Error during feeding schedule saving: %s", strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    return -1;
  }

  fprintf(fp, "arms: %hhu\n", feedingSchedule.feedingWheelArms);

  for (uint8_t i = 0; i < feedingSchedule.activeFeedingTimes; i++) {
    fprintf(fp, "%hhu:%hhu - %hhu\n", feedingSchedule.feedingTime[i].hour, feedingSchedule.feedingTime[i].minute, feedingSchedule.feedingTime[i].portions);
  }

  fclose(fp);

  sprintf(logMessageBuffer, "Saved feeding schedule with %hhu active feedings "
    "times. Feeding wheel configured with %hhu arms",
    feedingSchedule.activeFeedingTimes,
    feedingSchedule.feedingWheelArms);
  logMessage(INFO, logMessageBuffer);

  return 0;
}

/*******************************************************************************
* loadFeedingSchedule
*
* @brief Loads the feeding schedule from the feeding.cfg file
*
* @return 0 on success, -1 on failure
*******************************************************************************/
int8_t loadFeedingSchedule() {
  char logMessageBuffer[120];

  FILE *fp = fopen("feeding.cfg", "r");
  if (fp == NULL) {
    sprintf(logMessageBuffer, "Error during feeding schedule loading: %s", strerror(errno));
    logMessage(ERROR, logMessageBuffer);
    feedingSchedule.feedingWheelArms = 4;
    return -1;
  }

  char line[20];
  uint8_t index = 0;

  if (fgets(line, sizeof(line), fp) != NULL) {
    sscanf(line, "arms: %hhu", &feedingSchedule.feedingWheelArms);
  }
  else {
    feedingSchedule.feedingWheelArms = 4;
    return -1;
  }

  while (fgets(line, sizeof(line), fp) != NULL && index < 10) {
    if (3 == sscanf(line, "%hhu:%hhu - %hhu",
      &feedingSchedule.feedingTime[index].hour,
      &feedingSchedule.feedingTime[index].minute,
      &feedingSchedule.feedingTime[index].portions)) {
      index++;
    }
    feedingSchedule.activeFeedingTimes++;
  }

  fclose(fp);

  sprintf(logMessageBuffer, "Loaded feeding schedule with %hhu active feedings "
    "times. Feeding wheel configured with %hhu arms",
    feedingSchedule.activeFeedingTimes,
    feedingSchedule.feedingWheelArms);

  logMessage(INFO, logMessageBuffer);

  return 0;
}

/*******************************************************************************
* saveModifiedFeedingTime
*
* @brief Modifies the feeding time at the specified index and reorders it in the
*        schedule by removing and adding it again
*
* @param[in] index The index of the feeding time to modify
* @param[in] hour The new hour for the feeding time
* @param[in] minute The new minute for the feeding time
*******************************************************************************/
void saveModifiedFeedingTime(uint8_t index, uint8_t hour, uint8_t minute) {
  uint8_t portions = feedingSchedule.feedingTime[index].portions;

  removeFeedingTime(index);

  addFeedingTime(hour, minute, portions);

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Feeding time index %hhu modified. New time %hhu:%hhu", index, hour, minute);
  logMessage(INFO, logMessageBuffer);
}

/*******************************************************************************
* removeFeedingTime
*
* @brief Removes a feeding time from the schedule and shifts the rest of the
* schedule
*
* @param[in] index The index of the feeding time to remove
*******************************************************************************/
void removeFeedingTime(uint8_t index) {
  uint8_t i;

  // Reset values at index
  feedingSchedule.feedingTime[index].hour = 0;
  feedingSchedule.feedingTime[index].minute = 0;
  feedingSchedule.feedingTime[index].portions = 0;
  feedingSchedule.activeFeedingTimes--;

  // Shift the rest of the schedule up one index
  for (i = index; i < feedingSchedule.activeFeedingTimes; i++) {
    feedingSchedule.feedingTime[i].hour = feedingSchedule.feedingTime[i + 1].hour;
    feedingSchedule.feedingTime[i].minute = feedingSchedule.feedingTime[i + 1].minute;
    feedingSchedule.feedingTime[i].portions = feedingSchedule.feedingTime[i + 1].portions;
  }

  // Clear the last entry
  feedingSchedule.feedingTime[i].hour = 0;
  feedingSchedule.feedingTime[i].minute = 0;
  feedingSchedule.feedingTime[i].portions = 0;

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Removed feeding time with index: %hhu", index);
  logMessage(INFO, logMessageBuffer);

  saveFeedingSchedule();
}

/*******************************************************************************
* addFeedingTime
*
* @brief Adds a feeding time to the schedule in the correct position.
*
* @param[in] hour Hour of the feeding time
* @param[in] minute Minute of the feeding time
* @param[in] portions Amount of portions for that feeding time
*******************************************************************************/
void addFeedingTime(uint8_t hour, uint8_t minute, uint8_t portions) {
  uint8_t i;
  uint8_t index = feedingSchedule.activeFeedingTimes;

  // Find correct index based on hour and minute
  for (i = 0; i < feedingSchedule.activeFeedingTimes; i++) {
    if ((feedingSchedule.feedingTime[i].hour > hour) ||
        (feedingSchedule.feedingTime[i].hour == hour &&
         feedingSchedule.feedingTime[i].minute > minute)) {
      index = i;
      break;
    }
  }

  // Move schedule 1 index down from the index for new feeding time
  if (feedingSchedule.activeFeedingTimes < 10) {
    for (i = feedingSchedule.activeFeedingTimes; i > index; i--) {
      feedingSchedule.feedingTime[i].hour = feedingSchedule.feedingTime[i - 1].hour;
      feedingSchedule.feedingTime[i].minute = feedingSchedule.feedingTime[i - 1].minute;
      feedingSchedule.feedingTime[i].portions = feedingSchedule.feedingTime[i - 1].portions;
    }

    // Add new feeding time at index
    feedingSchedule.feedingTime[index].hour = hour;
    feedingSchedule.feedingTime[index].minute = minute;
    feedingSchedule.feedingTime[index].portions = portions;
    feedingSchedule.activeFeedingTimes++;
  }

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Added feeding time with index: %hhu", index);
  logMessage(INFO, logMessageBuffer);

  saveFeedingSchedule();
}

/*******************************************************************************
* minutesToNextFeeding
*
* @brief Returns the number of minutes until the next feeding
*
* @return The number of minutes until the next feeding or UINT16_MAX if schedule
*         is empty
*******************************************************************************/
uint16_t minutesToNextFeeding() {
  if (feedingSchedule.activeFeedingTimes == 0) return UINT16_MAX;

  uint8_t nextFeedingIndex;
  time_t rawTime;
  struct tm *timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  // Look for the next feeding time today
  for (nextFeedingIndex = 0; nextFeedingIndex < feedingSchedule.activeFeedingTimes; nextFeedingIndex++) {
    if (feedingSchedule.feedingTime[nextFeedingIndex].hour > timeInfo->tm_hour ||
        (feedingSchedule.feedingTime[nextFeedingIndex].hour == timeInfo->tm_hour &&
         feedingSchedule.feedingTime[nextFeedingIndex].minute > timeInfo->tm_min)) {
      break;
    }
  }

  int8_t hoursDiff, minutesDiff;

  // If no feeding time is found today, return the time until the first feeding time tomorrow
  if (nextFeedingIndex == feedingSchedule.activeFeedingTimes) {
    hoursDiff = 24 - timeInfo->tm_hour + feedingSchedule.feedingTime[0].hour;
    minutesDiff = feedingSchedule.feedingTime[0].minute - timeInfo->tm_min;
  } else {
    hoursDiff = feedingSchedule.feedingTime[nextFeedingIndex].hour - timeInfo->tm_hour;
    minutesDiff = feedingSchedule.feedingTime[nextFeedingIndex].minute - timeInfo->tm_min;
  }

  if (minutesDiff < 0) {
    minutesDiff += 60;
    hoursDiff--;
  }

  if (hoursDiff < 0) {
    hoursDiff += 24;
  }

  return (hoursDiff * 60) + minutesDiff;
}

/*******************************************************************************
* isFeedingTimeDuplicate
*
* @brief Checks if the specified hour and minute is already in the schedule
*
* @param[in] hour The hour to check
* @param[in] minute The minute to check
*
* @return True if the time is a duplicate, false otherwise
*******************************************************************************/
bool isFeedingTimeDuplicate(uint8_t hour, uint8_t minute) {
  for (uint8_t i = 0; i < feedingSchedule.activeFeedingTimes; i++) {
    if (feedingSchedule.feedingTime[i].hour == hour && feedingSchedule.feedingTime[i].minute == minute) {
      return true;
    }
  }

  return false;
}

/*******************************************************************************
* handleFeeding
*
* @brief Handles the feeding process
******************************************************************************/
void handleFeeding() {
  uint8_t feedIndex;
  time_t rawTime;
  struct tm *timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  for (feedIndex = 0; feedIndex < feedingSchedule.activeFeedingTimes; feedIndex++) {
    if (feedingSchedule.feedingTime[feedIndex].hour == timeInfo->tm_hour &&
        feedingSchedule.feedingTime[feedIndex].minute == timeInfo->tm_min &&
        !feedingSchedule.feedingTime[feedIndex].isDone) {
      feed(feedingSchedule.feedingTime[feedIndex].portions);
      feedingSchedule.feedingTime[feedIndex].isDone = true;
      break;
    }
  }

  for (feedIndex = 0; feedIndex < feedingSchedule.activeFeedingTimes; feedIndex++) {
    if (feedingSchedule.feedingTime[feedIndex].isDone &&
        feedingSchedule.feedingTime[feedIndex].hour == timeInfo->tm_hour &&
        feedingSchedule.feedingTime[feedIndex].minute == timeInfo->tm_min + 1) {
      feedingSchedule.feedingTime[feedIndex].isDone = false;
      break;
    }
  }
}

/*******************************************************************************
* feed
*
* @brief Feeds the specified amount of portions
*
* @param[in] portions The amount of portions to feed
*******************************************************************************/
void feed(uint8_t portions) {
  for (uint8_t i = 0; i < portions; i++) {
    rotateMotor(360 / feedingSchedule.feedingWheelArms);
    delay(1000);
  }

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Feed %hhu portions", portions);
  logMessage(INFO, logMessageBuffer);
}

/*******************************************************************************
* getFeedingWheelArms
*
* @brief Returns the amount of arms on the feeding wheel
*
* @return Amount of arms on the feeding wheel
*******************************************************************************/
uint8_t getFeedingWheelArms() {
  return feedingSchedule.feedingWheelArms;
}

/*******************************************************************************
* setFeedingWheelArms
*
* @brief Sets the amount of arms on the feeding wheel
*
* @param[in] feedingWheelArms New amount of arms on the feeding wheel
*******************************************************************************/
void setFeedingWheelArms(uint8_t feedingWheelArms) {
  feedingSchedule.feedingWheelArms = feedingWheelArms;

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Set feeding wheel arms to %hhu", feedingSchedule.feedingWheelArms);
  logMessage(INFO, logMessageBuffer);

  saveFeedingSchedule();
}

/*******************************************************************************
* getFeedingTimeHour
*
* @brief Gets the hour of the feeding time at the specified index
*
* @param[in] index The index of the feeding time
*
* @return The hour of the feeding time
*******************************************************************************/
uint8_t getFeedingTimeHour(uint8_t index) {
  return feedingSchedule.feedingTime[index].hour;
}

/*******************************************************************************
* getFeedingTimeMinute
*
* @brief Gets the minute of the feeding time at the specified index
*
* @param[in] index The index of the feeding time
*
* @return The minute of the feeding time
*******************************************************************************/
uint8_t getFeedingTimeMinute(uint8_t index) {
  return feedingSchedule.feedingTime[index].minute;
}

/*******************************************************************************
* getFeedingTimePortions
*
* @brief Gets the amount of portions for the feeding time at the specified index
*
* @param[in] index The index of the feeding time
*
* @return The amount of portions for the feeding time
*******************************************************************************/
uint8_t getFeedingTimePortions(uint8_t index) {
  return feedingSchedule.feedingTime[index].portions;
}

/*******************************************************************************
* setFeedingTimePortions
*
* @brief Sets the amount of portions for the feeding time at the specified index
*
* @param[in] index The index of the feeding time
* @param[in] portions The amount of portions to set
*******************************************************************************/
void setFeedingTimePortions(uint8_t index, uint8_t portions) {
  feedingSchedule.feedingTime[index].portions = portions;

  char logMessageBuffer[120];
  sprintf(logMessageBuffer, "Set portions for feeding time index %hhu to %hhu", index, feedingSchedule.feedingTime[index].portions);
  logMessage(INFO, logMessageBuffer);

  saveFeedingSchedule();
}

/*******************************************************************************
* getActiveFeedingTimes
*
* @brief Gets the amount of active feeding times
*
* @return The amount of active feeding times
*******************************************************************************/
uint8_t getActiveFeedingTimes() {
  return feedingSchedule.activeFeedingTimes;
}
