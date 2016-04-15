
/* Includes ------------------------------------------------------------------*/
#include "DopeComs.h"
#include "math.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_DopeComs;   

osThreadDef(Thread_DopeComs, osPriorityNormal, 1, NULL); 

// TODO: Remove
uint8_t debug = 0;

/* Private Functions ---------------------------------------------------------*/

/**  Initiates SPI Communication thread
   * @brief  Builds thread and starts it
   * @param  None
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_DopeComs(void) {

  tid_Thread_DopeComs = osThreadCreate(osThread(Thread_DopeComs), NULL); 
  if (!tid_Thread_DopeComs){
		printf("Error starting DopeComs thread!");
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
	
	//printf("%i,%i,%i,%i\n", inputArray[0],inputArray[1],inputArray[2],inputArray[3]);
	
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
	//int test[] = {1,0,1,1};
	
	// Find number of integer values
	for(i=0;i<3;i++){
		if(input/pow(10,i) > 0) numIntegerDigits = i+1;
		else break;
	}
	
	// Calculate integer value
	tempIntegerValue = (int) input;
	
	// Calculate decimal value
	tempDecimalValue = (int)((input*(100.0)) - ((float)tempIntegerValue*100.0));
	
	// Convert values to bit streams
	for (i= 0;i<8;i++){ 
		integerValue[7-i] = tempIntegerValue & (1 << i) ? 1 : 0;
		decimalValue[7-i] = tempDecimalValue & (1 << i) ? 1 : 0;
	}
	
	
	// Write first four bits of integer value (MSB first)
	for(i=0;i<4;i++)tempArray[i]=integerValue[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	// Write next four bits of integer value (MSB first)
	for(i=0;i<4;i++)tempArray[i]=integerValue[i+4];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	
	// Write first four bits of decimal value (MSB first)
	for(i=0;i<4;i++)tempArray[i]=decimalValue[i];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	// Write next four bits of decimal value (MSB first)
	for(i=0;i<4;i++)tempArray[i]=decimalValue[i+4];
	Set_DataLines(tempArray);
	
	// Signal to Nucleo that pins are ready
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	Reset_DataLines();
	
	return; 
	
}
	

void Slave_Write_Boolean(int input){
	
	// Check input (pin set = 1/true, pin reset = 0/false)
	if(input == 1) HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_RESET);
	else HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_SET);
	
	// Signal to Nucleo that pin has been written
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish reading
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	// Reset to proper pin order
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);

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
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
	
	// Read first four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)returnArray[i]=tempArray[i];
	
	// Signal to Nucleo that pins have been read
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
	
	// Wait for Nucleo to finish writing
	while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);
	
	// Read next four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)returnArray[i+4]=tempArray[i];
	
	// Signal to Nucleo that pins have been read
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Convert data into relevant outputs
	ledState = returnArray[7] + returnArray[6]*2;
	for(i=0;i<6;i++) dcPrescaler += ((uint8_t)pow(2,i)) * returnArray[5-i];
	
	//printf("Led State: %d, Dc Prescaler: %d\n", ledState, dcPrescaler);
	
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
void Thread_DopeComs(void const *argument){
	
	float temperature, pitch, roll;
	uint8_t ledState;
	
	while(1){

		/* Wait for temperature sensor or accelerometer to update its value.
		   Upon signal, transmit temperature and accelerometer values before
			 receiving current LED state value from Nucleo                  */
		osSignalWait(THREAD_GREEN_LIGHT, THREAD_TIMEOUT);
	
