/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "stm32f4xx_hal.h"
#include <lcd_hd44780.h>
#include <stdio.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define IR_RX_BUFFER_SIZE 100

#ifndef IR_Pin
#define IR_Pin GPIO_PIN_10
#endif
#ifndef IR_GPIO_Port
#define IR_GPIO_Port GPIOA
#endif

#define NEC_TIMEOUT_US 100000
#define NEC_FRAME_START_MIN_GAP_US 12000

#define NEC_AGC_MARK_MIN_US 8000
#define NEC_AGC_MARK_MAX_US 10000
#define NEC_AGC_SPACE_MIN_US 4000
#define NEC_AGC_SPACE_MAX_US 5000

#define NEC_BIT_MARK_MIN_US 400
#define NEC_BIT_MARK_MAX_US 750

#define NEC_BIT_0_SPACE_MIN_US 400
#define NEC_BIT_0_SPACE_MAX_US 750

#define NEC_BIT_1_SPACE_MIN_US 1500
#define NEC_BIT_1_SPACE_MAX_US 1900

#define NEC_PULSE_COUNT 67

#define NEC_REPEAT_SPACE_MIN_US 2000
#define NEC_REPEAT_SPACE_MAX_US 2500
#define NEC_REPEAT_PULSE_COUNT 3

#define TIM_IR_PRESCALER (15)

#define DISCRETE_CMD_MIN_INTERVAL_MS 400
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
int lewy, prawy, srodek;
uint32_t duty = 1400;
#define DUTY_FAST 1400
#define DUTY_SLOW 1150
#define DUTY_STOP 0
int bitSkretu = 1;

volatile uint8_t control_mode = 0;

volatile uint32_t last_edge_time = 0;
volatile uint32_t pulse_widths[IR_RX_BUFFER_SIZE];
volatile uint8_t pulse_index = 0;
volatile bool ir_frame_ready = false;
volatile bool capture_started = false;

volatile bool ir_repeat_detected = false;
volatile uint32_t last_good_ir_code = 0xFFFFFFFF;

volatile bool is_moving_remotely = false;
volatile uint32_t last_remote_movement_command_time = 0;
#define REMOTE_MOVEMENT_TIMEOUT 300

uint32_t last_processed_discrete_ir_code = 0xFFFFFFFF;
uint32_t last_processed_discrete_ir_time = 0;

static bool lcd_initialized = false;

typedef struct
{
    uint16_t frequency; // Hz
    uint16_t duration;  // ms
} Note;

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_A5 880
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_D6 1175
#define NOTE_REST 0

Note happy_bounce[] = {
    {NOTE_C5, 300}, {NOTE_REST, 50}, {NOTE_E5, 300}, {NOTE_REST, 50}, {NOTE_G5, 300}, {NOTE_REST, 50}, {NOTE_C6, 600}, {NOTE_REST, 100},

    {NOTE_B5, 300},
    {NOTE_REST, 50},
    {NOTE_G5, 300},
    {NOTE_REST, 50},
    {NOTE_E5, 300},
    {NOTE_REST, 50},
    {NOTE_C5, 600},
    {NOTE_REST, 100},

    {NOTE_D5, 300},
    {NOTE_REST, 50},
    {NOTE_F5, 300},
    {NOTE_REST, 50},
    {NOTE_A5, 300},
    {NOTE_REST, 50},
    {NOTE_B5, 300},
    {NOTE_REST, 50},
    {NOTE_C6, 600},
    {NOTE_REST, 100},

    {NOTE_C6, 150},
    {NOTE_B5, 150},
    {NOTE_A5, 150},
    {NOTE_G5, 150},
    {NOTE_F5, 150},
    {NOTE_E5, 150},
    {NOTE_D5, 150},
    {NOTE_C5, 600},
    {NOTE_REST, 200},

    {NOTE_REST, 0}};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void play_note(uint16_t freq, uint16_t duration_ms)
{
    if (freq == 0)
    {
        HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
        HAL_Delay(duration_ms);
        return;
    }

    uint32_t tim_clock = 16000000;
    uint16_t prescaler = 71;
    uint32_t period = (tim_clock / (prescaler + 1)) / freq - 1;

    __HAL_TIM_SET_PRESCALER(&htim4, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim4, period);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, period / 2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

    HAL_Delay(duration_ms);

    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
}

void play_melody(Note *melody)
{
    for (int i = 0; melody[i].duration > 0; i++)
    {
        play_note(melody[i].frequency, melody[i].duration);
    }
}

uint32_t decode_nec(volatile uint32_t *pulses, uint8_t count);

