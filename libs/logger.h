#ifndef logger_h
#define logger_h

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

extern int errno;

typedef enum {
  INFO,
  WARNING,
  ERROR
} loggerEventType;

int8_t initLogger();
void logMessage(loggerEventType type, char *message);

#endif // logger_h