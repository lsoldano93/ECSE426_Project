
/* Includes ------------------------------------------------------------------*/
#include "NucleoSPI.h"

/* Private variables ---------------------------------------------------------*/
__IO uint32_t  MasterTimeout = MASTER_FLAG_TIMEOUT;
SPI_HandleTypeDef    SpiHandleDiscovery;


/**
  * @brief  Writes one byte to the Discovery board
  * @param  pBuffer : pointer to the buffer  containing the data to be written to Discovery.
  * @param  VariableToWrite : Discovery's variable to be written to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void Master_Write(uint8_t* pBuffer, uint8_t VariableToWrite, uint16_t NumByteToWrite){

	uint8_t rBuffer;
	HAL_StatusTypeDef errorCode;

	
  /* Set chip select Low at the start of the transmission */
  DISCOVERY_CS_LOW();

	errorCode = HAL_SPI_TransmitReceive(&SpiHandleDiscovery, pBuffer, &rBuffer, NumByteToWrite, MASTER_FLAG_TIMEOUT);
	if(errorCode == HAL_OK) printf("HAL SPI Transmission succesful!\n");
  else printf("Error in HAL SPI Transmit call!\n");

  /* Set chip select High at the end of the transmission */
  DISCOVERY_CS_HIGH();
	
	printf("Value of rBuffer: %x\n", rBuffer);
	
}


/**
  * @brief  Reads a block of data from the Discovery board.
  * @param  pBuffer : pointer to the buffer that receives the data read from Discovery.
	* @param	VariableToRead: Discovery's variable to be read from.
  * @param  NumByteToRead : number of bytes to read from the Discovery.
  * @retval None
  */
void Master_Read(uint8_t* pBuffer, uint8_t VariableToRead, uint16_t NumByteToRead){

  /* Set chip select Low at the start of the transmission */
  DISCOVERY_CS_LOW();



  /* Set chip select High at the end of the transmission */
  DISCOVERY_CS_HIGH();
	
}

/**
  * @brief  Configures Nucleo board for SPI communication with Discovery board
  * @param  None
  * @retval None
  */
void NucleoSPI_Config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;

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