void dzidaDoPrzodu(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void doTylu(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void skretWLewo(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void skretWPrawo(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void stop(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
}

void lewy90(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void prawy90(void)
{
    HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty);
}

void searchLine(int lastTurnDirection)
{
    uint32_t search_duty = DUTY_SLOW;

    if (lastTurnDirection >= 0)
    {
        HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC, IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, IN2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, IN3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, IN4_Pin, GPIO_PIN_RESET);
    }
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, search_duty);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, search_duty);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == IR_Pin)
    {
        if (ir_frame_ready || ir_repeat_detected)
            return;

        uint32_t now = __HAL_TIM_GET_COUNTER(&htim3);
        uint32_t duration = (uint16_t)(now - last_edge_time);

        if (duration > NEC_FRAME_START_MIN_GAP_US)
        {
            pulse_index = 0;
            capture_started = true;
            last_edge_time = now;
            return;
        }

        if (capture_started)
        {
            if (pulse_index < IR_RX_BUFFER_SIZE)
            {
                pulse_widths[pulse_index++] = duration;
            }
            else
            {
                capture_started = false;
                pulse_index = 0;
            }

            if (capture_started)
            {
                if (pulse_index == NEC_REPEAT_PULSE_COUNT)
                {
                    if ((pulse_widths[0] >= NEC_AGC_MARK_MIN_US && pulse_widths[0] <= NEC_AGC_MARK_MAX_US) &&
                        (pulse_widths[1] >= NEC_REPEAT_SPACE_MIN_US && pulse_widths[1] <= NEC_REPEAT_SPACE_MAX_US) &&
                        (pulse_widths[2] >= NEC_BIT_MARK_MIN_US && pulse_widths[2] <= NEC_BIT_MARK_MAX_US))
                    {
                        ir_repeat_detected = true;
                        capture_started = false;
                    }
                }

                if (!ir_repeat_detected && pulse_index >= NEC_PULSE_COUNT)
                {
                    ir_frame_ready = true;
                    capture_started = false;
                }
            }
        }
        last_edge_time = now;
    }
}

