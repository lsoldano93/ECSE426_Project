
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_SPICommunication;   
SPI_HandleTypeDef NucleoSpiHandle;

osThreadDef(Thread_SPICommunication, osPriorityNormal, 1, NULL); 


/* Private Functions ---------------------------------------------------------*/

uint32_t  SPI_Timeout = SPI_Timeout_Flag;
/**  Initiates SPI Communication thread
   * @brief  Builds thread and starts it
   * @param  None
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_SPICommunication (void) {

  tid_Thread_SPICommunication = osThreadCreate(osThread(Thread_SPICommunication ), NULL); 
  if (!tid_Thread_SPICommunication){
		printf("Error starting SPI communication thread!");
		return(-1); 
	}
  return(0);
}


/**  
   * @brief  Runs SPI communication thread which communicates with Nucleo-64 via SPI
	 * @param  None
   * @retval None
   */
void Thread_SPICommunication (void const *argument){
	
	uint8_t returnValue;
	
	while(1){

		// Wait for signal from GPIO pin
		while(HAL_GPIO_ReadPin(NUCLEO_SPI_CS_GPIO_PORT, NUCLEO_SPI_CS_PIN) == GPIO_PIN_SET);
	
		returnValue = Slave_ReadByte();
		printf("Should be 0x80 = 164, returnValue = %d\n", returnValue);
		
		returnValue = Slave_ReadByte();
		printf("Should be 0x11 = 17, returnValue = %d\n", returnValue);
		
		// TODO: After GPIO Interrupt read, be ready to read in first value from Nucleo
		
		// TODO: If value from Nucleo indicates that master wants to write, prepare to read into LED state
		//       After reading LED state, gain access to shared state variable and write it
		
		// TODO: If value from Nucleo indicates that master wants to read, prepare to write specified value
		//       Gain access to shared variable and then send it to Nucleo

	}                                                       
}


/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @param  Data: Data to be transmitted.
  * @retval None
  */
void Slave_Spi_SendData(SPI_HandleTypeDef *hspi, uint16_t Data)
{ 
  /* Write in the DR register the data to be sent */
  hspi->Instance->DR = Data;
}


/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @retval The value of the received data.
  */
uint8_t Slave_Spi_ReceiveData(SPI_HandleTypeDef *hspi)
{
  /* Return the data in the DR register */
  return hspi->Instance->DR;
}


/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t Slave_SendByte(uint8_t byte) {
  /* Loop while DR register in not empty */
	SPI_Timeout = SPI_Timeout_Flag;
  while (__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_TXE) == RESET)
  {
    if((SPI_Timeout--) == 0) return 0;
  }

  /* Send a Byte through the SPI peripheral */
  Slave_Spi_SendData(&NucleoSpiHandle,  byte);

  /* Wait to receive a Byte */
  SPI_Timeout = SPI_Timeout_Flag;
  while (__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_RXNE) == RESET)
  {
    if((SPI_Timeout--) == 0) {
			return 0;
		}
  }

  /* Return the Byte read from the SPI bus */ 
  return Slave_Spi_ReceiveData(&NucleoSpiHandle);
}


static uint8_t Slave_ReadByte(void) {
	/* Wait to receive a Byte */
  SPI_Timeout = SPI_Timeout_Flag;
  while (__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_RXNE) == RESET) {
    if((SPI_Timeout--) == 0) {
			return 0;
		}
  }
	/* Return the Byte read from the SPI bus */ 
  return Slave_Spi_ReceiveData(&NucleoSpiHandle);
}


/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void SPICommunication_config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
  /* SPI configuration -------------------------------------------------------*/
	/* Enable the SPI periph */
  __SPI3_CLK_ENABLE();
	
  HAL_SPI_DeInit(&NucleoSpiHandle);
  NucleoSpiHandle.Instance 							  = SPI3;
  NucleoSpiHandle.Init.BaudRatePrescaler 	= SPI_BAUDRATEPRESCALER_4;
  NucleoSpiHandle.Init.Direction 					= SPI_DIRECTION_2LINES; // set full duplex communication
  NucleoSpiHandle.Init.CLKPhase 					= SPI_PHASE_1EDGE;
  NucleoSpiHandle.Init.CLKPolarity 				= SPI_POLARITY_LOW;
  NucleoSpiHandle.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLED;
  NucleoSpiHandle.Init.CRCPolynomial 			= 7;
  NucleoSpiHandle.Init.DataSize 					= SPI_DATASIZE_8BIT;
  NucleoSpiHandle.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
  NucleoSpiHandle.Init.NSS 								= SPI_NSS_SOFT;
  NucleoSpiHandle.Init.TIMode 						= SPI_TIMODE_DISABLED;
  NucleoSpiHandle.Init.Mode 							= SPI_MODE_SLAVE;
		
	if (HAL_SPI_Init(&NucleoSpiHandle) != HAL_OK) printf ("ERROR: Error in initialising SPI Nucleo \n");
  
	/* SPI3 Handle is for comm between discovery and nucleo */
	/* Enable SCK, MOSI, CS and MISO GPIO clocks */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;

	// PI3_MISO = PB4
	GPIO_InitStructure.Pin = NUCLEO_SPI_MISO_PIN;
	HAL_GPIO_Init(NUCLEO_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
	
	// SPI3_MOSI = PB5
	GPIO_InitStructure.Pin = NUCLEO_SPI_MOSI_PIN;
	HAL_GPIO_Init(NUCLEO_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
	
	// SPI3_SCK = PB3
	GPIO_InitStructure.Pin = NUCLEO_SPI_SCK_PIN;
	HAL_GPIO_Init(NUCLEO_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	
	// SPI3 CS = PA15
	GPIO_InitStructure.Pin   = NUCLEO_SPI_CS_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
	
	__HAL_SPI_ENABLE(&NucleoSpiHandle);
	
}
