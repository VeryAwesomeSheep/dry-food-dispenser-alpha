#ifndef lcd_h
#define lcd_h

#include <stdint.h>
#include <wiringPi.h>

// Commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// Flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// Flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

typedef struct lcd_paramsS {
  uint8_t RS; // RS pin
  uint8_t E; // Enable pin
  uint8_t D0; // Data pins
  uint8_t D1;
  uint8_t D2;
  uint8_t D3;
  uint8_t D4;
  uint8_t D5;
  uint8_t D6;
  uint8_t D7;
  uint8_t address; // I2C address
  int8_t fd; // File descriptor
  uint8_t cols; // Number of columns
  uint8_t rows; // Number of rows
  uint8_t charsize; // Character size
  uint8_t function; // Function set
  uint8_t control; // Display control
  uint8_t mode; // Entry mode
  uint8_t bitmode; // 4 (0) or 8 (1) bit mode
  uint8_t row_offsets[4]; // Row offsets
} lcd_paramsS;


void lcd_init_i2c(uint8_t address, uint8_t cols, uint8_t rows, uint8_t charsize);

void lcd_begin_i2c(uint8_t cols, uint8_t rows);
void lcd_setRowOffsets(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);

void lcd_command_i2c(uint8_t value);
void lcd_send_i2c(uint8_t value, uint8_t mode);
void lcd_pulse_i2c(uint8_t value);

void lcd_writeChar_i2c(char c);
void lcd_writeString_i2c(char *s);
void lcd_removeChar_i2c(uint8_t col, uint8_t row);
void lcd_clear_i2c();
void lcd_cursorOff_i2c();
void lcd_blinkOn_i2c();
void lcd_blinkOff_i2c();
void lcd_setCursor_i2c(uint8_t col, uint8_t row);

#endif // lcd_h
