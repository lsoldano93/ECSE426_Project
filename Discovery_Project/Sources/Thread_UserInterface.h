#ifndef THREAD_USERINTERFACE_H
#define THREAD_USERINTERFACE_H

/* Includes ------------------------------------------------------------------*/
#include "global_vars.h"
#include "stm32f4xx_hal_gpio.h"

#define UI_THREAD_OSDELAY 10  // Delay in milliseconds
#define DISPLAY_DELAY 1			 // Delay in milliseconds
#define DEBOUNCE_DELAY 25    // Delay in milliseconds

#define ROWS 4
#define COLS 3

	/* Pin mappings
	
		 7-Segment Display Pinout {1:CCD1, 2:CCD2, 3:D, 4:Degree, 5:E, 6:CCD3, 7:DP,
		                          8:CCD4, 9:, 10:, 11:F, 12:, 13:C, 14:A, 15:G, 16:B}
	
		 Keypad Pinout {1:R1, 2:R2, 4:R4, 5:R5, 6:C1, 7:C2, 8:C3}
	
	   GPIO Pin Mapping for Display
		 Segment Ctrls {CCD1:PC1, CCD2:PC2, CCD3:PC4, CCD4:PC5}
	   DP & DC Ctrl  {DP:PC6, DC:PC7}
		 Segments      {A:PB0, B:PB1, C:PB5, D:PB11, E:PB12, F:PB13, G:PB14} 
	
		 GPIO Pin Mapping for Keypad
		 Rows: 		{R1:PD1, R2:PD2, R3:PD6 , R4:PD7}
		 Columns: {C1:PD8, C2:PD9, C3:PD10}
	
	*/

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

/**  Handle key presses
   * @brief Pressing 0 toggles temperature/accelerometer; pressing 1/2 toggles tilt angles
   */
void handleKeyPress(void);

/**  Causes the display of a temperature value
   * @brief  Draws temperature on display
	 * @param  Float of temperature to be displayed **/
void drawTemperature(float temp);

/**  Causes the display of an angle value
   * @brief  Draws angle on display
	 * @param  Float of angle to be displayed **/
void drawAngle(float angle);

/**
   * @brief Determines row and column of key press then returns value from map
   * @retval Returns exact key pressed **/
int getKey(void);

/**
   * @brief Reports column of key that was pressed
   * @retval Int of column activated **/
uint8_t getColumn(void);

/**
   * @brief Reports row of key that was pressed
   * @retval Int of row activated **/
uint8_t getRow(void);

/**  Initialization of pins to allow for row read
   * @brief  Sets row pins as input pullup and column pins as output no pull **/
void init_rows(void);

/**  Initialization of pins to allow for column read
   * @brief  Sets column pins as input pullup and row pins as output no pull **/
void init_columns(void);

/**  Selects segment digit to display for
   * @brief  Cycles through cases for each segment of display and enables/disables proper pins
	 * @param  Segment digit to be illuminated **/
void selectDigit(int digit);

/**  Lights segment digit selected with proper display
   * @brief  Takes in value to be displayed on preselected 7 segment display
	 * @param  Value to be displayed **/
void lightNum(int num);

/**  Resets all display pins
   * @brief  "" **/
void resetPins();

/**  Configures system for user interface
   * @brief  Initializes pins for display and clock for keypad **/
void UserInterface_config(void);

/**  Uses timer 3 to generate delay for display
   * @brief  Allows for software delay to be used for display purposes **/
void Delay(uint32_t time);

#endif