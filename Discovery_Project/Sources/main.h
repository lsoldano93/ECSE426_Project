#ifndef MAIN_H
#define MAIN_H

#include "global_vars.h"
#include "RTE_Components.h"             // Component selection
#include "Thread_TempSensor.h"
#include "Thread_Accelerometer.h"
#include "Thread_UserInterface.h"
#include "Thread_SPICommunication.h"

extern osThreadId tid_Thread_LED;
extern osThreadId tid_Thread_TempSensor; 
extern osThreadId tid_Thread_Accelerometer;
extern osThreadId tid_Thread_UserInterface;

osMutexId temperatureMutex;
osMutexId tiltAnglesMutex;
TIM_HandleTypeDef handle_tim3;

uint8_t tim3_ticks;
uint32_t timingDelay;

void SystemClock_Config(void);

/**  Timer3 initialization function
   * @brief  Function to initialize timer 3 **/
void init_TIM3(void);

/**
  * @brief  This function handles accelerometer interrupt requests
  * @param  None
  * @retval None
  */
//void EXTI0_IRQHandler(void);

/**
  * @}
  */ 
void TIM3_IRQHandler(void);

/**  Runs user interface system
   * @brief  Initializes threads and peripherals to maintian a RTOS for the user **/
int main (void);




#endif