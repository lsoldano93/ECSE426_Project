
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
	
	HAL_StatusTypeDef errorCode;
	
	uint8_t returnValue;
	
	while(1){

		/* Wait for temperature sensor or accelerometer to update its value.
		   Upon signal, transmit temperature and accelerometer values before
			 receiving current LED state value from Nucleo                  */
		osSignalWait(THREAD_GREEN_LIGHT, THREAD_TIMEOUT);
	
		// Set GPIO interrupt pin low to communication with Nucleo that new data is available
		HAL_GPIO_WritePin(DISCOVERY_SPI_INTERRUPT_PORT, DISCOVERY_SPI_INTERRUPT_PIN, GPIO_PIN_RESET);
		
		// Wait for chip select line to go low so information can be read
		//while(HAL_GPIO_ReadPin(DISCOVERY_SPI_CS_GPIO_PORT, DISCOVERY_SPI_CS_PIN) == GPIO_PIN_SET);
		
		// Receive byte from master
		errorCode = HAL_SPI_Receive(&NucleoSpiHandle, &returnValue, sizeof(uint8_t), SPI_Timeout_Flag);
		if(errorCode == HAL_OK) printf("Should be 0x11, returnValue = %x\n", returnValue);
		else if (errorCode == HAL_TIMEOUT) printf("HAL SPI Receive call timed out!\n");
		else if (errorCode == HAL_BUSY) printf("HAL SPI Handle is busy!\n");
		else printf("Error in HAL SPI Receive call!\n");
		
		// Send byte back to confirm
		errorCode = HAL_SPI_Transmit(&NucleoSpiHandle, &returnValue, sizeof(uint8_t), SPI_Timeout_Flag);
		if (errorCode == HAL_TIMEOUT) printf("HAL SPI Transmit call timed out!\n");
		else if (errorCode == HAL_BUSY) printf("HAL SPI (Transmit) Handle is busy!\n");
		else printf("Error in HAL SPI Transmit call!\n");
		
		
		// TODO: After GPIO Interrupt read, be ready to read in first value from Nucleo
		
		
		// TODO: If value from Nucleo indicates that master wants to write, prepare to read into LED state
		//       After reading LED state, gain access to shared state variable and write it
		
		
		// TODO: If value from Nucleo indicates that master wants to read, prepare to write specified value
		//       Gain access to shared variable and then send it to Nucleo
		
		
		// Set GPIO interrupt pin back to high
		HAL_GPIO_WritePin(DISCOVERY_SPI_INTERRUPT_PORT, DISCOVERY_SPI_INTERRUPT_PIN, GPIO_PIN_SET);

	}                                                       
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
  __HAL_RCC_SPI2_CLK_ENABLE();
	
  HAL_SPI_DeInit(&NucleoSpiHandle);
  NucleoSpiHandle.Instance 							  = SPI2;
  NucleoSpiHandle.Init.BaudRatePrescaler 	= SPI_BAUDRATEPRESCALER_256;
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
	
	/* Enable the SPI periph */
  __SPI2_CLK_ENABLE();
	
	/* SPI2 Handle is for comm between discovery and nucleo */
	/* Enable SCK, MOSI, CS and MISO GPIO clocks */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	// SPI2_MISO = PB4
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pin = DISCOVERY_SPI_MISO_PIN;
	HAL_GPIO_Init(DISCOVERY_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
	
	// SPI2_MOSI = PB5
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pin = DISCOVERY_SPI_MOSI_PIN;
	HAL_GPIO_Init(DISCOVERY_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
	
	// SPI2_SCK = PB3
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;			
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	GPIO_InitStructure.Pin = DISCOVERY_SPI_SCK_PIN;
	HAL_GPIO_Init(DISCOVERY_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
	
	// SPI2 CS = PA15  (Input - Active Low)
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Pin   = DISCOVERY_SPI_CS_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
	
	// Nucleo GPIO Interrupt (Out - Active Low)
	GPIO_InitStructure.Pin   = DISCOVERY_SPI_INTERRUPT_PIN;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_SPI_INTERRUPT_PORT, &GPIO_InitStructure);
	
	
	HAL_GPIO_WritePin(DISCOVERY_SPI_INTERRUPT_PORT, DISCOVERY_SPI_INTERRUPT_PIN, GPIO_PIN_SET);
	
	__HAL_SPI_ENABLE(&NucleoSpiHandle);
	
}