//		if(debug == 1){
//			HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET) printf("Nucleo to Discovery HS reading high\n");
//			
//			HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi0_GPIO_PORT, DISCOVERY_DATAi0_PIN) == GPIO_PIN_SET) printf("Input zero is reading high\n");
//			
//			HAL_GPIO_WritePin(DISCOVERY_DATAo1_GPIO_PORT, DISCOVERY_DATAo1_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi1_GPIO_PORT, DISCOVERY_DATAi1_PIN) == GPIO_PIN_SET) printf("Input one is reading high\n");
//			
//			HAL_GPIO_WritePin(DISCOVERY_DATAo2_GPIO_PORT, DISCOVERY_DATAo2_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi2_GPIO_PORT, DISCOVERY_DATAi2_PIN) == GPIO_PIN_SET) printf("Input two is reading high\n");
//			
//			HAL_GPIO_WritePin(DISCOVERY_DATAo3_GPIO_PORT, DISCOVERY_DATAo3_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi3_GPIO_PORT, DISCOVERY_DATAi3_PIN) == GPIO_PIN_SET) printf("Input three is reading high\n");
//			
//			Reset_DataLines();
//			debug = 0;
//		}
//		else if(debug == 2){
//			
//			HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
//			Reset_DataLines();
//			
//			if(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET) printf("Nucleo to Discovery HS reading high\n");
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi0_GPIO_PORT, DISCOVERY_DATAi0_PIN) == GPIO_PIN_SET) printf("Input zero is reading high\n");
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi1_GPIO_PORT, DISCOVERY_DATAi1_PIN) == GPIO_PIN_SET) printf("Input one is reading high\n");
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi2_GPIO_PORT, DISCOVERY_DATAi2_PIN) == GPIO_PIN_SET) printf("Input two is reading high\n");
//			if(HAL_GPIO_ReadPin(DISCOVERY_DATAi3_GPIO_PORT, DISCOVERY_DATAi3_PIN) == GPIO_PIN_SET) printf("Input three is reading high\n");
//			
////			HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(DISCOVERY_DATAo0_GPIO_PORT, DISCOVERY_DATAo0_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(DISCOVERY_DATAo1_GPIO_PORT, DISCOVERY_DATAo1_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(DISCOVERY_DATAo2_GPIO_PORT, DISCOVERY_DATAo2_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(DISCOVERY_DATAo3_GPIO_PORT, DISCOVERY_DATAo3_PIN, GPIO_PIN_SET);
//			
//			Reset_DataLines();
//			debug = 0;
//			
//		}
		
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
		
		// Set GPIO interrupt pin high to communicate with Nucleo that new data is available
		HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_SET);
		
		// Adjust communications back to active high
		while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_RESET);
		HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT,DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
		while(HAL_GPIO_ReadPin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN) == GPIO_PIN_SET);

//		temperature = 25.71;
//		pitch = 167.52;
//		roll = 1.24;
//		doubleTap = !doubleTap;
		
		// Write temperature value
		Slave_Write(temperature);
		//printf("Temperature: %f\n", temperature);
		
		// Write pitch value
		Slave_Write(pitch);
		//printf("Pitch: %f\n", pitch);
	
		// Write roll value
		Slave_Write(roll);
		//printf("Roll: %f\n", roll);
	
		// Write double tap boolean
		Slave_Write_Boolean(doubleTap);
	
		// Read LED_State & Duty Cycle
		Slave_Read();
		//printf("-----------\n");
		
		HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
		Reset_DataLines();
		
	}                                                       
}

/**
  * @brief  Initialize SPI handle for slave device (Discovery board)
  * @param  None
  * @retval None
  */
void DopeComs_config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	DISCOVERY_DATAio_CLOCK_ENABLE();
	DISCOVERY_HS_CLOCK_ENABLE();                                                 
	
	// Discovery Output Pins  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAo0_PIN | DISCOVERY_DATAo1_PIN | DISCOVERY_DATAo2_PIN | DISCOVERY_DATAo3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAo0_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery to Nucleo  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = DISCOVERY_TO_NUCLEO_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_TO_NUCLEO_GPIO_PORT, &GPIO_InitStructure);
	
	Reset_DataLines();
	
	/* Instantiate HS pin to low for active high state */
	HAL_GPIO_WritePin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN, GPIO_PIN_RESET);
	
	// Discovery Input Pins  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Pin   = DISCOVERY_DATAi0_PIN | DISCOVERY_DATAi1_PIN | DISCOVERY_DATAi2_PIN | DISCOVERY_DATAi3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_DATAi0_GPIO_PORT, &GPIO_InitStructure);
	
	// Nucleo to Discovery Handshake  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Pin   = NUCLEO_TO_DISCOVERY_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_TO_DISCOVERY_GPIO_PORT, &GPIO_InitStructure);
	
}
