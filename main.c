#include "stm32f1xx_hal.h"

#define IGNITION_PIN              GPIO_PIN_0  // PA0: Ignition switch
#define DRIVER_OCCUPANCY_PIN      GPIO_PIN_1  // PA1: Driver occupancy switch
#define DRIVER_SEATBELT_PIN       GPIO_PIN_2  // PA2: Driver seat belt switch
#define PASSENGER_OCCUPANCY_PIN   GPIO_PIN_3  // PA3: Passenger occupancy switch
#define PASSENGER_SEATBELT_PIN    GPIO_PIN_4  // PA4: Passenger seat belt switch
#define DRIVER_WARNING_LED_PIN    GPIO_PIN_0  // PB0: Driver warning LED
#define PASSENGER_WARNING_LED_PIN GPIO_PIN_1  // PB1: Passenger warning LED
#define BUZZER_PIN                GPIO_PIN_2  // PB2: Buzzer

// GPIO Port Definitions
#define INPUT_PORT                GPIOA      // All inputs on GPIOA
#define OUTPUT_PORT               GPIOB      // All outputs on GPIOB

// Function prototypes
void SystemClock_Config(void);
void GPIO_Init(void);
void update_warnings(GPIO_PinState driver_warning, GPIO_PinState passenger_warning);


int main(void) {
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();

  while (1) {
    GPIO_PinState ignition = HAL_GPIO_ReadPin(INPUT_PORT, IGNITION_PIN);
    GPIO_PinState driver_occupancy = HAL_GPIO_ReadPin(INPUT_PORT, DRIVER_OCCUPANCY_PIN);
    GPIO_PinState driver_seatbelt = HAL_GPIO_ReadPin(INPUT_PORT, DRIVER_SEATBELT_PIN);
    GPIO_PinState passenger_occupancy = HAL_GPIO_ReadPin(INPUT_PORT, PASSENGER_OCCUPANCY_PIN);
    GPIO_PinState passenger_seatbelt = HAL_GPIO_ReadPin(INPUT_PORT, PASSENGER_SEATBELT_PIN);

    // Warning if: ignition on, seat occupied, seat belt unfastened
    GPIO_PinState driver_warning = (ignition == GPIO_PIN_RESET && 
                                   driver_occupancy == GPIO_PIN_RESET && 
                                   driver_seatbelt == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState passenger_warning = (ignition == GPIO_PIN_RESET && 
                                      passenger_occupancy == GPIO_PIN_RESET && 
                                      passenger_seatbelt == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    update_warnings(driver_warning, passenger_warning);
    HAL_Delay(100);
  }
}

void SystemClock_Config(void) {
  SystemCoreClockUpdate();
}

void GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();


  GPIO_InitStruct.Pin = IGNITION_PIN | DRIVER_OCCUPANCY_PIN | DRIVER_SEATBELT_PIN | 
                        PASSENGER_OCCUPANCY_PIN | PASSENGER_SEATBELT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(INPUT_PORT, &GPIO_InitStruct);


  GPIO_InitStruct.Pin = DRIVER_WARNING_LED_PIN | PASSENGER_WARNING_LED_PIN | BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OUTPUT_PORT, &GPIO_InitStruct);
}


void update_warnings(GPIO_PinState driver_warning, GPIO_PinState passenger_warning) {
  static uint32_t last_toggle = 0;
  static GPIO_PinState led_state = GPIO_PIN_RESET;
  uint32_t current_time = HAL_GetTick();

  if (current_time - last_toggle >= 500) {
    led_state = (led_state == GPIO_PIN_RESET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    last_toggle = current_time;
  }


  HAL_GPIO_WritePin(OUTPUT_PORT, DRIVER_WARNING_LED_PIN, 
                    (driver_warning == GPIO_PIN_SET && led_state == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OUTPUT_PORT, PASSENGER_WARNING_LED_PIN, 
                    (passenger_warning == GPIO_PIN_SET && led_state == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(OUTPUT_PORT, BUZZER_PIN, 
                    (driver_warning == GPIO_PIN_SET || passenger_warning == GPIO_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_MspInit(void) {
  // Enable AFIO and PWR clocks
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}
