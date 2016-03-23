/*******************************************************************************
  * @file    main.c
  * @author  Ashraf Suyyagh
	* @version V1.2.0
  * @date    17-January-2016
  * @brief   This file demonstrates flasing one LED at an interval of one second
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */

#include "main.h"


/**
	These lines are mandatory to make CMSIS-RTOS RTX work with te new Cube HAL
*/
#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) {
  return os_time; 
}
#endif

/**
  * System Clock Configuration
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}



/**  Timer3 initialization function
   * @brief  Function to initialize timer 3 **/
void init_TIM3(void) {
	
	TIM_Base_InitTypeDef init_TIM;
	
	// Enable clock for TIM3 
	__HAL_RCC_TIM3_CLK_ENABLE();
	
	/* Desired Rate = ClockFrequency / (prescaler * period)
		 Rate = 1000Hz, frequency = 84MHz																		
		 need to setup period and prescaler
		 set rate to 500Hz
		 Initialize timer 3 initialization struct     */
	init_TIM.Period = 83;  // Period is in MHz
	init_TIM.Prescaler = 999;
	init_TIM.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	init_TIM.CounterMode = TIM_COUNTERMODE_UP;
	
	// Initialize timer 3 handle struct
	handle_tim3.Instance = TIM3;
	handle_tim3.Init = init_TIM;
	handle_tim3.Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
	handle_tim3.Lock = HAL_UNLOCKED;
	handle_tim3.State = HAL_TIM_STATE_READY;

	// Initialize timer 3 handle and enable interrupts
	HAL_TIM_Base_MspInit(&handle_tim3);
	HAL_TIM_Base_Init(&handle_tim3);
	HAL_TIM_Base_Start_IT(&handle_tim3);
		
	// Configure NVIC 
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_SetPriority(TIM3_IRQn, 0,0);
	
}


/**
  * @brief  This function handles accelerometer interrupt requests
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void){
	// Handle external interrupts on pin 0
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
	
}


/**
  * @}
  */ 
void TIM3_IRQHandler(void) {
	// Handle interrupts deriving from timer 3
	HAL_TIM_IRQHandler(&handle_tim3);
	
}


/**
  * @brief  Input Capture callback in non blocking mode 
  * @param  htim: pointer to a TIM_HandleTypeDef structure that contains
  *                the configuration information for TIM module.
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	
	/* Prevent unused argument(s) compilation warning */
  __IO uint32_t tmpreg = 0x00;
  UNUSED(tmpreg); 
	
	/* If callback regards GPIO pin associated with external accelerometer interrupt, 
		 read accelerometer to output and signal accelerometer thread to execute */
	if(GPIO_Pin == GPIO_PIN_0) {
		LIS3DSH_ReadACC(accelerometer_out);
		osSignalSet(tid_Thread_Accelerometer, (int32_t) THREAD_GREEN_LIGHT);
	}	
	
}


/**
  * @brief  Period elapsed callback in non blocking mode 
  * @param  htim: pointer to a TIM_HandleTypeDef structure that contains
  *                the configuration information for TIM module.
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  
	// Only perform callback if proper timer is calling it
	if (htim == &handle_tim3){
		
		if(timingDelay !=0) {
			timingDelay--;
		}
		
		tim3_ticks++;
		if (tim3_ticks > 8){
			// Send signal to temperature sensor to take in a new reading
			osSignalSet(tid_Thread_TempSensor, (int32_t) THREAD_GREEN_LIGHT);
			tim3_ticks = 0;
		}
	}
	
}


/**  Runs user interface system
   * @brief  Initializes threads and peripherals to maintian a RTOS for the user **/
int main (void) {
	
  osKernelInitialize();                     /* Initialize CMSIS-RTOS          */
  HAL_Init();                               /* Initialize the HAL Library     */
  SystemClock_Config();                     /* Configure the System Clock     */
	
	// Initialize flags and counters
	timingDelay = 0;
	tim3_ticks = 0;
	
	printf("Beginning Program\n");
	
	/* User codes goes here*/
	init_TIM3();															/* Initialize timer 3 				     	*/
  
	ADC_config();														  /* Initialize temp sensor ADC     	*/
	start_Thread_TempSensor(); 								/* Start temp sensor thread  				*/
	
	Accelerometer_config();										/* Initialize accelerometer         */
	start_Thread_Accelerometer();							/* Start accelerometer thread       */
	
	UserInterface_config();										/* Initialize display and keypad    */
	start_Thread_UserInterface();				  		/* Start UI thread                  */

	/* User codes ends here*/
  
	osKernelStart();                          /* Start thread execution         */
	
}

