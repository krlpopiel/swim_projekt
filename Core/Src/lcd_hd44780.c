#include "lcd_hd44780.h"
#include "stm32l0xx_hal.h"
#include "string.h"

static void LCD_EnablePulse(void) {
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_Write8Bits(uint8_t data) {
    HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_EnablePulse();
}

static void LCD_Send(uint8_t data, uint8_t rs) {
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_Write8Bits(data);
}

void LCD_SendCommand(uint8_t cmd) {
    LCD_Send(cmd, 0);
}

void LCD_SendData(uint8_t data) {
    LCD_Send(data, 1);
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    LCD_SendCommand(0x80 | (addr + col));
}

void LCD_SendString(char* str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

void LCD_Init(void) {
    HAL_Delay(100);

    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    LCD_Write8Bits(0x30);
     HAL_Delay(5);
     LCD_Write8Bits(0x30);
     HAL_Delay(5);
     LCD_Write8Bits(0x30);
     HAL_Delay(5);
    LCD_SendCommand(0x38);
    HAL_Delay(5);
    LCD_SendCommand(0x0F);
    HAL_Delay(2);
    LCD_SendCommand(0x01);
    HAL_Delay(2);
    LCD_SendCommand(0x02);
    HAL_Delay(2);
    LCD_Clear();
}
