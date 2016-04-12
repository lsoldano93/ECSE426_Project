
/* Includes ------------------------------------------------------------------*/
#include "NucleoSPI.h"
#include <math.h>

/* Private variables ---------------------------------------------------------*/

void Read_DataLines(int* inputArray){

	// inputArray[0] = MSB, read from Datai3 {Datai3, Datai2, Datai1, Datai0} = {inputArray[0],inputArray[1],inputArray[2],inputArray[3]}
	if(HAL_GPIO_ReadPin(NUCLEO_DATAi3_GPIO_PORT, NUCLEO_DATAi3_PIN) == GPIO_PIN_SET) inputArray[0] = 1;
	else inputArray[0] = 0;
	
	if(HAL_GPIO_ReadPin(NUCLEO_DATAi2_GPIO_PORT, NUCLEO_DATAi2_PIN) == GPIO_PIN_SET) inputArray[1] = 1;
	else inputArray[1] = 0;
	
	if(HAL_GPIO_ReadPin(NUCLEO_DATAi1_GPIO_PORT, NUCLEO_DATAi1_PIN) == GPIO_PIN_SET) inputArray[2] = 1;
	else inputArray[2] = 0;
	
	if(HAL_GPIO_ReadPin(NUCLEO_DATAi0_GPIO_PORT, NUCLEO_DATAi0_PIN) == GPIO_PIN_SET) inputArray[3] = 1;
	else inputArray[3] = 0;
	
	return;
}
	

void Set_DataLines(int* inputArray){
	
	// inputArray[0] = MSB, write to Datao3 {Datao3, Datao2, Datao1, Datao0} = {inputArray[0],inputArray[1],inputArray[2],inputArray[3]}
	if(inputArray[0] == 1) HAL_GPIO_WritePin(NUCLEO_DATAo3_GPIO_PORT, NUCLEO_DATAo3_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(NUCLEO_DATAo3_GPIO_PORT, NUCLEO_DATAo3_PIN, GPIO_PIN_RESET);
	
	if(inputArray[1] == 1) HAL_GPIO_WritePin(NUCLEO_DATAo2_GPIO_PORT, NUCLEO_DATAo2_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(NUCLEO_DATAo2_GPIO_PORT, NUCLEO_DATAo2_PIN, GPIO_PIN_RESET);
	
	if(inputArray[2] == 1) HAL_GPIO_WritePin(NUCLEO_DATAo1_GPIO_PORT, NUCLEO_DATAo1_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(NUCLEO_DATAo1_GPIO_PORT, NUCLEO_DATAo1_PIN, GPIO_PIN_RESET);
	
	if(inputArray[3] == 1) HAL_GPIO_WritePin(NUCLEO_DATAo0_GPIO_PORT, NUCLEO_DATAo0_PIN, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(NUCLEO_DATAo0_GPIO_PORT, NUCLEO_DATAo0_PIN, GPIO_PIN_RESET);
	
	return;
}

void Reset_DataLines(){
	
	HAL_GPIO_WritePin(NUCLEO_DATAo3_GPIO_PORT, NUCLEO_DATAo3_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NUCLEO_DATAo2_GPIO_PORT, NUCLEO_DATAo2_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NUCLEO_DATAo1_GPIO_PORT, NUCLEO_DATAo1_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NUCLEO_DATAo0_GPIO_PORT, NUCLEO_DATAo0_PIN, GPIO_PIN_RESET);
	
	return;
}

void Master_Write(uint8_t VariableToWrite){

	int i;
	int outputArray[8];
	int tempArray[4];
	
	// LED state will be in format (LED_State*100) + DC Prescalar
	// Because VariableToWrite is an integer, division by 100 will leave factor of 100
	if (VariableToWrite/100 == 0){
		// LED State = 0 = Off
		// Prescalar doesn't matter; DC not a factor
		for(i=0;i<6;i++) outputArray[i] = 0;
		outputArray[6] = 0;
		outputArray[7] = 0;
	}
	else if (VariableToWrite/100 == 1){
		// LED State = 1 = On
		outputArray[6] = 0;
		outputArray[7] = 1;
		
		// Convert into bit-stream array (MSB first {7,6,5,4,3,2})
		VariableToWrite -= 100;
		for (i= 0;i<6;i++) outputArray[5-i] = VariableToWrite & (1 << i) ? 1 : 0;

	}
	else if (VariableToWrite/100 == 2){
		// LED State = 2 = CW Rotation
		// Prescalar doesn't matter; DC not a factor
		for(i=0;i<6;i++) outputArray[i] = 0;
		outputArray[6] = 1;
		outputArray[7] = 0;
	}
	else if (VariableToWrite/100 == 3){
		// LED State = 3 = CCW Rotation
		// Prescalar doesn't matter; DC not a factor
		for(i=0;i<6;i++) outputArray[i] = 0;
		outputArray[6] = 1;
		outputArray[7] = 1;
	}
	
	// Send first 4 bits of byte (MSB first)
	for(i=0;i<4;i++)tempArray[i]=outputArray[i];
	Set_DataLines(tempArray);
	
	// Indicate that data ready to be read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_RESET);
	
	// Wait for Discovery to finish reading
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET);
	
	// Send last 4 bits of byte (MSB first)
	for(i=0;i<4;i++)tempArray[i]=outputArray[4+i];
	Set_DataLines(tempArray);
	
	// Indicate that data ready to be read again
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
	
	// Wait for Discovery to finish reading
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_RESET);
	
	Reset_DataLines();

	return;
	
}



float Master_Read(){

	int i;
	
	// Can only read decimal values between 0.0 & 0.99
	int integerValue[8];
	int decimalValue[8];
	int tempArray[4];
	
	float tempDecimalValue;
	float returnValue;
	
	// Wait for Discovery to finish writing
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET);
	
	// Read first four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)integerValue[i]=tempArray[i];
	
	// Signal to Discovery that pins have been read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_RESET);
	
	// Wait for Discovery to finish writing
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_RESET);
	
	// Read next four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)integerValue[i+4]=tempArray[i];
	
	// Signal to Discovery that pins have been read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
	
	// Wait for Discovery to finish writing
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET);
	
	// Read first four bits of decimal value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)decimalValue[i]=tempArray[i];
	
	// Signal to Discovery that pins have been read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_RESET);
	
	// Wait for Discovery to finish writing
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_RESET);
	
	// Read next four bits of integer value (MSB first)
	Read_DataLines(tempArray);
	for(i=0;i<4;i++)decimalValue[i+4]=tempArray[i];
	
	// Signal to Discovery that pins have been read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
	
	// Convert integer bit stream into float value
	for(i=0;i<8;i++) returnValue += ((float)pow(2,i)) * integerValue[7-i];
	for(i=0;i<8;i++) tempDecimalValue += ((float)pow(2,i)) * decimalValue[7-i];
	
	returnValue += (tempDecimalValue/100);
	
	return returnValue;
	
}

