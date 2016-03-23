
/* Includes ------------------------------------------------------------------*/
#include "Thread_UserInterface.h"


/* Private variables ---------------------------------------------------------*/
const int maxTemp = 35.0;


osThreadId tid_Thread_UserInterface; 
GPIO_InitTypeDef GPIO_row, GPIO_col;
	
uint8_t key_state;
uint8_t tilt_state;   

osThreadDef(Thread_UserInterface, osPriorityBelowNormal, 1, NULL);  // TODO: See if below normal priority helps with constant calling

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

	float tempValue = 0.0;
	float rollAngle = 0.0;
	float pitchAngle = 0.0;
	int counter = 0;
	
	/* Initialize key and tilt states
		 Key States = {0:Temperature, 1:Tilt Angles}
		 States = {0:Roll, 1:Pitch}                    */
	key_state = 0;
	tilt_state = 0;
	
	while(1){
		
		osDelay(UI_THREAD_OSDELAY);
		
		// Read keys
		//handleKeyPress();
		
		// temperature alarm goes off and blinks segment display
		if(temperatureValue >= maxTemp) {
				counter++;
				if(counter >= 30) {
					counter = 0;
					resetPins();
					Delay(200);					
				}				
				
		}

		/* If key state is 0 then get access to temperature variable
		   before drawing it to the display */
		if (key_state == 0){
			osMutexWait(temperatureMutex, (uint32_t) THREAD_TIMEOUT);
			tempValue = temperatureValue;			
			osMutexRelease(temperatureMutex);
			counter++;
			drawTemperature(tempValue);
			
		}
		/* Key state must be one, so see which tilt value to display and gain access
		   before drawing it to the display */
		else {
			if (tilt_state == 0){
				osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
				rollAngle = rollValue;
				osMutexRelease(tiltAnglesMutex);
				drawAngle(rollAngle);
			}
			else if (tilt_state == 1){
				osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
				pitchAngle = pitchValue;
				osMutexRelease(tiltAnglesMutex);
				drawAngle(pitchAngle);
			}
			
		}                                       
	}
	
}


/**  Handle key presses
   * @brief Pressing 0 toggles temperature/accelerometer; pressing 1/2 toggles tilt angles
   */
void handleKeyPress(void) {

	int key; 
	
	if ((key = getKey()) == -1) return;
	if (key == 0 && key_state == 0) key_state = 1;
	else if (key == 0 && key_state == 1) key_state = 0;
	else if (key_state == 1 && key == 1) tilt_state = 0;
	else if (key_state == 1 && key == 3) tilt_state = 1;
		
	return;
	
}


/**  Resets all display pins
   * @brief  "" **/
void resetPins() {
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //a
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //b
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //c
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); // Digit 1
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // Digit 2
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // Digit 3
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); // Digit 4
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //degree
	
}


/**  Uses timer 3 to generate delay for display
   * @brief  Allows for software delay to be used for display purposes **/
void Delay(uint32_t time){
	
	timingDelay = time;
  while(timingDelay !=0);

	
}



/**  Configures system for user interface
   * @brief  Initializes pins for display and clock for keypad **/
void UserInterface_config(void){

}

void ledsRotate(void) {

	// Green light on
	if (ledCount < 500){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	}
	else {
		
		// Orange light on
		if (ledCount < 1000){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
		}
		else{
			
			// Red light on
			if (ledCount < 1500){
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			}
			
			// Blue light on
			else{
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
			}
		}
	}

}
void ledsCounterRotate(void) {
	

	if (ledCount < 500){
		// Blue light on
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
	}
	else {
				
		if (ledCount < 1000){
			// Red light on
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			
		}

		else {			
			
			if (ledCount < 1500){
				// Orange light on
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			}
						
			else {
				// Green light on
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
			}
		}
	}
}

void ledsOn(void) {
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
}

void ledsOff(void) {
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

