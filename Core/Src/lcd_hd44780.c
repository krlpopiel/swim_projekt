#include "lcd_hd44780.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include <stdint.h>


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


//easter egg

void LCD_CreateChar(uint8_t location, uint8_t charmap[]) {
    location &= 0x7; // tylko 0-7
    LCD_SendCommand(0x40 | (location << 3));
    for (int i = 0; i < 8; i++) {
        LCD_SendData(charmap[i]);
    }
}

// Lewo
uint8_t eye_left[8] = {
    0b11111,
    0b10001,
    0b10101,
    0b10101,
    0b10001,
    0b11111,
    0b00000,
    0b00000
};

// Prosto
uint8_t eye_forward[8] = {
    0b11111,
    0b10001,
    0b10001,
    0b10101,
    0b10001,
    0b11111,
    0b00000,
    0b00000
};

// Prawo
uint8_t eye_right[8] = {
    0b11111,
    0b10001,
    0b10101,
    0b10101,
    0b10001,
    0b11111,
    0b00000,
    0b00000
};

// Nos – prosto
uint8_t nose_forward[8] = {
    0b00000,
    0b00000,
    0b00100,
    0b00100,
    0b01010,
    0b01010,
    0b10001,
    0b00000
};

// Nos – w lewo
uint8_t nose_left[8] = {
    0b00000,
    0b00000,
    0b00010,
    0b00010,
    0b00101,
    0b00101,
    0b01001,
    0b00000
};

// Nos – w prawo
uint8_t nose_right[8] = {
    0b00000,
    0b00000,
    0b01000,
    0b01000,
    0b10100,
    0b10100,
    0b10010,
    0b00000
};


void LCD_DrawFace(uint8_t direction) {
    // direction: 0 = lewo, 1 = prosto, 2 = prawo

    switch (direction) {
        case 0:
            LCD_CreateChar(0, eye_left);
            LCD_CreateChar(1, nose_left);
            break;
        case 1:
            LCD_CreateChar(0, eye_forward);
            LCD_CreateChar(1, nose_forward);
            break;
        case 2:
            LCD_CreateChar(0, eye_right);
            LCD_CreateChar(1, nose_right);
            break;
    }

    LCD_Clear();

    // Górna linia: oczy
    LCD_SetCursor(0, 6);
    LCD_SendData(0);
    LCD_SendData(' ');
    LCD_SendData(0);

    // Dolna linia: nos
    LCD_SetCursor(1, 7);
    LCD_SendData(1);
}
