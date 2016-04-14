#ifndef SPI_COMM_H
#define SPI_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"


/* Private defines -----------------------------------------------------------*/

#define SPI_TIMEOUT								((uint32_t) 100000)

#define COMMAND_TEMPERATURE				((uint16_t) 0x1111)
#define COMMAND_PITCH							((uint16_t) 0x6666)
#define COMMAND_ROLL							((uint16_t) 0x7777)
#define COMMAND_DTAP							((uint16_t) 0x8888)
#define COMMAND_LEDSTATE					((uint16_t) 0xffff)

/* DISCOVERY Pins associated with Nucleo signal */

#define DISCOVERY_SCK_PIN							GPIO_PIN_10  // GPIO_C10

#define DISCOVERY_MOSI_PIN						GPIO_PIN_12  // GPIO_C12

#define DISCOVERY_MISO_PIN						GPIO_PIN_11  // GPIO_C11

#define DISCOVERY_SPI_GPIO_PORT				GPIOC

#define DISCOVERY_SPI_CLOCK_ENABLE()						__GPIOC_CLK_ENABLE()
#define DISCOVERY_INTERRUPT_CLOCK_ENABLE()			__GPIOA_CLK_ENABLE()

/* Private Variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


void SPI2_ISR();

void SPICommunication_config(void);


#endif