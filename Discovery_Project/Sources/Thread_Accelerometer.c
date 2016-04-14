
/* Includes ------------------------------------------------------------------*/
#include "Thread_Accelerometer.h"
#include "stdlib.h"

/* Private variables ---------------------------------------------------------*/

osThreadId tid_Thread_Accelerometer;

float accelerometer_out[3];
float movingZ[3];
uint8_t i = 0;
uint8_t lastTapI = 15;
uint8_t firstZ = 1;
uint8_t numTaps = 0;
accelerometer_values accel;
float accel_x, accel_y, accel_z;
float rollValue, pitchValue;
kalman_t kalmanX, kalmanY, kalmanZ;
int DOUBLE_TAP_BOOLEAN = 0;
const void* tiltAnglesMutexPtr;

/* Private functions ---------------------------------------------------------*/

osThreadDef(Thread_Accelerometer, osPriorityNormal, 1, NULL); 

/**  Initiates accelerometer thread
   * @brief  Builds thread and starts it
	 * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_Accelerometer (void) {
  tid_Thread_Accelerometer = osThreadCreate(osThread(Thread_Accelerometer), NULL); 
  if (!tid_Thread_Accelerometer){
		printf("Error starting acclerometer thread!");
		return(-1); 
	}
  return(0);
}

/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void Thread_Accelerometer (void const *argument){
	
	uint8_t j;
	float difference;
	
	osEvent Status_Accelerometer;
	// Update accelerometer values when signaled to do so, clear said signal after execution
	while(1){
		
		Status_Accelerometer = osSignalWait((int32_t) THREAD_GREEN_LIGHT, (uint32_t) THREAD_TIMEOUT);
		accelerometer_mode();
	
		//printf("%f,\n", accel.z);
		
		// Ghetto Double tap code
		for(j=0;j<3;j++){
			difference = movingZ[i] - movingZ[j];
			//if(i == lastTapI) lastTapI = 15;
			if(difference > TAP_THRESHOLD){
					//if(lastTapI == 15){
						numTaps++;
						firstZ = 1;
						//lastTapI = i;
						if (numTaps == 1) printf("IT BEEN TAPPED!\n");
						else if(numTaps > 1){
							printf("IT BEEN TAPPED TWICE!\n");
							osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
							DOUBLE_TAP_BOOLEAN = 1;
							osMutexRelease(tiltAnglesMutex);
							numTaps = 0;
						}
						osDelay(400);
					}
			//}
			if(firstZ) break;
		}
		
		//HAL_GPIO_WritePin(ACCELEROMETER_INTERRUPT_PORT, ACCELEROMETER_INTERRUPT_PIN, GPIO_PIN_SET);
		
	}
	
}


/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void accelerometer_mode(void) {
	
	// Access accelerometer_out and then calculate real values
	update_accel_values(accelerometer_out[0], accelerometer_out[1], accelerometer_out[2]);
	//printf("%f\n", accelerometer_out[2]);
	// Filter updated accelerometer values
	Kalmanfilter_asm(&accel.x, &accel.x, 1, &kalmanX);	
	Kalmanfilter_asm(&accel.y, &accel.y, 1, &kalmanY);	
	Kalmanfilter_asm(&accel.z, &accel.z, 1, &kalmanZ);	
	
	// Calculate tilt angles when permission to resources granted
	osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
	rollValue = calc_roll_angle();
	pitchValue = calc_pitch_angle();
	osMutexRelease(tiltAnglesMutex);
	
}


/**  Configure kalman filtering structures for x,y,z values of accelerometer
   * @brief  Gives pre-defined values to new kalman structs  **/
