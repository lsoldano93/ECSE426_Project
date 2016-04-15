#ifndef DOPECOMS_H
#define DOPECOMS_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"


/* Private defines -----------------------------------------------------------*/

/* DISCOVERY Pins associated with Nucleo signal */
#define DISCOVERY_DATAi0_GPIO_PORT			GPIOD
#define DISCOVERY_DATAi0_PIN						GPIO_PIN_2  // GPIO_D2

#define DISCOVERY_DATAi1_GPIO_PORT			GPIOD
#define DISCOVERY_DATAi1_PIN						GPIO_PIN_3  // GPIO_D5

#define DISCOVERY_DATAi2_GPIO_PORT			GPIOD
#define DISCOVERY_DATAi2_PIN						GPIO_PIN_6  // GPIO_D6

#define DISCOVERY_DATAi3_GPIO_PORT			GPIOD
#define DISCOVERY_DATAi3_PIN						GPIO_PIN_7  // GPIO_D7

#define DISCOVERY_DATAo0_GPIO_PORT			GPIOD
#define DISCOVERY_DATAo0_PIN						GPIO_PIN_8  // GPIO_D8

#define DISCOVERY_DATAo1_GPIO_PORT			GPIOD
#define DISCOVERY_DATAo1_PIN						GPIO_PIN_9  // GPIO_D9

#define DISCOVERY_DATAo2_GPIO_PORT			GPIOD
#define DISCOVERY_DATAo2_PIN						GPIO_PIN_10  // GPIO_D10

#define DISCOVERY_DATAo3_GPIO_PORT			GPIOD
#define DISCOVERY_DATAo3_PIN						GPIO_PIN_11  // GPIO_D11

#define NUCLEO_TO_DISCOVERY_GPIO_PORT		GPIOC
#define NUCLEO_TO_DISCOVERY_PIN					GPIO_PIN_4  // GPIO_C4

#define DISCOVERY_TO_NUCLEO_GPIO_PORT		GPIOC
#define DISCOVERY_TO_NUCLEO_PIN					GPIO_PIN_5  // GPIO_C5

#define DISCOVERY_DATAio_CLOCK_ENABLE()		__GPIOD_CLK_ENABLE()
#define DISCOVERY_HS_CLOCK_ENABLE()				__GPIOC_CLK_ENABLE()

/* Private Variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**  Initiates SPI Communication thread
   * @brief  Builds thread and starts it
   * @param  None
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_DopeComs (void);

/**  
   * @brief  Runs SPI communication thread which communicates with Nucleo-64 via SPI
	 * @param  None
   * @retval None
   */
void Thread_DopeComs (void const *argument);

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t Slave_SendByte(uint8_t byte);

static uint8_t Slave_ReadByte(void);

/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void DopeComs_config(void);


#endif