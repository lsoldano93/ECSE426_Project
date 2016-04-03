#ifndef SPI_COMM_H
#define SPI_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"


/* Private defines -----------------------------------------------------------*/

/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                 			((uint8_t)0x00)

/* Command signals for Discovery to indicate variable of interest */
#define COMMAND_READ_TEMPERATURE				((uint8_t)0x20)

#define COMMAND_READ_ACCELEROMETER_X		((uint8_t)0x40)
#define COMMAND_READ_ACCELEROMETER_Y		((uint8_t)0x50)
#define COMMAND_READ_ACCELEROMETER_Z		((uint8_t)0x60)

#define COMMAND_WRITE_LED_PATTERN				((uint8_t)0x80)

#define SPI_Timeout_Flag 4096

/* Pins associated with Nucleo signal */
#define NUCLEO_SPI_SCK_GPIO_PORT		GPIOC
#define NUCLEO_SPI_SCK_PIN					GPIO_PIN_10  // GPIO_C10

#define NUCLEO_SPI_MISO_GPIO_PORT		GPIOC
#define NUCLEO_SPI_MISO_PIN					GPIO_PIN_11  // GPIO_C11

#define NUCLEO_SPI_MOSI_GPIO_PORT		GPIOC
#define NUCLEO_SPI_MOSI_PIN					GPIO_PIN_12  // GPIO_C12

#define NUCLEO_SPI_CS_GPIO_PORT			GPIOA
#define NUCLEO_SPI_CS_PIN						GPIO_PIN_15  // GPIO_A15

#define NUCLEO_SPI_INTERRUPT_PORT		GPIOA
#define NUCLEO_SPI_INTERRUPT_PIN		GPIO_PIN_8	 // GPIO_A8

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

void Discovery_SPI_Read(void);

/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void SPICommunication_config(void);


#endif