float Master_Read_Boolean(){
	
	float boolean;
	
	// Wait for Discovery to finish writing
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET);
	
	// Read input port 0 (pin set = true, pin reset = false)
	if(HAL_GPIO_ReadPin(NUCLEO_DATAi0_GPIO_PORT, NUCLEO_DATAi0_PIN) == GPIO_PIN_SET) boolean = 1;
	else boolean = 0;
	
	// Signal to Discovery that pins have been read
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_RESET);
	
	// Reset to proper pin order
	while(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_RESET);
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
	
	return boolean;
}


// float* = float[4] = {temperature, pitch, roll, double tap boolean}
// uint8_t = bits[7..2] = Duty cycle prescalar, bits[1..0] = LED state
void Master_Communication(uint8_t LED_STATE, float* returnArray){
	
	// Read temperature value
	returnArray[0] = Master_Read();
	
	// Read pitch value
	//returnArray[1] = Master_Read();
	
	// Read roll value
	//returnArray[2] = Master_Read();
	
	// Read double tap boolean
	//returnArray[3] = Master_Read_Boolean();
	
	// Send LED_State & Duty Cycle
	//Master_Write(LED_STATE);
	
	return;
}

/**
  * @brief  Configures Nucleo board for SPI communication with Discovery board
  * @param  None
  * @retval None
  */
void NucleoSPI_Config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	NUCLEO_DATAio_CLOCK_ENABLE();
	NUCLEO_HSI_CLOCK_ENABLE();                                                 
	
	// NUCLEO Output Pin 0  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAo0_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAo0_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Output Pin 1  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAo1_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAo1_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Output Pin 2  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAo2_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAo2_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Output Pin 3  (Output - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAo3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAo3_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO to Discovery Handshake Pin  (Output - Active Low)
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Pin   = NUCLEO_TO_DISCOVERY_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_TO_DISCOVERY_GPIO_PORT, &GPIO_InitStructure);
	
	/* Instantiate pins to high for active low state */
	Reset_DataLines();
	
	/* Instantiate HS pin to high for active low state */
	HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
	
	// NUCLEO Input Pin 0  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAi0_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAi0_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Input Pin 1  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAi1_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAi1_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Input Pin 2  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAi2_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAi2_GPIO_PORT, &GPIO_InitStructure);
	
	// NUCLEO Input Pin 3  (Input - Active High)
	GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
	GPIO_InitStructure.Pin   = NUCLEO_DATAi3_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(NUCLEO_DATAi3_GPIO_PORT, &GPIO_InitStructure);
	
	// Discovery to Nucleo Handshake Pin  (Input - Active Low)
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Pin   = DISCOVERY_TO_NUCLEO_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(DISCOVERY_TO_NUCLEO_GPIO_PORT, &GPIO_InitStructure);
	
	/* Setup input interrupt line from Discovery */
  GPIO_InitStructure.Pin = NUCLEO_INTERRUPT_PIN;
	GPIO_InitStructure.Mode  = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(NUCLEO_INTERRUPT_PORT, &GPIO_InitStructure);
		
	/* Configure the NVIC for SPI */  
  HAL_NVIC_SetPriority(EXTI4_IRQn, 4, 0);    
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	
}

