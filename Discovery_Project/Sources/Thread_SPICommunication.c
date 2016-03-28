
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_SPICommunication;   
SPI_HandleTypeDef SpiHandle;

osThreadDef(Thread_SPICommunication, osPriorityNormal, 1, NULL); 


/* Private Functions ---------------------------------------------------------*/

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
	
	while(1){
		
		// TODO: Have this Thread read be called via a GPIO pin interrupt that will be set by Nucleo
		
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
void SPI_SendData(SPI_HandleTypeDef *hspi, uint16_t Data)
{ 
  /* Write in the DR register the data to be sent */
  hspi->Instance->DR = Data;
}


/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @retval The value of the received data.
  */
uint8_t SPI_ReceiveData(SPI_HandleTypeDef *hspi)
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
  while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_TXE) == RESET)
  {
    if((SPI_Timeout--) == 0) return 0;
  }

  /* Send a Byte through the SPI peripheral */
  SPI_SendData(&SpiHandle,  byte);

  /* Wait to receive a Byte */
  SPI_Timeout = SPI_Timeout_Flag;
  while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_RXNE) == RESET)
  {
    if((SPI_Timeout--) == 0) {
			return 0;
		}
  }

  /* Return the Byte read from the SPI bus */ 
  return SPI_ReceiveData(&SpiHandle);
}


/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void SPICommunication_config(void){
	
	/* SPI3 uses port b */
	GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SCK, MOSI and MISO GPIO clocks */
  __GPIOB_CLK_ENABLE();

  /* Enable CS, INT1, INT2  GPIO clock */
  __GPIOE_CLK_ENABLE();
 
  GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;

  // SPI3_SCK = PB3,  PI3_MISO = PB4, SPI3_MOSI = PB5
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 |GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
	/* Enable the SPI periph */
  __SPI3_CLK_ENABLE();
	
  HAL_SPI_DeInit(&SpiHandle);
  SpiHandle.Instance 							  = SPI3;
  SpiHandle.Init.BaudRatePrescaler 	= SPI_BAUDRATEPRESCALER_4;
  SpiHandle.Init.Direction 					= SPI_DIRECTION_2LINES; // set full duplex communication
  SpiHandle.Init.CLKPhase 					= SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity 				= SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial 			= 7;
  SpiHandle.Init.DataSize 					= SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS 								= SPI_NSS_SOFT;
  SpiHandle.Init.TIMode 						= SPI_TIMODE_DISABLED;
  SpiHandle.Init.Mode 							= SPI_MODE_SLAVE;
		
	if (HAL_SPI_Init(&SpiHandle) != HAL_OK) {printf ("ERROR: Error in initialising SPI1 \n");};
  
	__HAL_SPI_ENABLE(&SpiHandle);
	
}
