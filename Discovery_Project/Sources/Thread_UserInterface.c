
/* Includes ------------------------------------------------------------------*/
#include "Thread_UserInterface.h"


/* Private variables ---------------------------------------------------------*/

osThreadId tid_Thread_UserInterface; 

TIM_HandleTypeDef handle_tim4;

uint8_t rotateClockwise = 0;
uint8_t currentLED = 1;

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
	
	//ledsOff();
	
	while(1){
		
		osDelay(UI_THREAD_OSDELAY);
		//ledsRotate();
		//ledsOn();
			
	}
}

void ledsRotate(void) {

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
	HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_SET);
}

void ledsOff(void) {
	HAL_GPIO_WritePin(GPIOD, LED1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, LED4, GPIO_PIN_RESET);
}


void init_TIM4(void) {
	
	/* Setup of this PWM timer is referencing information provided at the following URL
		 https://trix.io/output-compare-on-stm32/ */
	
	TIM_Base_InitTypeDef init_TIM4;
	TIM_OC_InitTypeDef init_OC;
	
	// Enable clock for TIM4 
	__HAL_RCC_TIM4_CLK_ENABLE();
	
	init_TIM4.Prescaler = 0;
	init_TIM4.CounterMode = TIM_COUNTERMODE_UP;
	init_TIM4.Period = 842;  // 38,005 Hz
	
	// Initialize timer 4 handle struct
	handle_tim4.Instance = TIM4;
	handle_tim4.Init = init_TIM4;
	handle_tim4.Channel = HAL_TIM_ACTIVE_CHANNEL_4; // TODO: Can I comment this out so I can jus iniiate all channels?
	handle_tim4.State = HAL_TIM_STATE_RESET;
	
	// Initialize timer 4 handle 
	HAL_TIM_OC_Init(&handle_tim4);

	// Setup PWM
	init_OC.OCMode = TIM_OCMODE_TOGGLE;
  init_OC.Pulse = 0; 
  
	HAL_TIM_OC_ConfigChannel(&handle_tim4, &init_OC, TIM_CHANNEL_4);
	HAL_TIM_OC_Start(&handle_tim4, TIM_CHANNEL_4);
	
	
}


void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim){
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(htim == &handle_tim4){
		
		__GPIOD_CLK_ENABLE();
		
		GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Pull  = GPIO_PULLUP;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStructure.Pin = LED1 | LED2 | LED3 | LED4;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
	}
	
}
