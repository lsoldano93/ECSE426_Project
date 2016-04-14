
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"
#include "math.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_SPICommunication;   
SPI_HandleTypeDef NucleoSpiHandle;

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


void Slave_Write(float input){
	
	int i;
	
	// Can only write decimal values between 0.0 & 0.99
	int numIntegerDigits = 0;
	uint8_t tempDecimalValue = 0;
	uint8_t tempIntegerValue = 0;
	uint16_t messageValue = 0;
	
	// Find number of integer values
	for(i=0;i<3;i++){
		if(input/pow(10,i) > 0) numIntegerDigits = i+1;
		else break;
	}
	
	// Calculate integer value
	if(numIntegerDigits == 3){
		tempIntegerValue = ((int)(input/100))*100;
		tempIntegerValue += (((int)(input/10)) - ((int)(tempIntegerValue/10)))*10 ;
		tempIntegerValue += ((int) input) - tempIntegerValue;
	}
	else if(numIntegerDigits == 2){
		tempIntegerValue = ((int)(input/10));
		tempIntegerValue += ((int) input) - tempIntegerValue;
	}
	else if(numIntegerDigits == 1){
		tempIntegerValue = (int) input;
	}
	
	// Calculate decimal value 
	tempDecimalValue = (int)((input*(100.0)) - ((float)tempIntegerValue*100.0));
	
	// Create bit stream where MS -bits are integer value and LS -bits are decimal value
	messageValue = ((uint16_t)tempIntegerValue << 8) | tempDecimalValue;
	
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_RXNE) == RESET);
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_TXE) == RESET);
	
	// Write message to Data register for transfer
	NucleoSpiHandle.Instance->DR = messageValue;
	
	// Go through proper wait protocol for transfer to complete
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_TXE) == SET);
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_RXNE) == SET);
	messageValue = NucleoSpiHandle.Instance->DR;
	
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_BSY) == RESET);
	
	return; 
	
}
	

void Slave_Read(){
	
	uint8_t ledState, dcPrescaler;
	uint16_t messageValue;
	
	int i;
	
	// Wait for Nucleo to finish writing
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_RXNE) == SET);
	
	// Read message
	messageValue = NucleoSpiHandle.Instance->DR;
	
	while(__HAL_SPI_GET_FLAG(&NucleoSpiHandle, SPI_FLAG_BSY) != RESET);
	
	// Convert data into relevant outputs
	dcPrescaler = (uint8_t) messageValue >> 2;
	ledState = (uint8_t)(messageValue - (dcPrescaler << 2));
	
	// Update LED State values
	osMutexWait(ledStateMutex, (uint32_t) THREAD_TIMEOUT);
	LED_ROTATE_STATE = ledState;
	LED_DC_PRESCALER = dcPrescaler;
	osMutexRelease(ledStateMutex);
	
	return;
}


/**  
   * @brief  Runs SPI communication thread which communicates with Nucleo-64 via SPI
	 * @param  None
   * @retval None
   */
void Thread_SPICommunication (void const *argument){
	
	float temperature, pitch, roll;
	int doubleTap;
	uint8_t ledState;
	
	while(1){

		/* Wait for temperature sensor or accelerometer to update its value.
		   Upon signal, transmit temperature and accelerometer values before
			 receiving current LED state value from Nucleo                  */
		osSignalWait(THREAD_GREEN_LIGHT, THREAD_TIMEOUT);
	
		// Read in latest temperature and accelerometer values
		osMutexWait(temperatureMutex, (uint32_t) THREAD_TIMEOUT);
		temperature = temperatureValue;
		osMutexRelease(temperatureMutex);
		
		osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
		roll = rollValue;
		pitch = pitchValue;
		doubleTap = DOUBLE_TAP_BOOLEAN;
		DOUBLE_TAP_BOOLEAN = 0;   // Clear flag
		osMutexRelease(tiltAnglesMutex);
		
		
		HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_RESET);
		
		// Write temperature value
		Slave_Write(temperature);
		printf("Temperature: %f\n", temperature);
		// Set GPIO interrupt pin back to high
		
		// Write pitch value
		//Slave_Write(pitch);
	
		// Write roll value
		//Slave_Write(roll);
	
		// Write double tap boolean
		//Slave_Write_Boolean(doubleTap);
	
		// Read LED_State & Duty Cycle
		//Slave_Read();
		
		HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_SET);
		
		return;
	}                                                       
}

/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void SPICommunication_config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_SPI2_CLK_ENABLE();
	
	HAL_SPI_DeInit(&NucleoSpiHandle);
	NucleoSpiHandle.Instance									= SPI2;
	NucleoSpiHandle.Init.BaudRatePrescaler 		= SPI_BAUDRATEPRESCALER_4;
	NucleoSpiHandle.Init.Direction						= SPI_DIRECTION_2LINES;
	NucleoSpiHandle.Init.CLKPhase							= SPI_PHASE_1EDGE;
	NucleoSpiHandle.Init.CLKPolarity					= SPI_POLARITY_LOW;
	NucleoSpiHandle.Init.CRCCalculation				= SPI_CRCCALCULATION_DISABLED;
	NucleoSpiHandle.Init.CRCPolynomial 				= 7;
  NucleoSpiHandle.Init.DataSize 						= SPI_DATASIZE_16BIT;
  NucleoSpiHandle.Init.FirstBit 						= SPI_FIRSTBIT_MSB;
  NucleoSpiHandle.Init.NSS 									= SPI_NSS_SOFT;
  NucleoSpiHandle.Init.TIMode 							= SPI_TIMODE_DISABLED;
  NucleoSpiHandle.Init.Mode 								= SPI_MODE_SLAVE;
	if (HAL_SPI_Init(&NucleoSpiHandle) != HAL_OK) {printf ("ERROR: Error in initialising SPI2 \n");};
  
	__HAL_SPI_ENABLE(&NucleoSpiHandle);
	
	__SPI2_CLK_ENABLE();
	
	DISCOVERY_SPI_CLOCK_ENABLE();
	DISCOVERY_INTERRUPT_CLOCK_ENABLE();                                                 
	
	// SPI Pin Configurations
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
	
	// SCK, MOSI, MISO Pins
	GPIO_InitStructure.Pin   = DISCOVERY_SCK_PIN | DISCOVERY_MOSI_PIN | DISCOVERY_MISO_PIN;
	HAL_GPIO_Init(DISCOVERY_SPI_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery to Nucleo GPIO Interrupt (Out - Active Low)
	GPIO_InitStructure.Pin   = DISCOVERY_INTERRUPT_PIN;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_INTERRUPT_PORT, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_SET);
	
}