uint32_t decode_nec(volatile uint32_t *pulses, uint8_t count)
{
    if (count < NEC_PULSE_COUNT)
        return 0xFFFFFFFF;

    if (pulses[0] < NEC_AGC_MARK_MIN_US || pulses[0] > NEC_AGC_MARK_MAX_US)
        return 0xFFFFFFFF;
    if (pulses[1] < NEC_AGC_SPACE_MIN_US || pulses[1] > NEC_AGC_SPACE_MAX_US)
        return 0xFFFFFFFF;

    uint32_t decoded_code = 0;
    for (int i = 0; i < 32; i++)
    {
        uint32_t bit_mark = pulses[2 + (2 * i)];
        uint32_t bit_space = pulses[2 + (2 * i) + 1];

        if (bit_mark < NEC_BIT_MARK_MIN_US || bit_mark > NEC_BIT_MARK_MAX_US)
            return 0xFFFFFFFF;

        decoded_code <<= 1;
        if (bit_space >= NEC_BIT_1_SPACE_MIN_US && bit_space <= NEC_BIT_1_SPACE_MAX_US)
        {
            decoded_code |= 1;
        }
        else if (bit_space >= NEC_BIT_0_SPACE_MIN_US && bit_space <= NEC_BIT_0_SPACE_MAX_US)
        {
        }
        else
        {
            return 0xFFFFFFFF;
        }
    }
    return decoded_code;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    char lcd_buf[17];
    char lcd_action_msg[17];
    char clear_line[17];
    memset(clear_line, ' ', 16);
    clear_line[16] = '\0';
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
    LCD_Init();
    LCD_DrawFace(1);
    play_melody(happy_bounce);
    lcd_initialized = true;
    LCD_SetCursor(0, 0);
    LCD_SendString("PROJEKT SWIM");
    LCD_SetCursor(1, 0);
    LCD_SendString("Czekam na B1...");

    HAL_TIM_Base_Start(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    last_edge_time = __HAL_TIM_GET_COUNTER(&htim3);
    pulse_index = 0;
    ir_frame_ready = false;
    ir_repeat_detected = false;
    capture_started = false;

    while (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET)
    {
        HAL_Delay(100);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_SendString("Tryb: Auto");
    LCD_SetCursor(1, 0);
    LCD_SendString("Start!");
    HAL_Delay(1000);
    LCD_DrawFace(1);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    stop();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
        uint32_t ir_code_to_process = 0xFFFFFFFF;
        bool is_repeat_frame = false;

        if (ir_frame_ready)
        {
            uint32_t local_pulse_widths[IR_RX_BUFFER_SIZE];
            uint8_t local_pulse_count;

            HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

            local_pulse_count = pulse_index;
            memcpy(local_pulse_widths, (void *)pulse_widths, local_pulse_count * sizeof(uint32_t));

            pulse_index = 0;
            ir_frame_ready = false;
            HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

            uint32_t decoded_code = decode_nec(local_pulse_widths, local_pulse_count);

            if (decoded_code != 0xFFFFFFFF)
            {
                ir_code_to_process = decoded_code;
                last_good_ir_code = decoded_code;
                is_repeat_frame = false;
            }
            else
            {
                LCD_SetCursor(0, 0);
                LCD_SendString(clear_line);
                LCD_SetCursor(1, 0);
                LCD_SendString(clear_line);
                LCD_SetCursor(0, 0);
                LCD_SendString("Blad dekodowania");
            }
        }
        else if (ir_repeat_detected)
        {
            HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
            ir_repeat_detected = false;
            HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

            if (last_good_ir_code != 0xFFFFFFFF)
            {
                ir_code_to_process = last_good_ir_code;
                is_repeat_frame = true;
            }
            else
            {
            }
        }

        if (ir_code_to_process != 0xFFFFFFFF)
        {
            bool allow_processing = true;
            memset(lcd_action_msg, 0, sizeof(lcd_action_msg));

            LCD_SetCursor(0, 0);
            LCD_SendString(clear_line);
            LCD_SetCursor(1, 0);
            LCD_SendString(clear_line);
            LCD_SetCursor(0, 0);

            if (is_repeat_frame)
            {
                sprintf(lcd_buf, "IR RPT(0x%04X)", (unsigned int)(ir_code_to_process & 0xFFFF));
            }
            else
            {
                sprintf(lcd_buf, "IR:0x%08X", (unsigned int)ir_code_to_process);
            }
            LCD_SendString(lcd_buf);

            bool is_discrete_event_code =
                (ir_code_to_process == 0x00FF6897) ||
                (ir_code_to_process == 0x00FFB04F) ||
                (ir_code_to_process == 0x00FF38C7) ||
                (ir_code_to_process == 0x00FFA25D) ||
                (ir_code_to_process == 0x00FFE21D);

            if (is_discrete_event_code)
            {
                if (ir_code_to_process == last_processed_discrete_ir_code &&
                    (HAL_GetTick() - last_processed_discrete_ir_time < DISCRETE_CMD_MIN_INTERVAL_MS))
                {
                    allow_processing = false;
                    if (is_repeat_frame)
                        sprintf(lcd_action_msg, "Hold Ignored");
                    else
                        sprintf(lcd_action_msg, "Debounce Ignored");
                }
            }

            if (allow_processing)
            {
                bool discrete_action_has_been_processed_this_cycle = false;

                if (ir_code_to_process == 0x00FF6897)
                {
                    if (control_mode != 1)
                    {
                        control_mode = 1;
                        stop();
                        is_moving_remotely = false;
                    }
                    sprintf(lcd_action_msg, "Tryb: Zdalny");
                    discrete_action_has_been_processed_this_cycle = true;
                }
                else if (ir_code_to_process == 0x00FFB04F)
                {
                    if (control_mode != 0)
                    {
                        control_mode = 0;
                        stop();
                        is_moving_remotely = false;
                        bitSkretu = 0;
                    }
                    sprintf(lcd_action_msg, "Tryb: Auto");
                    discrete_action_has_been_processed_this_cycle = true;
                }
                else if (control_mode == 1)
                {
                    switch (ir_code_to_process)
                    {
                    case 0x00FF18E7:
                        dzidaDoPrzodu();
                        sprintf(lcd_action_msg, "Cmd: Naprzod");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;
                    case 0x00FF4AB5:
                        doTylu();
                        sprintf(lcd_action_msg, "Cmd: Tyl");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;
                    case 0x00FF10EF:
                        lewy90();
                        sprintf(lcd_action_msg, "Cmd: Obr Lewo");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;
                    case 0x00FF5AA5:
                        prawy90();
                        sprintf(lcd_action_msg, "Cmd: Obr Prawo");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;
                    case 0x00FF22DD:
                        skretWLewo();
                        sprintf(lcd_action_msg, "Cmd: Skret L");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;
                    case 0x00FFC23D:
                        skretWPrawo();
                        sprintf(lcd_action_msg, "Cmd: Skret P");
                        is_moving_remotely = true;
                        last_remote_movement_command_time = HAL_GetTick();
                        break;

                    case 0x00FF38C7:
                        stop();
                        sprintf(lcd_action_msg, "Cmd: STOP");
                        is_moving_remotely = false;
                        discrete_action_has_been_processed_this_cycle = true;
                        break;
                    case 0x00FFA25D:
                        duty = DUTY_SLOW;
                        sprintf(lcd_action_msg, "Predk: Wolno");
                        if (is_moving_remotely)
                            last_remote_movement_command_time = HAL_GetTick();
                        discrete_action_has_been_processed_this_cycle = true;
                        break;
                    case 0x00FFE21D:
                        duty = DUTY_FAST;
                        sprintf(lcd_action_msg, "Predk: Szybko");
                        if (is_moving_remotely)
                            last_remote_movement_command_time = HAL_GetTick();
                        discrete_action_has_been_processed_this_cycle = true;
                        break;

                    default:
                        if (lcd_action_msg[0] == '\0')
                        {
                            sprintf(lcd_action_msg, "IR Kod ???");
                        }
                        break;
                    }
                }
                else
                {
                    if (lcd_action_msg[0] == '\0' &&
                        ir_code_to_process != 0x00FF6897 &&
                        ir_code_to_process != 0x00FFB04F)
                    {
                        sprintf(lcd_action_msg, "IR (Tryb Auto)");
                    }
                }

                if (discrete_action_has_been_processed_this_cycle)
                {
                    last_processed_discrete_ir_code = ir_code_to_process;
                    last_processed_discrete_ir_time = HAL_GetTick();
                }
            }

            if (lcd_action_msg[0] != '\0')
            {
                LCD_SetCursor(1, 0);
                LCD_SendString(lcd_action_msg);
            }
        }

        if (control_mode == 1)
        {
            if (is_moving_remotely && (HAL_GetTick() - last_remote_movement_command_time > REMOTE_MOVEMENT_TIMEOUT))
            {
                stop();
                is_moving_remotely = false;

                LCD_SetCursor(0, 0);
                LCD_SendString(clear_line);
                LCD_SetCursor(1, 0);
                LCD_SendString(clear_line);
                LCD_SetCursor(0, 0);
                LCD_SendString("Pilot Timeout!");
                LCD_SetCursor(1, 0);
                LCD_SendString("STOP");
            }
            HAL_Delay(20);
        }
        else
        {
            lewy = HAL_GPIO_ReadPin(IR1_GPIO_Port, IR1_Pin);
            srodek = HAL_GPIO_ReadPin(IR2_GPIO_Port, IR2_Pin);
            prawy = HAL_GPIO_ReadPin(IR3_GPIO_Port, IR3_Pin);

            if (lewy && srodek && prawy)
            {
                searchLine(bitSkretu);
            }
            else if (!lewy && !srodek && !prawy)
            {
                duty = DUTY_SLOW;
                dzidaDoPrzodu();
                bitSkretu = 0;
                LCD_DrawFace(1);
            }
            else if (lewy && !srodek && prawy)
            {
                duty = DUTY_FAST;
                dzidaDoPrzodu();
                bitSkretu = 0;
                LCD_DrawFace(1);
            }
            else if (!lewy && !srodek && prawy)
            {
                duty = DUTY_SLOW;
                skretWLewo();
                bitSkretu = 1;
                LCD_DrawFace(0);
            }
            else if (lewy && !srodek && !prawy)
            {
                duty = DUTY_SLOW;
                skretWPrawo();
                bitSkretu = -1;
                LCD_DrawFace(2);
            }
            else if (!lewy && srodek && prawy)
            {
                duty = DUTY_SLOW;
                skretWLewo();
                bitSkretu = 1;
                LCD_DrawFace(0);
            }
            else if (lewy && srodek && !prawy)
            {
                duty = DUTY_SLOW;
                skretWPrawo();
                bitSkretu = -1;
                LCD_DrawFace(2);
            }
            else
            {
                searchLine(bitSkretu);
            }
            HAL_Delay(50);
        }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */
  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1600;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */
  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */
  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 504;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |IN4_Pin|IN3_Pin|IN1_Pin|IN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|D7_Pin|D4_Pin|D5_Pin
                          |D6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, E_Pin|RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : D0_Pin D1_Pin D2_Pin D3_Pin
                           IN4_Pin IN3_Pin IN1_Pin IN2_Pin */
  GPIO_InitStruct.Pin = D0_Pin|D1_Pin|D2_Pin|D3_Pin
                          |IN4_Pin|IN3_Pin|IN1_Pin|IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Pin D7_Pin D4_Pin D5_Pin
                           D6_Pin */
  GPIO_InitStruct.Pin = LED_Pin|D7_Pin|D4_Pin|D5_Pin
                          |D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : E_Pin RS_Pin */
  GPIO_InitStruct.Pin = E_Pin|RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_Pin */
  GPIO_InitStruct.Pin = IR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IR1_Pin IR3_Pin IR2_Pin */
  GPIO_InitStruct.Pin = IR1_Pin|IR3_Pin|IR2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();

    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_SendString("Error_Handler!");
    LCD_SetCursor(1, 0);
    LCD_SendString("System Zatrzymany");

    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(200);
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    char msg[32];
    __disable_irq();

    if (lcd_initialized)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_SendString("ASSERT FAILED");
        LCD_SetCursor(1, 0);
        sprintf(msg, "Linia: %u", (unsigned int)line);
        LCD_SendString(msg);
    }
    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(100);
    }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
