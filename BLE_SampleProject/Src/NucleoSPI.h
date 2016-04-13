#ifndef NUCLEOSPI_H
#define NUCLEOSPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"


/* Private defines -----------------------------------------------------------*/

/* DISCOVERY Pins associated with Nucleo signal */
#define NUCLEO_DATAi0_GPIO_PORT			GPIOC
#define NUCLEO_DATAi0_PIN						GPIO_PIN_9  // GPIO_C9

#define NUCLEO_DATAi1_GPIO_PORT			GPIOC
#define NUCLEO_DATAi1_PIN						GPIO_PIN_8  // GPIO_C8

#define NUCLEO_DATAi2_GPIO_PORT			GPIOC
#define NUCLEO_DATAi2_PIN						GPIO_PIN_6  // GPIO_C6

#define NUCLEO_DATAi3_GPIO_PORT			GPIOC
#define NUCLEO_DATAi3_PIN						GPIO_PIN_5  // GPIO_C5

#define NUCLEO_DATAo0_GPIO_PORT			GPIOB
#define NUCLEO_DATAo0_PIN						GPIO_PIN_2  // GPIO_B2

#define NUCLEO_DATAo1_GPIO_PORT			GPIOB
#define NUCLEO_DATAo1_PIN						GPIO_PIN_15  // GPIO_B15

#define NUCLEO_DATAo2_GPIO_PORT			GPIOB
#define NUCLEO_DATAo2_PIN						GPIO_PIN_14  // GPIO_B14

#define NUCLEO_DATAo3_GPIO_PORT			GPIOB
#define NUCLEO_DATAo3_PIN						GPIO_PIN_13  // GPIO_B13

#define NUCLEO_TO_DISCOVERY_GPIO_PORT			GPIOA
#define NUCLEO_TO_DISCOVERY_PIN						GPIO_PIN_10  // GPIO_A10

#define DISCOVERY_TO_NUCLEO_GPIO_PORT			GPIOA
#define DISCOVERY_TO_NUCLEO_PIN						GPIO_PIN_2  // GPIO_A2

#define NUCLEO_INTERRUPT_PORT							GPIOA
#define NUCLEO_INTERRUPT_PIN							GPIO_PIN_4	// GPIO_A4

#define NUCLEO_DATAo_CLOCK_ENABLE()		__GPIOB_CLK_ENABLE()
#define NUCLEO_DATAi_CLOCK_ENABLE()		__GPIOC_CLK_ENABLE()
#define NUCLEO_HSI_CLOCK_ENABLE()		__GPIOA_CLK_ENABLE()


/* Private function prototypes -----------------------------------------------*/

// float* = float[4] = {temperature, pitch, roll, double tap boolean}
// uint8_t = bits[7..2] = Duty cycle prescalar, bits[1..0] = LED state
void Master_Communication(uint8_t LED_STATE, float* returnArray);

void NucleoSPI_Config(void);


#endif