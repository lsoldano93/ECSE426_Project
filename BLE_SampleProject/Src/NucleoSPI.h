#ifndef NUCLEOSPI_H
#define NUCLEOSPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
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

/* Values for Discovery SPI communications */
#define DISCOVERY_FLAG_TIMEOUT      	  ((uint32_t)0x1000)

/* Refer to page 33 of the following document
   http://www.st.com/web/en/resource/technical/document/user_manual/DM00105823.pdf */
#define DISCOVERY_SPI_SCK_GPIO_PORT			GPIOB
#define DISCOVERY_SPI_SCK_PIN						GPIO_PIN_13  // GPIO_B13

#define DISCOVERY_SPI_MOSI_GPIO_PORT		GPIOB
#define DISCOVERY_SPI_MOSI_PIN					GPIO_PIN_15  // GPIO_B15

#define DISCOVERY_SPI_MISO_GPIO_PORT		GPIOB
#define DISCOVERY_SPI_MISO_PIN					GPIO_PIN_14  // GPIO_B14

#define DISCOVERY_SPI_CS_GPIO_PORT			GPIOB
#define DISCOVERY_SPI_CS_PIN						GPIO_PIN_6  // GPIO_B6


/* Private macros ------------------------------------------------------------*/

/* Macros for Discovery SPI communications */
#define DISCOVERY_CS_LOW()       				HAL_GPIO_WritePin(DISCOVERY_SPI_CS_GPIO_PORT, DISCOVERY_SPI_CS_PIN, GPIO_PIN_RESET)
#define DISCOVERY_CS_HIGH()     		  	HAL_GPIO_WritePin(DISCOVERY_SPI_CS_GPIO_PORT, DISCOVERY_SPI_CS_PIN, GPIO_PIN_SET)


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
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t DISCOVERY_TIMEOUT_UserCallback(void);

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t Discovery_SendByte(uint8_t byte);

/**
  * @brief  Writes one byte to the Discovery board
  * @param  pBuffer : pointer to the buffer  containing the data to be written to Discovery.
  * @param  VariableToWrite : Discovery's variable to be written to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void Discovery_Write(uint8_t* pBuffer, uint8_t VariableToWrite, uint16_t NumByteToWrite);

/**
  * @brief  Reads a block of data from the Discovery board.
  * @param  pBuffer : pointer to the buffer that receives the data read from Discovery.
	* @param	VariableToRead: Discovery's variable to be read from.
  * @param  NumByteToRead : number of bytes to read from the Discovery.
  * @retval None
  */
void Discovery_Read(uint8_t* pBuffer, uint8_t VariableToRead, uint16_t NumByteToRead);

/**
  * @brief  Configures Nucleo board for SPI communication with Discovery board
  * @param  None
  * @retval None
  */
void NucleoSPI_Config(void);


#endif