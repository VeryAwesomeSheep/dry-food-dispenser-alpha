#include "lcd.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>

lcd_paramsS lcd_params;

void lcd_init_i2c(uint8_t address, uint8_t cols, uint8_t rows, uint8_t charsize) {
  lcd_params.address = address;
  lcd_params.fd = wiringPiI2CSetup(address);
  delayMicroseconds(15000);
  lcd_params.bitmode = 0;
  lcd_params.charsize = charsize;

  lcd_begin_i2c(cols, rows);
}

void lcd_begin_i2c(uint8_t cols, uint8_t rows) {
  if (rows > 1) {
    lcd_params.function |= LCD_2LINE;
  }

  lcd_params.rows = rows;

  lcd_setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  if (lcd_params.charsize != LCD_5x8DOTS && rows == 1) {
    lcd_params.function |= LCD_5x10DOTS;
  } else {
    lcd_params.function |= LCD_5x8DOTS;
  }

  lcd_command_i2c(0x03);
  delayMicroseconds(4500);

  lcd_command_i2c(0x03);
  delayMicroseconds(4500);

  lcd_command_i2c(0x03);
  delayMicroseconds(150);

  lcd_command_i2c(0x02);

  lcd_command_i2c(LCD_FUNCTIONSET | lcd_params.function);

  lcd_params.control = LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON;

  lcd_clear_i2c();

  lcd_params.mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

  lcd_command_i2c(LCD_ENTRYMODESET | lcd_params.mode);
}

void lcd_setRowOffsets(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4) {
  lcd_params.row_offsets[0] = row1;
  lcd_params.row_offsets[1] = row2;
  lcd_params.row_offsets[2] = row3;
  lcd_params.row_offsets[3] = row4;
}

void lcd_command_i2c(uint8_t value) {
  lcd_send_i2c(value, LOW);
}

void lcd_send_i2c(uint8_t value, uint8_t mode) {
  wiringPiI2CWrite(lcd_params.fd, mode | (value & 0xF0) | LCD_DISPLAYCONTROL);
  lcd_pulse_i2c(mode | (value & 0xF0) | LCD_DISPLAYCONTROL);
  wiringPiI2CWrite(lcd_params.fd, mode | ((value << 4) & 0xF0) | LCD_DISPLAYCONTROL);
  lcd_pulse_i2c(mode | ((value << 4) & 0xF0) | LCD_DISPLAYCONTROL);
}

void lcd_pulse_i2c(uint8_t value) {
  wiringPiI2CWrite(lcd_params.fd, value | LCD_DISPLAYON);
  delayMicroseconds(500);
  wiringPiI2CWrite(lcd_params.fd, value & ~LCD_DISPLAYON);
  delayMicroseconds(500);
}

void lcd_writeChar_i2c(char c) {
  lcd_send_i2c(c, HIGH);
}

void lcd_writeString_i2c(char *string) {
  while (*string) {
    lcd_writeChar_i2c(*string++);
  }
}

void lcd_removeChar_i2c(uint8_t col, uint8_t row) {
  lcd_setCursor_i2c(col, row);
  lcd_writeChar_i2c(' ');
  lcd_setCursor_i2c(col, row);
}

void lcd_clear_i2c() {
  lcd_command_i2c(LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

void lcd_cursorOff_i2c() {
  lcd_params.control &= ~LCD_CURSORON;
  lcd_command_i2c(LCD_DISPLAYCONTROL | lcd_params.control);
}

void lcd_blinkOn_i2c() {
  lcd_params.control |= LCD_BLINKON;
  lcd_command_i2c(LCD_DISPLAYCONTROL | lcd_params.control);
}

void lcd_blinkOff_i2c() {
  lcd_params.control &= ~LCD_BLINKON;
  lcd_command_i2c(LCD_DISPLAYCONTROL | lcd_params.control);
}

void lcd_setCursor_i2c(uint8_t col, uint8_t row) {
  const uint8_t max_rows = sizeof(lcd_params.row_offsets) / sizeof(lcd_params.row_offsets[0]);

  if (row >= max_rows) {
    row = max_rows - 1;
  }

  if (row >= lcd_params.rows) {
    row = lcd_params.rows - 1;
  }

  lcd_command_i2c(LCD_SETDDRAMADDR | (col + lcd_params.row_offsets[row]));
}
