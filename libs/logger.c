#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "logger.h"
#include <string.h>

const char* loggerEventStrings[] = {
  "INFO",
  "WARNING",
  "ERROR"
};

char loggerFileName[25] = {0};

/*******************************************************************************
* initLogger
*
* @brief Initializes the logger file
*
* @return 0 on success, -1 on failure
*******************************************************************************/
int8_t initLogger() {
  time_t rawTime;
  struct tm* timeInfo;
  char buffer[14];

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  strftime(buffer, sizeof(buffer), "%d%m%y_%H%M%S", timeInfo);

  sprintf(loggerFileName, "logger_%s.log", buffer);

  FILE *fp = fopen(loggerFileName, "ab+");
  if (fp == NULL) {
    return -1;
  }

  fclose(fp);

  return 0;
}

/*******************************************************************************
* logMessage
*
* @brief Logs a message to the logger file
*
* @param[in] type The type of message to be logged
* @param[in] message The message to be logged
*******************************************************************************/
void logMessage(loggerEventType type, char *message) {
  time_t rawTime;
  struct tm* timeInfo;
  char buffer[22];

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  strftime(buffer, sizeof(buffer), "%d.%m.%Y - %H:%M:%S", timeInfo);

  FILE *fp = fopen(loggerFileName, "a");
  if (fp == NULL) {
    fprintf(stderr, "Error during logging: %s", strerror(errno));
  }

  fprintf(fp, "(%s) [%s] %s\n", buffer, loggerEventStrings[type], message);
  fprintf(stderr, "(%s) [%s] %s\n", buffer, loggerEventStrings[type], message);

  fclose(fp);
}