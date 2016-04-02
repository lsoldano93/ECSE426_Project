
#include "Thread_TempSensor.h"

float temperatureValue;

/* Private variables ---------------------------------------------------------*/

osThreadId tid_Thread_TempSensor;  

ADC_HandleTypeDef ADC1_Handle;
kalman_t kalman_temperature;
const void* temperatureMutexPtr;

osThreadDef(Thread_TempSensor, osPriorityNormal, 1, NULL); 

/**  Initiates temperature sensor thread
   * @brief  Builds thread and starts it
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_TempSensor (void) {

  tid_Thread_TempSensor = osThreadCreate(osThread(Thread_TempSensor ), NULL); 
  if (!tid_Thread_TempSensor){
		printf("Error starting temperature sensor thread!");
		return(-1); 
	}
  return(0);
}

/**  Runs temperature sensor thread which updates temperature value for display
   * @brief  Obtains temperature voltage readout from ADC1 Channel 16
   */
void Thread_TempSensor (void const *argument){
	
	osEvent Status_TempSensor;

	// Update temperature values when signaled to do so, clear said signal after execution
	while(1){
		
		Status_TempSensor = osSignalWait((int32_t) THREAD_GREEN_LIGHT, (uint32_t) THREAD_TIMEOUT);
		updateTemp();
		osSignalSet(tid_Thread_SPICommunication, (int32_t) THREAD_GREEN_LIGHT);

	}                                                       
}
	
/**  Get temperature
   * @brief  Obtains temperature voltage readout from ADC1 Channel 16
   * @param  None
   * @retval Temperature float in celcius
   */
void updateTemp(void) {

	float VSENSE;	
	
	// Obtain temperature voltage value from ADC
	//need to get poll working
	VSENSE = HAL_ADC_GetValue(&ADC1_Handle); 

	// Filter raw temperature sensor values
	Kalmanfilter_asm(&VSENSE, &VSENSE, 1, &kalman_temperature);
	//printf("%f\n", VSENSE);
	
	/* Obtain permission for access to tempValue and then Supdate
	   ---------------------------------------------------------
		 ADC 3.0 Volts per 2^12 steps (12 bit resolution in configuration)
	   Voltage at 25C is 760mV
	   Avg slop is 25mV/1C 
	   --------------------------------------------------------- */
	osMutexWait(temperatureMutex, (uint32_t) THREAD_TIMEOUT);
	temperatureValue  = (VSENSE*(3.0f/ 4096.0f) - 0.76f)/0.025f + 25.0f;	
	//printf("Temperature Value: %f\n", temperatureValue);
	osMutexRelease(temperatureMutex);	
	
}
 

/**  ADC Configuration
   * @brief  Configures ADC1 Channel 16 so that temperature values can be read
   */
void ADC_config(void) {  // TODO: Make this configuration proper so that it actually works

	ADC_ChannelConfTypeDef ADC1_ch16;
	
	
	// Initialize values for ADC1 handle type def
	ADC1_Handle.Instance = ADC1;
	ADC1_Handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	ADC1_Handle.Init.Resolution = ADC_RESOLUTION_12B;    									 
	ADC1_Handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;         						
	ADC1_Handle.Init.ScanConvMode = DISABLE;           
	ADC1_Handle.Init.EOCSelection = DISABLE;         			
	ADC1_Handle.Init.ContinuousConvMode = ENABLE;      //
	ADC1_Handle.Init.DMAContinuousRequests = DISABLE;  
	ADC1_Handle.Init.NbrOfConversion = 1;       													
	ADC1_Handle.Init.DiscontinuousConvMode = DISABLE;  
	ADC1_Handle.Init.NbrOfDiscConversion = 0;    
	ADC1_Handle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;       //
	ADC1_Handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;  //
	
	
	// Initialize values for temperature sensor (Temperature analog channel is Ch16 of ADC1)
	ADC1_ch16.Channel = ADC_CHANNEL_16;
	ADC1_ch16.Rank = 1;
	ADC1_ch16.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	ADC1_ch16.Offset = 0;
	
	
	// Enable ADC clock
	__ADC1_CLK_ENABLE();
	
	// Initialize clock with error handling

	if(HAL_ADC_Init(&ADC1_Handle)!=HAL_OK){
		//Error_Handler(ADC_INIT_FAIL);
		printf("adc init fail\n");
	}
	// Configure temperature sensor peripheral 
	HAL_ADC_ConfigChannel(&ADC1_Handle, &ADC1_ch16);
	
	HAL_ADC_Start(&ADC1_Handle);
	
	// Allot values to the kalman filtration struct for the temperature sensor
	kalman_temperature.q = 0.3;
	kalman_temperature.r = 1.2;
	kalman_temperature.x = 1000.0;
	kalman_temperature.p = 0.0;
	kalman_temperature.k = 0.0;
	
	// Initialize temperature sensor mutex
	temperatureMutex = osMutexCreate(temperatureMutexPtr);
	
}

