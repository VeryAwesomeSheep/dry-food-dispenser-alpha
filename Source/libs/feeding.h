#ifndef feeding_h
#define feeding_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef struct feedingTimeS {
  uint8_t hour; // Hour of the feeding time
  uint8_t minute; // Minute of the feeding time
  uint8_t portions; // Number of portions to feed in the feed
  bool isDone;
} feedingTimeS;

typedef struct feedingScheduleS {
  uint8_t activeFeedingTimes; // Number of active feeding times
  feedingTimeS feedingTime[10]; // Max 10 feeding times per day
  uint8_t feedingWheelArms; // Arms of the feeding wheel
} feedingScheduleS;

int8_t saveFeedingSchedule();
int8_t loadFeedingSchedule();
void saveModifiedFeedingTime(uint8_t index, uint8_t hour, uint8_t minute);
void removeFeedingTime(uint8_t index);
void addFeedingTime(uint8_t hour, uint8_t minute, uint8_t portions);
uint16_t minutesToNextFeeding();
bool isFeedingTimeDuplicate(uint8_t hour, uint8_t minute);
void handleFeeding();
void feed(uint8_t portions);
uint8_t getFeedingWheelArms();
void setFeedingWheelArms(uint8_t feedingWheelArms);
uint8_t getFeedingTimeHour(uint8_t index);
uint8_t getFeedingTimeMinute(uint8_t index);
uint8_t getFeedingTimePortions(uint8_t index);
void setFeedingTimePortions(uint8_t index, uint8_t portions);
uint8_t getActiveFeedingTimes();

#endif // feeding_h
