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

#define TEMPERATURE_INTERRUPT_PORT				GPIOA
#define TEMPERATURE_INTERRUPT_PIN					GPIO_PIN_8	// GPIO_A8

#define ACCELEROMETER_INTERRUPT_PORT			GPIOA
#define ACCELEROMETER_INTERRUPT_PIN				GPIO_PIN_3	// GPIO_A3

#define LEDSTATE_INTERRUPT_PORT						GPIOA
#define LEDSTATE_INTERRUPT_PIN						GPIO_PIN_2	// GPIO_A2

/* Public variables ----------------------------------------------------------*/

extern osThreadId tid_Thread_TempSensor; 
extern osThreadId tid_Thread_Accelerometer;
extern osThreadId tid_Thread_UserInterface;

extern SPI_HandleTypeDef NucleoSpiHandle;

// Mutexes
extern osMutexId temperatureMutex;
extern osMutexId tiltAnglesMutex;
extern osMutexId ledStateMutex;

// Shared variables
extern float temperatureValue;
extern float accelerometer_out[3];
extern float rollValue, pitchValue;
extern uint8_t LED_ROTATE_STATE;
extern uint8_t LED_DC_PRESCALER;
extern int DOUBLE_TAP_BOOLEAN;

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
