#ifndef SPI_COMM_H
#define SPI_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"


/* Private defines -----------------------------------------------------------*/

#define SPI_TIMEOUT										((uint16_t) 4096)

/* DISCOVERY Pins associated with Nucleo signal */

#define DISCOVERY_SCK_PIN							GPIO_PIN_13  // GPIO_B13

#define DISCOVERY_MOSI_PIN						GPIO_PIN_15  // GPIO_B15

#define DISCOVERY_MISO_PIN						GPIO_PIN_2  // GPIO_B14

#define DISCOVERY_SPI_GPIO_PORT				GPIOB

#define DISCOVERY_INTERRUPT_PORT			GPIOA
#define DISCOVERY_INTERRUPT_PIN				GPIO_PIN_8	 // GPIO_A8

#define DISCOVERY_SPI_CLOCK_ENABLE()						__GPIOB_CLK_ENABLE()
#define DISCOVERY_INTERRUPT_CLOCK_ENABLE()			__GPIOA_CLK_ENABLE()

/* Private Variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**  Initiates SPI Communication thread
   * @brief  Builds thread and starts it
   * @param  None
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_SPICommunication (void);

/**  
   * @brief  Runs SPI communication thread which communicates with Nucleo-64 via SPI
	 * @param  None
   * @retval None
   */
void Thread_SPICommunication (void const *argument);

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
void SPICommunication_config(void);


#endif