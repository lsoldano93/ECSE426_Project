#ifndef NUCLEOSPI_H
#define NUCLEOSPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"


/* Private defines -----------------------------------------------------------*/

/* Nucleo Pins associated with Discovery signal */
#define NUCLEO_SCK_PIN						GPIO_PIN_13  // GPIO_C9

#define NUCLEO_MOSI_PIN						GPIO_PIN_15  // GPIO_C8

#define NUCLEO_MISO_PIN						GPIO_PIN_14  // GPIO_C6

#define NUCLEO_SPI_GPIO_PORT			GPIOB

#define NUCLEO_INTERRUPT_PORT							GPIOA
#define NUCLEO_INTERRUPT_PIN							GPIO_PIN_4	// GPIO_A4

#define NUCLEO_SPI_CLOCK_ENABLE()					__GPIOB_CLK_ENABLE()
#define NUCLEO_INTERRUPT_CLOCK_ENABLE()		__GPIOA_CLK_ENABLE()


/* Private function prototypes -----------------------------------------------*/

// float* = float[4] = {temperature, pitch, roll, double tap boolean}
// uint8_t = bits[7..2] = Duty cycle prescalar, bits[1..0] = LED state
void Master_Communication(uint8_t LED_STATE, float* returnArray);

void NucleoSPI_Config(void);


#endif