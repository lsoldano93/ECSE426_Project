#ifndef SPI_COMM_H
#define SPI_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
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

/* Private Variables ---------------------------------------------------------*/

uint32_t  SPI_Timeout = SPI_Timeout_Flag;


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @param  Data: Data to be transmitted.
  * @retval None
  */
void SPI_SendData(SPI_HandleTypeDef *hspi, uint16_t Data);

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @retval The value of the received data.
  */
uint8_t SPI_ReceiveData(SPI_HandleTypeDef *hspi);

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t Slave_SendByte(uint8_t byte);

/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void SPI_Slave_Init(void);


#endif