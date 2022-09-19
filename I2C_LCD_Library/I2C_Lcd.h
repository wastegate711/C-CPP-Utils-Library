#ifndef TESTPROJECT_I2C_LCD_H
#define TESTPROJECT_I2C_LCD_H

#define E_SET   0x04
#define RS_SET  0x01
#define SET_LED 0x08

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <i2c.h>
#include <string.h>

void I2C_Lcd_InitializationLibrary(I2C_HandleTypeDef handle, uint16_t addressLcd);
void I2C_Lcd_LcdInitialization();
void I2C_Lcd_SetCursor(uint8_t x, uint8_t y);
void I2C_Lcd_Print_text(char *message);
void I2C_Lcd_Print_symbol(uint8_t symbol);
void I2C_Lcd_Move_to_the_left(void);
void I2C_Lcd_Backlight(bool state);
void I2C_Lcd_Create_symbol(uint8_t *my_Symbol, uint8_t memory_address);
void I2C_Lcd_Clean(void);

#endif //TESTPROJECT_I2C_LCD_H
