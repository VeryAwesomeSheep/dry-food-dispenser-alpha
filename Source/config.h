#ifndef config_h
#define config_h

/* LCD Display */
#define LCD_ADDRESS 0x27

/* Buttons */
#define BUTTON_UP 5
#define BUTTON_DOWN 6
#define BUTTON_LEFT 19
#define BUTTON_RIGHT 26
#define BUTTON_FEED 16
#define DEBOUNCE_TIME 20

/* DC Motor */
#define MOTOR_ENCODER_A 23
#define MOTOR_ENCODER_B 24
#define MOTOR_M1A 12
#define MOTOR_M1B 13
#define MOTOR_GEAR_RATIO 74.83
#define MOTOR_CPR 48
#define MOTOR_ENCODER_TICKS_PER_DEGREE (MOTOR_CPR * MOTOR_GEAR_RATIO / 360)

#endif // config_h
