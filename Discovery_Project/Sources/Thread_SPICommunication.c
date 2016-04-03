
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_SPICommunication;   
SPI_HandleTypeDef DiscoverySpiHandle;

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

		/* Wait for temperature sensor or accelerometer to update its value.
		   Upon signal, transmit temperature and accelerometer values before
			 receiving current LED state value from Nucleo                  */
		osSignalWait(THREAD_GREEN_LIGHT, THREAD_TIMEOUT);
	
		// Set GPIO interrupt pin low to communication with Nucleo that new data is available
		HAL_GPIO_WritePin(NUCLEO_SPI_INTERRUPT_PORT, NUCLEO_SPI_INTERRUPT_PIN, GPIO_PIN_RESET);
		
		// Wait for chip select line to go low so information can be read
		while(HAL_GPIO_ReadPin(NUCLEO_SPI_CS_GPIO_PORT, NUCLEO_SPI_CS_PIN) == GPIO_PIN_SET);
		Discovery_SPI_Read();
//		returnValue = Slave_ReadByte();
//		
//		// For the sake of debugging...
//		if(returnValue != 0) printf("Should be 0x80 = 164, returnValue = %d\n", returnValue);
//		
//		returnValue = Slave_ReadByte();
//		
//		// For the sake of debugging...
//		if(returnValue != 0) printf("Should be 0x11 = 17, returnValue = %d\n", returnValue);
		
		// TODO: After GPIO Interrupt read, be ready to read in first value from Nucleo
		
		// TODO: If value from Nucleo indicates that master wants to write, prepare to read into LED state
		//       After reading LED state, gain access to shared state variable and write it
		
		// TODO: If value from Nucleo indicates that master wants to read, prepare to write specified value
		//       Gain access to shared variable and then send it to Nucleo
		
		// Set GPIO interrupt pin back to high
		HAL_GPIO_WritePin(NUCLEO_SPI_INTERRUPT_PORT, NUCLEO_SPI_INTERRUPT_PIN, GPIO_PIN_SET);

	}                                                       
}


void Discovery_SPI_Read(void) {
	uint8_t buff[12];
	HAL_StatusTypeDef readStatus;
	
	printf("Begine Read\n");
	printf("bytes 1,2,3,4,5,6,: %d, %d, %d, %d, %d, %d \n", buff[0], buff[1], buff[2], buff[3],buff[4],buff[5]);
	printf("bytes 7,8,9,10,11,12: %d, %d, %d, %d, %d, %d \n", buff[6], buff[7], buff[8],buff[9], buff[10], buff[11] );
	printf("SPI READ NOW!\n");
	
	readStatus = HAL_SPI_Receive(&DiscoverySpiHandle, buff, 12, 4096);
	printf("After Read\n");
	printf("bytes 1,2,3,4,5,6,: %d, %d, %d, %d, %d, %d \n", buff[0], buff[1], buff[2], buff[3],buff[4],buff[5]);
	printf("bytes 7,8,9,10,11,12: %d, %d, %d, %d, %d, %d \n", buff[6], buff[7], buff[8],buff[9], buff[10], buff[11] );
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
	
  HAL_SPI_DeInit(&DiscoverySpiHandle);
  DiscoverySpiHandle.Instance 							  = SPI3;
  DiscoverySpiHandle.Init.BaudRatePrescaler 	= SPI_BAUDRATEPRESCALER_2;
  DiscoverySpiHandle.Init.Direction 					= SPI_DIRECTION_2LINES; // set full duplex communication
  DiscoverySpiHandle.Init.CLKPhase 					= SPI_PHASE_1EDGE;
  DiscoverySpiHandle.Init.CLKPolarity 				= SPI_POLARITY_LOW;
  DiscoverySpiHandle.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLED;
  DiscoverySpiHandle.Init.CRCPolynomial 			= 7;
  DiscoverySpiHandle.Init.DataSize 					= SPI_DATASIZE_8BIT;
  DiscoverySpiHandle.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
  DiscoverySpiHandle.Init.NSS 								= SPI_NSS_SOFT;
  DiscoverySpiHandle.Init.TIMode 						= SPI_TIMODE_DISABLED;
  DiscoverySpiHandle.Init.Mode 							= SPI_MODE_SLAVE;
		
	if (HAL_SPI_Init(&DiscoverySpiHandle) != HAL_OK) printf ("ERROR: Error in initialising SPI DiscoverySpiHandle \n");
  __HAL_SPI_ENABLE(&DiscoverySpiHandle);
	
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
	
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;			// TODO: Maybe this pin should be an input with no pull??
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF6_SPI3;
	
	// SPI3_SCK = PB3
	GPIO_InitStructure.Pin = NUCLEO_SPI_SCK_PIN;
	HAL_GPIO_Init(NUCLEO_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	
	// SPI3 CS = PA15  (Input - Active Low)
	GPIO_InitStructure.Pin   = NUCLEO_SPI_CS_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
	
	// Nucleo GPIO Interrupt (Out - Active Low)
	GPIO_InitStructure.Pin   = NUCLEO_SPI_INTERRUPT_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_SPI_INTERRUPT_PORT, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(NUCLEO_SPI_INTERRUPT_PORT, NUCLEO_SPI_INTERRUPT_PIN, GPIO_PIN_SET);
	
	
	
}