void config_accelerometer_kalman(void) {
	
	// Configure kalman filter for x
	kalmanX.q = 0.2;
	kalmanX.r = 1.1;
	kalmanX.x = 0.0;
	kalmanX.p = 0.0;
	kalmanX.k = 0.0;
		
	// Configure kalman filter for y
	kalmanY.q = 0.2;
	kalmanY.r = 1,1;
	kalmanY.x = 0.0;
	kalmanY.p = 0.0;
	kalmanY.k = 0.0;
	
	// Configure kalman filter for z
	kalmanZ.q = 0.2;
	kalmanZ.r = 1.1;
	kalmanZ.x = 1000.0;
	kalmanZ.p = 0.0;
	kalmanZ.k = 0.0;
	
}


/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void update_accel_values(float Ax, float Ay, float Az) {
	
	uint8_t j;
	
	accel.x = Ax*ACC11 + Ay*ACC12 + Az*ACC13 + ACC10;
	accel.y = Ax*ACC21 + Ay*ACC22 + Az*ACC23 + ACC20;
	accel.z = Ax*ACC31 + Ay*ACC32 + Az*ACC33 + ACC30;
	
	// Double tap moving array code
	if (firstZ){
		for(j=0;j<3;j++){
			movingZ[j] = accel.z;
		}
		firstZ = 0;
		i = 1;
	}
	else{ 
		movingZ[i] = accel.z;
		i++;
		if(i>2) i = 0;
	}
	
}

/**  Calculates pitch angle
   * @brief  Calculates pitch angle
   * @retval Returns pitch angle **/
float calc_pitch_angle(void){
	return 90.0 - DEGREES(atan(accel.y/sqrt((accel.x*accel.x)+(accel.z*accel.z))));

}

/**  Calculates roll angle
   * @brief  Calculates roll angle
   * @retval Returns roll angle **/
float calc_roll_angle(void){	
	return 90.0 - DEGREES(atan(accel.x/sqrt((accel.y*accel.y)+(accel.z*accel.z))));
	
}

/**  Calculates yaw angle
   * @brief  Calculates yaw angle
   * @retval Returns yaw angle **/
float calc_yaw_angle(void){
	return 90.0 - DEGREES(acos(accel.z/1000.0));
	
}

/**  Initialize accelerometer
   * @brief  Initializes accelerometer for use **/
void Accelerometer_config(void) {
	
	//Configure LIS3DSH accelermoter sensor
	LIS3DSH_InitTypeDef init;
	LIS3DSH_DRYInterruptConfigTypeDef init_it;
	GPIO_InitTypeDef GPIO_InitE;
	
	init.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;  	// Active mode with data rate 100HZ
	init.Axes_Enable = LIS3DSH_XYZ_ENABLE; 										// Enable all axes
	init.Continous_Update = LIS3DSH_ContinousUpdate_Enabled;  // Enable continuous update
	init.AA_Filter_BW = LIS3DSH_AA_BW_50; 										// TODO: Not sure about this one, BW = ODR/2
	init.Full_Scale = LIS3DSH_FULLSCALE_2; 										// Use full scale range of 2g
	init.Self_Test = LIS3DSH_SELFTEST_NORMAL; 								// Keep this set to normal
	LIS3DSH_Init(&init);

	// Accelerometer interrupt configuration
  init_it.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;
  init_it.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;                  
  init_it.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;      
	
	/* Set PE0 as the interupt for the accelorometer
	   Enable clocks for ports E */
	__HAL_RCC_GPIOE_CLK_ENABLE();

	// Give initialization values for GPIO E pin sets
	GPIO_InitE.Pin = GPIO_PIN_0 ;
	GPIO_InitE.Mode = GPIO_MODE_IT_RISING; 
	GPIO_InitE.Pull = GPIO_PULLDOWN;
	GPIO_InitE.Speed =  GPIO_SPEED_FREQ_HIGH;
	
	// Initialize GPIO PE0
	HAL_GPIO_Init(GPIOE, &GPIO_InitE);	
	
	// Enable external interrupt line 0
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	LIS3DSH_DataReadyInterruptConfig(&init_it); 
	
	// Initialize accelerometer mutexs
	tiltAnglesMutex = osMutexCreate(tiltAnglesMutexPtr);
	
	//initialize kalman filters for the accelerometer
	config_accelerometer_kalman();

}