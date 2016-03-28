#ifndef THREAD_TEMPSENSOR_H
#define THREAD_TEMPSENSOR_H

/* Includes ------------------------------------------------------------------*/              
#include "global_vars.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_gpio.h"

// TODO: Determine these values
#define tempKalman_q 0.1
#define tempKalman_r 2.0
#define tempKalman_x 25.0
#define tempKalman_p 0.0
#define tempKalman_k 0.0

/* Private typedef -----------------------------------------------------------*/

int start_Thread_TempSensor(void);
void Thread_TempSensor (void const *argument);  
void updateTemp(void);
void ADC_config(void);
int timer(__IO uint32_t time);
void Delay(uint32_t time);

#endif