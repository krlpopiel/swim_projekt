// lcd_hd44780.h
#ifndef __LCD_HD44780_H__
#define __LCD_HD44780_H__

#include "stm32f4xx_hal.h"

// Definicje pinów
#define LCD_RS_GPIO_Port   GPIOB
#define LCD_RS_Pin         GPIO_PIN_15
#define LCD_EN_GPIO_Port   GPIOB
#define LCD_EN_Pin         GPIO_PIN_10

#define LCD_D0_GPIO_Port   GPIOC
#define LCD_D0_Pin         GPIO_PIN_0
#define LCD_D1_GPIO_Port   GPIOC
#define LCD_D1_Pin         GPIO_PIN_1
#define LCD_D2_GPIO_Port   GPIOC
#define LCD_D2_Pin         GPIO_PIN_2
#define LCD_D3_GPIO_Port   GPIOC
#define LCD_D3_Pin         GPIO_PIN_3
#define LCD_D4_GPIO_Port   GPIOA
#define LCD_D4_Pin         GPIO_PIN_9
#define LCD_D5_GPIO_Port   GPIOA
#define LCD_D5_Pin         GPIO_PIN_11
#define LCD_D6_GPIO_Port   GPIOA
#define LCD_D6_Pin         GPIO_PIN_12
#define LCD_D7_GPIO_Port   GPIOA
#define LCD_D7_Pin         GPIO_PIN_8

// API
void LCD_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendString(char* str);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Clear(void);
void LCD_DrawFace(uint8_t direction);

#endif
