#ifndef THREAD_USERINTERFACE_H
#define THREAD_USERINTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"


/* Private define ------------------------------------------------------------*/
#define UI_THREAD_OSDELAY 10  // Delay in milliseconds
#define DISPLAY_DELAY 1			 // Delay in milliseconds
#define DEBOUNCE_DELAY 25    // Delay in milliseconds

#define ROWS 4
#define COLS 3


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

/**  Configures system for user interface
   * @brief  Initializes pins for display and clock for keypad **/
void UserInterface_config(void);


#endif