#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <stdio.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

#define THREAD_RED_LIGHT 0  	 // Signal for thread to wait
#define THREAD_GREEN_LIGHT 2   // Signal for thread to execute
#define THREAD_TIMEOUT 1000     // Thread timeout value in milliseconds

/* Public variables ----------------------------------------------------------*/

extern osThreadId tid_Thread_TempSensor; 
extern osThreadId tid_Thread_Accelerometer;
extern osThreadId tid_Thread_UserInterface;
extern osThreadId tid_Thread_SPICommunication;

// Mutexes
extern osMutexId temperatureMutex;
extern osMutexId tiltAnglesMutex;
extern osMutexId ledStateMutex;

// Shared variables
extern float temperatureValue;
extern float accelerometer_out[3];
extern float rollValue, pitchValue;
extern int alarmCount;
extern uint8_t LED_ROTATE_STATE;

// SPI Handles
extern SPI_HandleTypeDef SpiHandle;
extern SPI_HandleTypeDef NucleoSpiHandle;

// Timer 3 values
extern TIM_HandleTypeDef handle_tim3;
extern TIM_HandleTypeDef handle_tim4;
extern uint32_t timingDelay;

/* @brief Structure for the Kalman filter  */ 
typedef struct kalman_t{
	float q;
	float r;
	float x;
	float p;
	float k;
} kalman_t;

/* Private function prototypes -----------------------------------------------*/

/**  Assembly Kalmann filter function
   * @brief  Filters values to remove noisy fluctuations
	 * @param  Input value of measurements, array to output to, length of arrays, and kalman parameter struct
   * @retval Returns updated output array **/
int Kalmanfilter_asm(float* inputArray, float* outputArray, int arrayLength, kalman_t* kalman);

#endif
