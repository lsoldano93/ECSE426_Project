#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "global_vars.h" 
#include "lis3dsh.h"
#include "math.h"

#define PI 3.14159265358
#define ACC11 0.9523
#define ACC12 -0.0019
#define ACC13 -0.0079
#define ACC10 -0.0297
#define ACC21 -0.0397
#define ACC22 0.9896
#define ACC23 -0.0105
#define ACC20 0.0070
#define ACC31 -0.0047
#define ACC32 -0.0024
#define ACC33 1.0087
#define ACC30 -0.0328 
#define DEGREES(x) (180.0*x/PI)

// TODO: Determine these values
#define accelKalman_x_q 0.1
#define accelKalman_x_r 2.0
#define accelKalman_x_x 25.0
#define accelKalman_x_p 0.0
#define accelKalman_x_k 0.0

// TODO: Determine these values
#define accelKalman_y_q 0.1
#define accelKalman_y_r 2.0
#define accelKalman_y_x 25.0
#define accelKalman_y_p 0.0
#define accelKalman_y_k 0.0

// TODO: Determine these values
#define accelKalman_z_q 0.1
#define accelKalman_z_r 2.0
#define accelKalman_z_x 25.0
#define accelKalman_z_p 0.0
#define accelKalman_z_k 0.0

/* Private typedef -----------------------------------------------------------*/

typedef struct {
	float x; 
	float y; 
	float z;
} accelerometer_values;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/ 

/**  Initiates accelerometer thread
   * @brief  Builds thread and starts it
	 * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_Accelerometer (void);

/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void Thread_Accelerometer (void const *argument);

/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void accelerometer_mode(void);

/**  Configure kalman filtering structures for x,y,z values of accelerometer
   * @brief  Gives pre-defined values to new kalman structs  **/
void config_accelerometer_kalman(void);

/**  Accelerometer bread and butter
   * @brief  Updates x, y, z parameters of accelerometer by reading from MEMs device
	 * @param  Locations where updated values will be stored **/
void update_accel_values(float Ax, float Ay, float Az);

/**  Calculates pitch angle
   * @brief  Calculates pitch angle
   * @retval Returns pitch angle **/
float calc_pitch_angle(void);

/**  Calculates roll angle
   * @brief  Calculates roll angle
   * @retval Returns roll angle **/
float calc_roll_angle(void);

/**  Calculates yaw angle
   * @brief  Calculates yaw angle
   * @retval Returns yaw angle **/
float calc_yaw_angle(void);

/**  Initialize accelerometer
   * @brief  Initializes accelerometer for use **/
void Accelerometer_config(void);



#endif