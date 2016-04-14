
/* Includes ------------------------------------------------------------------*/
#include "Thread_UserInterface.h"


/* Private variables ---------------------------------------------------------*/

osThreadId tid_Thread_UserInterface; 

TIM_HandleTypeDef handle_tim4;
TIM_OC_InitTypeDef init_OC;

uint8_t ledState = 1;
uint8_t rotateClockwise = 0;
uint8_t currentLED = 1;
uint8_t dutyCyclePrescaler = 1;

uint32_t DutyCycle = 8399;
uint32_t selectedDutyCycle;

uint8_t LED_ROTATE_STATE;
uint8_t LED_DC_PRESCALER;
const void* ledStateMutexPtr;

osThreadDef(Thread_UserInterface, osPriorityNormal, 1, NULL);  // TODO: See if below normal priority helps with constant calling


/* Private functions ---------------------------------------------------------*/
	
/**  Initiates user interface thread
   * @brief  Builds thread and starts it
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_UserInterface (void) {

  tid_Thread_UserInterface = osThreadCreate(osThread(Thread_UserInterface ), NULL); 
  if (!tid_Thread_UserInterface){
		printf("Error starting user interface thread!");
		return(-1); 
	}
  return(0);
}

/**  User interface thread 
   * @brief  Runs UI thread which reads key presses and updates the display accordingly
   */
void Thread_UserInterface (void const *argument){
	
	// Initialize these values
	osMutexWait(ledStateMutex, (uint32_t) THREAD_TIMEOUT);
	LED_ROTATE_STATE = ledState;
	LED_DC_PRESCALER = dutyCyclePrescaler;
	osMutexRelease(ledStateMutex);
	
	while(1){
		
		// TODO: To implement LED speed take in a value  with LED state that adds/subtracts this osDelay
		osDelay(UI_THREAD_OSDELAY);
		
		HAL_GPIO_WritePin(LEDSTATE_INTERRUPT_PORT, LEDSTATE_INTERRUPT_PIN, GPIO_PIN_SET);
		
		osMutexWait(ledStateMutex, (uint32_t) THREAD_TIMEOUT);
		ledState = LED_ROTATE_STATE;
		dutyCyclePrescaler = LED_DC_PRESCALER;
		osMutexRelease(ledStateMutex);
		
		selectedDutyCycle = DutyCycle/dutyCyclePrescaler;
		
		/* LED_ROTATE_STATE = 0  -> Off
			 LED_ROTATE_STATE = 1  -> All On
			 LED_ROTATE_STATE = 2  -> Rotate CW
			 LED_ROTATE_STATE = 3  -> Rotate CCW */
		
		if (ledState == 0) ledsOff();
		else if (ledState == 1) ledsOn();
		else if (ledState == 2){
			rotateClockwise = 1;
			ledsRotate();
		}
		else if (ledState == 3){
			rotateClockwise = 0;
			ledsRotate();
		}
			
	}
}

void ledsRotate(void) {
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// Give initialization values for GPIO D pin sets
	GPIO_InitStructure.Pin = LED1 | LED2 | LED3 | LED4;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed =  GPIO_SPEED_FREQ_HIGH;
	
	// Initialize all GPIO pin set
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	if (currentLED == 1){
		if (rotateClockwise){
  
			HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_SET);
			currentLED = 2;
		}
		else{
			
			HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_SET);
			currentLED = 4;
		}
	}
	else if (currentLED == 2){
		if (rotateClockwise){
			
			HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_SET);
			currentLED = 3;
		}
		else{
			
			HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_SET);
			currentLED = 1;
		}
	}
	else if (currentLED == 3){
		if (rotateClockwise){
			
			HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_SET);
			currentLED = 4;
		}
		else{
			
			HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_SET);
			currentLED = 2;
		}
	}
	else {
		if (rotateClockwise){
			
			HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_SET);
			currentLED = 1;
		}
		else{
			
			HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_SET);
			currentLED = 3;
		}
	}
	

}

void ledsOn(void) {
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF2_TIM4;
	GPIO_InitStructure.Pin = LED1 | LED2 | LED3 | LED4;
	
	// Initialize all GPIO pin set
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	init_OC.Pulse = selectedDutyCycle;
	
	HAL_TIM_OC_ConfigChannel(&handle_tim4, &init_OC, TIM_CHANNEL_4);
	HAL_TIM_OC_Start(&handle_tim4, TIM_CHANNEL_4);
	
	HAL_TIM_OC_ConfigChannel(&handle_tim4, &init_OC, TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&handle_tim4, TIM_CHANNEL_3);
	
	HAL_TIM_OC_ConfigChannel(&handle_tim4, &init_OC, TIM_CHANNEL_2);
	HAL_TIM_OC_Start(&handle_tim4, TIM_CHANNEL_2);
	
	HAL_TIM_OC_ConfigChannel(&handle_tim4, &init_OC, TIM_CHANNEL_1);
	HAL_TIM_OC_Start(&handle_tim4, TIM_CHANNEL_1);
	
}

void ledsOff(void) {
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// Give initialization values for GPIO D pin sets
	GPIO_InitStructure.Pin = LED1 | LED2 | LED3 | LED4;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed =  GPIO_SPEED_FREQ_HIGH;
	
	// Initialize all GPIO pin set
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_RESET);
}


void init_TIM4(void) {
	
	
	TIM_Base_InitTypeDef init_TIM4;
	
	// Enable clock for TIM4 
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	init_TIM4.Prescaler = 0;
	init_TIM4.CounterMode = TIM_COUNTERMODE_UP;
	init_TIM4.Period = 8399;  // 10 kHz
	init_TIM4.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	
	// Initialize timer 4 handle struct
	handle_tim4.Instance = TIM4;
	handle_tim4.Init = init_TIM4;
	handle_tim4.State = HAL_TIM_STATE_RESET;
	
	// Initialize timer 4 handle 
	HAL_TIM_OC_Init(&handle_tim4);

	// Setup PWM
	init_OC.OCMode = TIM_OCMODE_PWM1;
	init_OC.OCPolarity = TIM_OCPOLARITY_HIGH;
	
	// Start LED clock
	__GPIOD_CLK_ENABLE();
	
	// Setup MUTEX
	ledStateMutex = osMutexCreate(ledStateMutexPtr);
	
}

