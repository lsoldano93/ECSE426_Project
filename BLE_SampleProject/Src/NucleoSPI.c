
///* Includes ------------------------------------------------------------------*/
//#include "NucleoSPI.h"

///* Private variables ---------------------------------------------------------*/
//__IO uint32_t  MasterTimeout = MASTER_FLAG_TIMEOUT;
//SPI_HandleTypeDef    SpiHandle;


///**
//  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
//  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
//  * @param  Data: Data to be transmitted.
//  * @retval None
//  */

//void SPI_SendData(SPI_HandleTypeDef *hspi, uint16_t Data){ 
//  /* Write in the DR register the data to be sent */
//  hspi->Instance->DR = Data;
//}


///**
//  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
//  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
//  * @retval The value of the received data.
//  */

//uint8_t SPI_ReceiveData(SPI_HandleTypeDef *hspi){
//  /* Return the data in the DR register */
//  return hspi->Instance->DR;
//}


///**
//  * @brief  Basic management of the timeout situation.
//  * @param  None.
//  * @retval None.
//  */
//uint32_t MASTER_TIMEOUT_UserCallback(void){
//  
//	printf("Discovery communication timed out \n");
//	
//	return 0;
//}


///**
//  * @brief  Sends a Byte through the SPI interface and return the Byte received
//  *         from the SPI bus.
//  * @param  Byte : Byte send.
//  * @retval The received byte value
//  */
//static uint8_t Master_SendByte(uint8_t byte){
//	
//  /* Loop while DR register in not empty */
//  MasterTimeout = MASTER_FLAG_TIMEOUT;
//  while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_TXE) == RESET)
//  {
//    if((MasterTimeout--) == 0) return MASTER_TIMEOUT_UserCallback();
//  }

//  /* Send a Byte through the SPI peripheral */
//  SPI_SendData(&SpiHandle,  byte);

//  /* Wait to receive a Byte */
//  MasterTimeout = MASTER_FLAG_TIMEOUT;
//  while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_RXNE) == RESET) {
//		//Master_SendByte(DUMMY_BYTE);   /// TODO: Is this necessary to generate clock? If not remove ***********
//    if((MasterTimeout--) == 0) {
//			return MASTER_TIMEOUT_UserCallback();
//		}
//  }

//  /* Return the Byte read from the SPI bus */ 
//  return SPI_ReceiveData(&SpiHandle);
//}


///**
//  * @brief  Writes one byte to the Discovery board
//  * @param  pBuffer : pointer to the buffer  containing the data to be written to Discovery.
//  * @param  VariableToWrite : Discovery's variable to be written to.
//  * @param  NumByteToWrite: Number of bytes to write.
//  * @retval None
//  */
//void Master_Write(uint8_t* pBuffer, uint8_t VariableToWrite, uint16_t NumByteToWrite){

//	HAL_StatusTypeDef errorCode;
//	
//  /* Set chip select Low at the start of the transmission */
//  DISCOVERY_CS_LOW();

////	errorCode = HAL_SPI_Transmit(&SpiHandleDiscovery, pBuffer, NumByteToWrite, MASTER_FLAG_TIMEOUT);
////	if(errorCode == HAL_OK) printf("HAL SPI Transmission succesful!\n");
////	else printf("Error in HAL SPI Transmit call!\n");
//	
//  /* Send the Address of the indexed register */
//  Master_SendByte(VariableToWrite);
//	
////  /* Send the data that will be written into the device (MSB First) */
//  while(NumByteToWrite >= 0x01) {
//    Master_SendByte(*pBuffer);
//    NumByteToWrite--;
//    pBuffer++;
//  }

//  /* Set chip select High at the end of the transmission */
//  DISCOVERY_CS_HIGH();
//	
//	//printf("Value of rBuffer: %x\n", rBuffer);
//	
//}


///**
//  * @brief  Reads a block of data from the Discovery board.
//  * @param  pBuffer : pointer to the buffer that receives the data read from Discovery.
//	* @param	VariableToRead: Discovery's variable to be read from.
//  * @param  NumByteToRead : number of bytes to read from the Discovery.
//  * @retval None
//  */
//void Discovery_Read(uint8_t* pBuffer, uint8_t VariableToRead, uint16_t NumByteToRead){
//  /* Set chip select Low at the start of the transmission */
//  DISCOVERY_CS_LOW();

//  /* Send the Address of the indexed register */
//  Master_SendByte(VariableToRead);

////  /* Receive the data that will be read from the device (MSB First) */
//  while(NumByteToRead > 0x00)
//  {
//    /* Send dummy byte (0x00) to generate the SPI clock to Discovery (Slave device) */
//    *pBuffer = Master_SendByte(DUMMY_BYTE);
//    NumByteToRead--;
//    pBuffer++;
//  }

//  /* Set chip select High at the end of the transmission */
//  DISCOVERY_CS_HIGH();
//	
//}

///**
//  * @brief  Configures Nucleo board for SPI communication with Discovery board
//  * @param  None
//  * @retval None
//  */
//void NucleoSPI_Config(void){

//	GPIO_InitTypeDef GPIO_InitStructure;

//  /* SPI configuration -------------------------------------------------------*/
//	/* Enable the SPI periph */
//  __HAL_RCC_SPI2_CLK_ENABLE();
//	
//  HAL_SPI_DeInit(&SpiHandle);
//  SpiHandle.Instance 							= SPI2;
//  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//  SpiHandle.Init.Direction 				= SPI_DIRECTION_2LINES; // set full duplex communication
//  SpiHandle.Init.CLKPhase 					= SPI_PHASE_2EDGE;
//  SpiHandle.Init.CLKPolarity 			= SPI_POLARITY_LOW;
//  SpiHandle.Init.CRCCalculation		= SPI_CRCCALCULATION_DISABLED;
//  SpiHandle.Init.CRCPolynomial 		= 7;
//  SpiHandle.Init.DataSize 					= SPI_DATASIZE_8BIT;
//  SpiHandle.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
//  SpiHandle.Init.NSS 							= SPI_NSS_SOFT;
//  SpiHandle.Init.TIMode 						= SPI_TIMODE_DISABLED;
//  SpiHandle.Init.Mode 							= SPI_MODE_MASTER;
//		
//	if (HAL_SPI_Init(&SpiHandle) != HAL_OK) {printf ("ERROR: Error in initialising SPIDiscovery \n");};
//  
//	__HAL_SPI_ENABLE(&SpiHandle);
//	
//}

