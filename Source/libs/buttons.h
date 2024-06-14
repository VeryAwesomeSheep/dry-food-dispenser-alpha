#ifndef buttons_h
#define buttons_h

#include <stdint.h>
#include <stdbool.h>

typedef struct buttonS {
  uint8_t pin;
  bool previousState;
  uint32_t lastDebounceTime;
  volatile bool state;
  uint8_t debounceTime;
} buttonS;

uint8_t initButtons();
void debounceButtons();
void debounceButton(buttonS *button);
void buttonUpISR();
void buttonDownISR();
void buttonLeftISR();
void buttonRightISR();
void buttonFeedISR();

#endif // buttons_h
