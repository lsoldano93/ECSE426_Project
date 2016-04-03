
/* Includes ------------------------------------------------------------------*/
#include "NucleoSPI.h"

/* Private variables ---------------------------------------------------------*/
__IO uint32_t  DiscoveryTimeout = DISCOVERY_FLAG_TIMEOUT;
SPI_HandleTypeDef    SpiHandleDiscovery;



/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */


/**
  * @brief  Writes one byte to the Discovery board
  * @param  pBuffer : pointer to the buffer  containing the data to be written to Discovery.
  * @param  VariableToWrite : Discovery's variable to be written to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void Discovery_Write(void){
	
	uint8_t buff[12];
	buff[0] = 1;
	buff[1] = 2;
	buff[2] = 3;
	buff[3] = 4;
	buff[4] = 5;
	buff[5] = 6;
	buff[6] = 7;
	buff[7] = 8;
	buff[8] = 9;
	buff[9] = 10;
	buff[10] = 11;
	buff[11] = 12;

  /* Set chip select Low at the start of the transmission */
  DISCOVERY_CS_LOW();

  HAL_SPI_Transmit(&SpiHandleDiscovery, buff, 12, 4096);
				


  /* Set chip select High at the end of the transmission */
  DISCOVERY_CS_HIGH();
}


/**
  * @brief  Reads a block of data from the Discovery board.
  * @param  pBuffer : pointer to the buffer that receives the data read from Discovery.
	* @param	VariableToRead: Discovery's variable to be read from.
  * @param  NumByteToRead : number of bytes to read from the Discovery.
  * @retval None
  */
void Discovery_Read(uint8_t* pBuffer, uint8_t VariableToRead, uint16_t NumByteToRead){

  /* Set chip select Low at the start of the transmission */
  DISCOVERY_CS_LOW();

  /* Send the Address of the indexed register */
  //Discovery_SendByte(VariableToRead);

  /* Receive the data that will be read from the device (MSB First) */
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to Discovery (Slave device) */
   // *pBuffer = Discovery_SendByte(DUMMY_BYTE);
    NumByteToRead--;
    pBuffer++;
  }

  /* Set chip select High at the end of the transmission */
  DISCOVERY_CS_HIGH();
	
}


/**
  * @brief  Configures Nucleo board for SPI communication with Discovery board
  * @param  None
  * @retval None
  */
void NucleoSPI_Config(void){
	
  /* SPI configuration -------------------------------------------------------*/
	/* Enable the SPI periph */
	__HAL_RCC_SPI2_CLK_ENABLE();

  HAL_SPI_DeInit(&SpiHandleDiscovery);
  SpiHandleDiscovery.Instance 							= SPI2;
  SpiHandleDiscovery.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  SpiHandleDiscovery.Init.Direction 				= SPI_DIRECTION_2LINES; // set full duplex communication
  SpiHandleDiscovery.Init.CLKPhase 					= SPI_PHASE_1EDGE;
  SpiHandleDiscovery.Init.CLKPolarity 			= SPI_POLARITY_LOW;
  SpiHandleDiscovery.Init.CRCCalculation		= SPI_CRCCALCULATION_DISABLED;
  SpiHandleDiscovery.Init.CRCPolynomial 		= 7;
  SpiHandleDiscovery.Init.DataSize 					= SPI_DATASIZE_8BIT;
  SpiHandleDiscovery.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
  SpiHandleDiscovery.Init.NSS 							= SPI_NSS_SOFT;
  SpiHandleDiscovery.Init.TIMode 						= SPI_TIMODE_DISABLED;
  SpiHandleDiscovery.Init.Mode 							= SPI_MODE_MASTER;
		
	if (HAL_SPI_Init(&SpiHandleDiscovery) != HAL_OK) {printf ("ERROR: Error in initialising SPIDiscovery \n");};
  
	__HAL_SPI_ENABLE(&SpiHandleDiscovery);
	
}

