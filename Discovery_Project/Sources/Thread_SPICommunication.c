
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


void Read_DataLines(int* inputArray){

	// inputArray[0] = MSB, read from Datai3 {Datai3, Datai2, Datai1, Datai0} = {inputArray[0],inputArray[1],inputArray[2],inputArray[3]}
	if(HAL_GPIO_ReadPin(DISCOVERY_DATAi3_GPIO_PORT, DISCOVERY_DATAi3_PIN) == GPIO_PIN_SET) inputArray[0] = 1;
	else inputArray[0] = 0;
	
	if(HAL_GPIO_ReadPin(DISCOVERY_DATAi2_GPIO_PORT, DISCOVERY_DATAi2_PIN) == GPIO_PIN_SET) inputArray[1] = 1;
	else inputArray[1] = 0;
	
	if(HAL_GPIO_ReadPin(DISCOVERY_DATAi1_GPIO_PORT, DISCOVERY_DATAi1_PIN) == GPIO_PIN_SET) inputArray[2] = 1;
	else inputArray[2] = 0;
	
	if(HAL_GPIO_ReadPin(DISCOVERY_DATAi0_GPIO_PORT, DISCOVERY_DATAi0_PIN) == GPIO_PIN_SET) inputArray[3] = 1;
	else inputArray[3] = 0;
	
	return;
}
	

void Set_DataLines(int* inputArray){
	
	// inputArray[0] = MSB, write to Datao3 {Datao3, Datao2, Datao1, Datao0} = {inputArray[0],inputArray[1],inputArray[2],inputArray[3]}
	if(inputArray[0] == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo3_GPIO_PORT, DISCOVERY_DATAo3_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo3_GPIO_PORT, DISCOVERY_DATAo3_PIN, GPIO_PIN_RESET);
	
	if(inputArray[1] == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo2_GPIO_PORT, DISCOVERY_DATAo2_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo2_GPIO_PORT, DISCOVERY_DATAo2_PIN, GPIO_PIN_RESET);
	
	if(inputArray[2] == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo1_GPIO_PORT, DISCOVERY_DATAo1_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo1_GPIO_PORT, DISCOVERY_DATAo1_PIN, GPIO_PIN_RESET);
	
	if(inputArray[3] == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_RESET);
	
	return;
}

void Reset_DataLines(){
	
	HAL_GPIO_WritePin(DISCOVERY_DATAo3_GPIO_PORT, DISCOVERY_DATAo3_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISCOVERY_DATAo2_GPIO_PORT, DISCOVERY_DATAo2_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISCOVERY_DATAo1_GPIO_PORT, DISCOVERY_DATAo1_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_RESET);
	
	return;
}

void Slave_Write(float input){
	
	int i;
	
	// Can only write decimal values between 0.0 & 0.99
	int numIntegerDigits = 0;
	int tempDecimalValue = 0;
	int tempIntegerValue = 0;
	
	int integerValue[8];
	int decimalValue[8];
	int tempArray[4];
	
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
	
	// Convert values to bit streams
	for (i= 0;i<8;i++){ 
		integerValue[7-i] = tempIntegerValue & (1 << i) ? 1 : 0;
		decimalValue[7-i] = tempDecimalValue & (1 << i) ? 1 : 0;
	}
	
	
	// Write first four bits of integer value (MSB first)
	for(i=0;i<4;i++)integerValue[i]=tempArray[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	// Write next four bits of integer value (MSB first)
	for(i=0;i<4;i++)integerValue[i+4]=tempArray[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	
	// Write first four bits of decimal value (MSB first)
	for(i=0;i<4;i++)decimalValue[i]=tempArray[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	// Write next four bits of decimal value (MSB first)
	for(i=0;i<4;i++)decimalValue[i+4]=tempArray[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	Reset_DataLines();
	
	return; 
	
}
	

void Slave_Write_Boolean(int input){
	
	// Check input (pin set = 1/true, pin reset = 0/false)
	if(input == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_RESET);
	
	// Signal to Nucleo that pin has been written
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	// Reset to proper pin order
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);

	// Reset output pins
	Reset_DataLines();
	
	return;
}
	

void Slave_Read(){
	
	uint8_t ledState, dcPrescaler;
	
	int i;
	
	int returnArray[8];
	int tempArray[4];
	
	// Wait for Nucleo to finish writing
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	// Read first four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)returnArray[i]=tempArray[i];
	
	// Signal to Nucleo that pins have been read
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish writing
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	// Read next four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)returnArray[i+4]=tempArray[i];
	
	// Signal to Nucleo that pins have been read
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Convert data into relevant outputs
	ledState = returnArray[7] + returnArray[6]*2;
	for(i=0;i<6;i++) dcPrescaler += ((uint8_t)pow(2,i)) * returnArray[5-i];
	
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
		
		// Set GPIO interrupt pin low to communication with Nucleo that new data is available
		HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_RESET);
		
		// Write temperature value
		Slave_Write(temperature);
		printf("Temperature: %f\n", temperature);
		// Set GPIO interrupt pin back to high
		HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_SET);
		
		// Write pitch value
		//Slave_Write(pitch);
	
		// Write roll value
		//Slave_Write(roll);
	
		// Write double tap boolean
		//Slave_Write_Boolean(doubleTap);
	
		// Read LED_State & Duty Cycle
		//Slave_Read();
		
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
	
	DISCOVERY_DATAio_CLOCK_ENABLE();
	DISCOVERY_HSI_CLOCK_ENABLE();                                                 
	
	// Discovery Output Pin 0  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAo0_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAo0_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Output Pin 1  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAo1_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAo1_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Output Pin 2  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAo2_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAo2_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Output Pin 3  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAo3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAo3_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery to Nucleo  (Output - Active Low)
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Pin   = DISCOVERY_TO_NUCLEO_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_TO_NUCLEO_GPIO_PORT, &GPIO_InitStructure);
	
	Reset_DataLines();
	
	/* Instantiate HS pin to high for active low state */
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Discovery Input Pin 0  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAi0_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAi0_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Input Pin 1  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAi1_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAi1_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Input Pin 2  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAi2_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAi2_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery Input Pin 3  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAi3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAi3_GPIO_PORT, &GPIO_InitStructure);
	
	// Nucleo to Discovery Handshake  (Input - Active Low)
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Pin   = NUCLEO_TO_DISCOVERY_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_TO_DISCOVERY_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery to Nucleo GPIO Interrupt (Out - Active Low)
	GPIO_InitStructure.Pin   = DISCOVERY_INTERRUPT_PIN;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_INTERRUPT_PORT, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(DISCOVERY_INTERRUPT_PORT, DISCOVERY_INTERRUPT_PIN, GPIO_PIN_SET);
	
}
