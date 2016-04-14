#ifndef THREAD_USERINTERFACE_H
#define THREAD_USERINTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"


/* Private define ------------------------------------------------------------*/
#define UI_THREAD_OSDELAY 300  // Delay in milliseconds

#define LED1 							GPIO_PIN_12
#define LED2 							GPIO_PIN_13
#define LED3 							GPIO_PIN_14
#define LED4 							GPIO_PIN_15


/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**  Initiates user interface thread
   * @brief  Builds thread and starts it
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_UserInterface (void);

/**  User interface thread 
   * @brief  Runs UI thread which reads key presses and updates the display accordingly
   */
void Thread_UserInterface (void const *argument);

/**  Resets all display pins
   * @brief  "" **/
void resetPins();

void ledsRotate(void);

void ledsCounterRotate(void);

void ledsOn(void);

void ledsOff(void);

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim);


#endif