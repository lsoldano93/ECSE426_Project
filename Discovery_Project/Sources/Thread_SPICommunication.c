
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"
#include "math.h"


/* Private Variables ---------------------------------------------------------*/

SPI_HandleTypeDef NucleoSpiHandle;

/* Private Functions ---------------------------------------------------------*/

/**
  * @brief This function handles SPI Communication Timeout.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *                the configuration information for SPI module.
  * @param  Flag: SPI flag to check
  * @param  Status: Flag status to check: RESET or set
  * @param  Timeout: Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef SPI_WaitOnFlagUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, FlagStatus Status, uint32_t Timeout)  
{
  int timeRemaining = Timeout;
	
	while(__HAL_SPI_GET_FLAG(hspi, Flag) == Status){
		timeRemaining--;
		if(timeRemaining == 0) return HAL_ERROR;
	}
	
  return HAL_OK;
}

void Slave_Write(float input){

	
	// Can only write decimal values between 0.0 & 0.99
	uint8_t tempDecimalValue = 0;
	uint8_t tempIntegerValue = 0;
	uint16_t messageValue = 0;

	// Calculate integer value
	tempIntegerValue = (int) input;
	
	// Calculate decimal value 
	tempDecimalValue = ((int)(input*(100.0)) - (tempIntegerValue*100.0));
	
	// Create bit stream where MS -bits are integer value and LS -bits are decimal value
	messageValue = ((uint16_t)tempIntegerValue << 8) | ((uint16_t)tempDecimalValue);
	printf("Temperature Message: %d\n", messageValue);
	
	// Write message to Data register for transfer
	NucleoSpiHandle.Instance->DR = messageValue;
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_TXE, RESET, SPI_TIMEOUT)!=HAL_OK) return; 
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_RXNE, RESET, SPI_TIMEOUT)!=HAL_OK) return;  
	
	// Go through proper wait protocol for transfer to complete
	messageValue = NucleoSpiHandle.Instance->DR;
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_BSY, !RESET, SPI_TIMEOUT)!=HAL_OK) return; 
	
	return; 
	
}
	

void Slave_Read(){
	
	uint8_t ledState, dcPrescaler;
	uint16_t messageValue;
	
	int i;
	
	// Wait for Nucleo to finish writing
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_RXNE, RESET, SPI_TIMEOUT)!=HAL_OK) return; 
	
	// Read message
	messageValue = NucleoSpiHandle.Instance->DR;
	NucleoSpiHandle.Instance->DR = 0x0000;
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_TXE, RESET, SPI_TIMEOUT)!=HAL_OK) return;  
	if(SPI_WaitOnFlagUntilTimeout(&NucleoSpiHandle, SPI_FLAG_BSY, !RESET, SPI_TIMEOUT)!=HAL_OK) return; 
	
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
void SPI2_ISR(){
	
	float temperature, pitch, roll;
	int doubleTap;
	uint8_t ledState;
	uint16_t returnValue;
	
	__disable_irq();
	__HAL_SPI_DISABLE_IT(&NucleoSpiHandle, SPI_FLAG_RXNE);
	
	// Receive information from master to begin communication
	returnValue = NucleoSpiHandle.Instance->DR;
	
	if(returnValue == COMMAND_TEMPERATURE){
		
		// Read in latest temperature values
		osMutexWait(temperatureMutex, (uint32_t) THREAD_TIMEOUT);
		temperature = temperatureValue;
		osMutexRelease(temperatureMutex);
		
		// Write temperature value
		Slave_Write(temperature);
		printf("Temperature: %f\n", temperature);
		
		HAL_GPIO_WritePin(TEMPERATURE_INTERRUPT_PORT, TEMPERATURE_INTERRUPT_PIN, GPIO_PIN_RESET);
		
	}
	else if(returnValue == COMMAND_PITCH){
	
		// Read in latest accelerometer values
		osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
		pitch = pitchValue;
		osMutexRelease(tiltAnglesMutex);
		
		// Write pitch value
		Slave_Write(pitch);
		printf("Pitch: %f\n", pitch);
		
		HAL_GPIO_WritePin(ACCELEROMETER_INTERRUPT_PORT, ACCELEROMETER_INTERRUPT_PIN, GPIO_PIN_RESET);
		
	}
	else if(returnValue == COMMAND_ROLL){
	
		// Read in latest accelerometer values
		osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
		roll = rollValue;
		osMutexRelease(tiltAnglesMutex);
		
		// Write pitch value
		Slave_Write(roll);
		printf("Roll: %f\n", roll);
		
		HAL_GPIO_WritePin(ACCELEROMETER_INTERRUPT_PORT, ACCELEROMETER_INTERRUPT_PIN, GPIO_PIN_RESET);
		
	}
	else if(returnValue == COMMAND_DTAP){
	
		// Read in latest accelerometer values
		osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
		doubleTap = DOUBLE_TAP_BOOLEAN;
		DOUBLE_TAP_BOOLEAN = 0;   // Clear flag
		osMutexRelease(tiltAnglesMutex);
		
		// Write pitch value
		Slave_Write(doubleTap);
		printf("Double Tap: %d\n", doubleTap);
		
		HAL_GPIO_WritePin(ACCELEROMETER_INTERRUPT_PORT, ACCELEROMETER_INTERRUPT_PIN, GPIO_PIN_RESET);
		
	}
	else if(returnValue == COMMAND_LEDSTATE){
		
		// Read LED State & Duty Cycle
		Slave_Read();
		
		HAL_GPIO_WritePin(LEDSTATE_INTERRUPT_PORT, LEDSTATE_INTERRUPT_PIN, GPIO_PIN_RESET);
		
	}
	else if(returnValue == 0);
	else NVIC_SystemReset();
	
		
	__HAL_SPI_ENABLE_IT(&NucleoSpiHandle, SPI_FLAG_RXNE);
	__enable_irq();
	
	
	return;
	                                                     
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
	NucleoSpiHandle.RxISR                     = SPI2_ISR;
	if (HAL_SPI_Init(&NucleoSpiHandle) != HAL_OK) {printf ("ERROR: Error in initialising SPI2 \n");};
  
	__HAL_SPI_ENABLE(&NucleoSpiHandle);
	__HAL_SPI_ENABLE_IT(&NucleoSpiHandle, SPI_IT_RXNE);
	
	HAL_NVIC_SetPriority(SPI2_IRQn, 0, 3);
	HAL_NVIC_EnableIRQ(SPI2_IRQn);
	
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
	
	// Discovery to Nucleo GPIO Interrupts
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	
	GPIO_InitStructure.Pin   = TEMPERATURE_INTERRUPT_PIN | ACCELEROMETER_INTERRUPT_PIN | LEDSTATE_INTERRUPT_PIN;
	HAL_GPIO_Init(TEMPERATURE_INTERRUPT_PORT, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(TEMPERATURE_INTERRUPT_PORT, TEMPERATURE_INTERRUPT_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(ACCELEROMETER_INTERRUPT_PORT, ACCELEROMETER_INTERRUPT_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LEDSTATE_INTERRUPT_PORT, LEDSTATE_INTERRUPT_PIN, GPIO_PIN_RESET);
	
}
