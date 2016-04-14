
/* Includes ------------------------------------------------------------------*/
#include "NucleoSPI.h"
#include <math.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef DiscoverySpiHandle;


void Master_Write(uint8_t VariableToWrite){

	uint16_t message = 0;
	uint16_t returnValue = 0;
	uint8_t tempValue = 0;
	
	// LED state will be in format (LED_State*100) + DC Prescalar
	// Because VariableToWrite is an integer, division by 100 will leave factor of 100
	if (VariableToWrite/100 == 0){
		// LED State = 0 = Off
		// Prescalar doesn't matter; DC not a factor
		message = 0x0000;
	}
	else if (VariableToWrite/100 == 1){
		// LED State = 1 = On
		tempValue = VariableToWrite - 100;
		message = (uint16_t)(VariableToWrite << 2);
		message += 0x0001;
	}
	else if (VariableToWrite/100 == 2){
		// LED State = 2 = CW Rotation
		// Prescalar doesn't matter; DC not a factor
		message = 0x0002;
	}
	else if (VariableToWrite/100 == 3){
		// LED State = 3 = CCW Rotation
		// Prescalar doesn't matter; DC not a factor
		message = 0x0003;
	}
	
	// Write LEDstate command 
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	DiscoverySpiHandle.Instance->DR = COMMAND_LEDSTATE;
	
	// Wait for line to be free for sending
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_RXNE) == RESET);
	returnValue = DiscoverySpiHandle.Instance->DR;
	
	// Send message
	DiscoverySpiHandle.Instance->DR = message;
	
	// Wait for message to be read and clear DR
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_RXNE) == RESET);
	
	returnValue = DiscoverySpiHandle.Instance->DR;
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_BSY) != RESET);

	return;
	
}



float Master_Read(uint16_t Instruction){

	uint16_t messageValue = 0;
	
	// Can only read decimal values between 0.0 & 0.99
	float tempDecimalValue;
	float returnValue;
	
	// Write command 
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	DiscoverySpiHandle.Instance->DR = Instruction;
	
	// Wait for line to be free for sending
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_RXNE) == RESET);
	returnValue = DiscoverySpiHandle.Instance->DR;
	
	// Generate clock and wait for read
	DiscoverySpiHandle.Instance->DR = 0x0000;
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_TXE) == RESET);
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_RXNE) == RESET);
	
	messageValue = DiscoverySpiHandle.Instance->DR;
	
	// Wait for line release
	while(__HAL_SPI_GET_FLAG(&DiscoverySpiHandle, SPI_FLAG_BSY) != RESET);
	
	// Convert message into meaningful values
	returnValue = (uint8_t) (messageValue >> 9);
	tempDecimalValue = messageValue - (((uint16_t)returnValue) << 9);
	returnValue += (tempDecimalValue/100);
	
	return returnValue;
	
}

/**
  * @brief  Configures Nucleo board for SPI communication with Discovery board
  * @param  None
  * @retval None
  */
void NucleoSPI_Config(void){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* SPI configuration -------------------------------------------------------*/
	__HAL_RCC_SPI2_CLK_ENABLE();
	
  HAL_SPI_DeInit(&DiscoverySpiHandle);
  DiscoverySpiHandle.Instance 							  = SPI2;
  DiscoverySpiHandle.Init.BaudRatePrescaler 	= SPI_BAUDRATEPRESCALER_4;
  DiscoverySpiHandle.Init.Direction 					= SPI_DIRECTION_2LINES;
  DiscoverySpiHandle.Init.CLKPhase 						= SPI_PHASE_1EDGE;
  DiscoverySpiHandle.Init.CLKPolarity 				= SPI_POLARITY_LOW;
  DiscoverySpiHandle.Init.CRCCalculation			= SPI_CRCCALCULATION_DISABLED;
  DiscoverySpiHandle.Init.CRCPolynomial 			= 7;
  DiscoverySpiHandle.Init.DataSize 						= SPI_DATASIZE_16BIT;
  DiscoverySpiHandle.Init.FirstBit 						= SPI_FIRSTBIT_MSB;
  DiscoverySpiHandle.Init.NSS 								= SPI_NSS_SOFT;
  DiscoverySpiHandle.Init.TIMode 							= SPI_TIMODE_DISABLED;
  DiscoverySpiHandle.Init.Mode 								= SPI_MODE_MASTER;
	if (HAL_SPI_Init(&DiscoverySpiHandle) != HAL_OK) {printf ("ERROR: Error in initialising SPI2 \n");};
  
	__HAL_SPI_ENABLE(&DiscoverySpiHandle);
	
	NUCLEO_SPI_CLOCK_ENABLE();
	TEMPERATURE_INTERRUPT_CLOCK_ENABLE();
	ACCELEROMETER_INTERRUPT_CLOCK_ENABLE();
	LEDSTATE_INTERRUPT_CLOCK_ENABLE();	
	
	__SPI2_CLK_ENABLE();
	
	// Configure SPI pins
	GPIO_InitStructure.Pull  				= GPIO_PULLDOWN;
	GPIO_InitStructure.Mode  				= GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 				= GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitStructure.Alternate    = GPIO_AF5_SPI2;
	
	GPIO_InitStructure.Pin   = NUCLEO_SCK_PIN | NUCLEO_MISO_PIN | NUCLEO_MOSI_PIN;
	HAL_GPIO_Init(NUCLEO_SPI_GPIO_PORT, &GPIO_InitStructure);
	
	/* Setup input interrupt line from Discovery */
	GPIO_InitStructure.Mode  = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	
	GPIO_InitStructure.Pin = TEMPERATURE_INTERRUPT_PIN;
	HAL_GPIO_Init(TEMPERATURE_INTERRUPT_PORT, &GPIO_InitStructure);
		
	/* Configure the NVIC for SPI */  
  HAL_NVIC_SetPriority(EXTI4_IRQn, 4, 0);    
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	
//	HAL_NVIC_SetPriority(EXTI2_IRQn, 4, 0);    
//  HAL_NVIC_EnableIRQ(EXTI2_IRQn);
//	
//	HAL_NVIC_SetPriority(EXTI3_IRQn, 4, 0);    
//  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	
}

