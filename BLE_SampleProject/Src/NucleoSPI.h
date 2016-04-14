#ifndef NUCLEOSPI_H
#define NUCLEOSPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"


/* Private defines -----------------------------------------------------------*/

#define COMMAND_TEMPERATURE				((uint16_t) 0x1111)
#define COMMAND_PITCH							((uint16_t) 0x6666)
#define COMMAND_ROLL							((uint16_t) 0x7777)
#define COMMAND_DTAP							((uint16_t) 0x8888)
#define COMMAND_LEDSTATE					((uint16_t) 0xffff)

/* Nucleo Pins associated with Discovery signal */
#define NUCLEO_SCK_PIN						GPIO_PIN_13  // GPIO_B13

#define NUCLEO_MOSI_PIN						GPIO_PIN_15  // GPIO_B15

#define NUCLEO_MISO_PIN						GPIO_PIN_14  // GPIO_B14

#define NUCLEO_SPI_GPIO_PORT			GPIOB

#define TEMPERATURE_INTERRUPT_PORT				GPIOA
#define TEMPERATURE_INTERRUPT_PIN					GPIO_PIN_4	// GPIO_A4

#define ACCELEROMETER_INTERRUPT_PORT			GPIOA
#define ACCELEROMETER_INTERRUPT_PIN				GPIO_PIN_2	// GPIO_A2

#define LEDSTATE_INTERRUPT_PORT						GPIOA
#define LEDSTATE_INTERRUPT_PIN						GPIO_PIN_3	// GPIO_A3

#define NUCLEO_SPI_CLOCK_ENABLE()								__GPIOB_CLK_ENABLE()
#define TEMPERATURE_INTERRUPT_CLOCK_ENABLE()		__GPIOA_CLK_ENABLE()
#define ACCELEROMETER_INTERRUPT_CLOCK_ENABLE()	__GPIOA_CLK_ENABLE()
#define LEDSTATE_INTERRUPT_CLOCK_ENABLE()				__GPIOA_CLK_ENABLE()


/* Private function prototypes -----------------------------------------------*/

void Master_Write(uint8_t VariableToWrite);

float Master_Read(uint16_t Instruction);

void NucleoSPI_Config(void);


#endif