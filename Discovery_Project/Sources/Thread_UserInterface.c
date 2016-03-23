
/* Includes ------------------------------------------------------------------*/
#include "Thread_UserInterface.h"


/* Private variables ---------------------------------------------------------*/
const int maxTemp = 25.0;

// Pin maps for keypad columns and rows
const uint16_t col_pinmap[3] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10};
const uint16_t row_pinmap[4] = {GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_6, GPIO_PIN_7};

// 11 indicates '*' and 12 indicates '#'
const uint8_t keypad_map[4][3] = {
	{1, 2, 3},
	{4, 5, 6},
	{7, 8, 9},
	{11, 0, 12}};

osThreadId tid_Thread_UserInterface; 
GPIO_InitTypeDef GPIO_row, GPIO_col;
	
uint8_t key_state;
uint8_t tilt_state;   

osThreadDef(Thread_UserInterface, osPriorityBelowNormal, 1, NULL);  // TODO: See if below normal priority helps with constant calling

/* Private functions ---------------------------------------------------------*/
	
/**  Initiates user interface thread
   * @brief  Builds thread and starts it
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_UserInterface (void) {

  tid_Thread_UserInterface = osThreadCreate(osThread(Thread_UserInterface ), NULL); 
  if (!tid_Thread_UserInterface){
		printf("Error starting user interface thread!");
		return(-1); 
	}
  return(0);
}

/**  User interface thread 
   * @brief  Runs UI thread which reads key presses and updates the display accordingly
   */
void Thread_UserInterface (void const *argument){

	float tempValue = 0.0;
	float rollAngle = 0.0;
	float pitchAngle = 0.0;
	int counter = 0;
	
	/* Initialize key and tilt states
		 Key States = {0:Temperature, 1:Tilt Angles}
		 States = {0:Roll, 1:Pitch}                    */
	key_state = 0;
	tilt_state = 0;
	
	while(1){
		
		osDelay(UI_THREAD_OSDELAY);
		
		// Read keys
		handleKeyPress();
		
		// temperature alarm goes off and blinks segment display
		if(temperatureValue >= maxTemp) {
				counter++;
				if(counter >= 30) {
					counter = 0;
					resetPins();
					Delay(200);					
				}				
				
		}

		/* If key state is 0 then get access to temperature variable
		   before drawing it to the display */
		if (key_state == 0){
			osMutexWait(temperatureMutex, (uint32_t) THREAD_TIMEOUT);
			tempValue = temperatureValue;			
			osMutexRelease(temperatureMutex);
			counter++;
			drawTemperature(tempValue);
			
		}
		/* Key state must be one, so see which tilt value to display and gain access
		   before drawing it to the display */
		else {
			if (tilt_state == 0){
				osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
				rollAngle = rollValue;
				osMutexRelease(tiltAnglesMutex);
				drawAngle(rollAngle);
			}
			else if (tilt_state == 1){
				osMutexWait(tiltAnglesMutex, (uint32_t) THREAD_TIMEOUT);
				pitchAngle = pitchValue;
				osMutexRelease(tiltAnglesMutex);
				drawAngle(pitchAngle);
			}
			
		}                                       
	}
	
}


/**  Handle key presses
   * @brief Pressing 0 toggles temperature/accelerometer; pressing 1/2 toggles tilt angles
   */
void handleKeyPress(void) {

	int key; 
	
	if ((key = getKey()) == -1) return;
	if (key == 0 && key_state == 0) key_state = 1;
	else if (key == 0 && key_state == 1) key_state = 0;
	else if (key_state == 1 && key == 1) tilt_state = 0;
	else if (key_state == 1 && key == 3) tilt_state = 1;
		
	return;
	
}


/**  Causes the display of a temperature value
   * @brief  Draws temperature on display
	 * @param  Float of temperature to be displayed **/
void drawTemperature(float temp){
	
	int displayValues, i;
	int tempValue[4];
	
	// Determine how many values to display and what each individual value is
	// Four digits to display (format xyz.a)
	if (temp >= 100.0){
		displayValues = 4;
		tempValue[0] = temp/100;
		tempValue[1] = temp/10 - tempValue[0]*10;
		tempValue[2] = temp - tempValue[0]*100 - tempValue[1]*10;
		tempValue[3] = (temp*10 - tempValue[0]*1000 - tempValue[1]*100 - tempValue[2]*10);
	
	}
	else {
		
		// Three digits to display (format xy.z)
		if (temp >= 10.0){
				displayValues = 3;
				tempValue[1] = temp/10;
				tempValue[2] = temp - tempValue[1]*10;
				tempValue[3] = (temp*10 - tempValue[1]*100 - tempValue[2]*10);	
		}
		
		// Two digits to display (format x.y or 0.x)
		else {
				displayValues = 2;
				tempValue[2] = temp;
				tempValue[3] = (temp*10 - tempValue[2]*10);
		}
	}
	
	for(i=4; i>0; i--) {
		
		selectDigit(i);
		//resetPins();
		 
		// Light proper number on display, lighting 10 indicates blank display value
		if (i > displayValues) {
			lightNum(10);
		}
		else {
			// Set decimal point on third value
			if (i == 2) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
			
			lightNum(tempValue[4-i]);	
			
		}
		
		Delay(DISPLAY_DELAY); 
		
	}
	
}	


/**  Causes the display of an angle value
   * @brief  Draws angle on display
	 * @param  Float of angle to be displayed **/
void drawAngle(float angle) {
	
	int i, decimalPos;
	int displayValues = 3;
	int angleValue[3];
	
	//7-segment display in the form of XXX°, XX.Y °or X.YY depending on the actual angle(i.e. 119°, 59.3°, 7.55°)
	//XXX°
	if (angle >= 100.0){
		
		// Don't want to light up decimal in this case, give dummy value
		decimalPos = 30;
		angleValue[0] = angle/100;
		angleValue[1] = angle/10 - angleValue[0]*10;
		angleValue[2] = angle - angleValue[0]*100 - angleValue[1]*10;

	}
	
	// Three digits to display (XX.Y°)
	else if(angle >= 10.0) {

		decimalPos = 2;
		angleValue[0] = angle/10;
		angleValue[1] = angle - angleValue[0]*10;
		angleValue[2] = (angle*10 - angleValue[0]*100 - angleValue[1]*10);	
	
	}
	
	//Two digits to display (X.YY°)
	else {
		decimalPos = 3;
		angleValue[0] = angle;
		angleValue[1] = (angle*10 - angleValue[0]*10);
		angleValue[2] = (angle*100 - angleValue[0]*100 - angleValue[1]*10);

	}	
	
	for(i=4; i>0; i--) {
		
		// For display of degree symbol
		if (i == 4 ) {
			
			resetPins();
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
			
		}
		else {
			resetPins();
			if(decimalPos == 30) {
				selectDigit(i+1);
				lightNum(angleValue[3-i]);
			}
			else {
				selectDigit(i);		
				if (i == decimalPos) HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
				else HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);	
				lightNum(angleValue[3-i]);	
			}
					
		}
		
		Delay(DISPLAY_DELAY); 
		
	}
	
}


/**
   * @brief Determines row and column of key press then returns value from map
   * @retval Returns exact key pressed
   */
int getKey(void) {
	
	int i = 0;
	
	uint8_t key;
	uint8_t row, column;
	
	// Interupt handling, and debouncing;
	if((column = getColumn()) == 9) return -1;
	if((row = getRow()) == 9) return -1;
	
	// Hold code in wait state until button released/held long enough
	key = keypad_map[row][column];
	while(1){
		
		// Wait state
		while(keypad_map[getRow()][getColumn()] == key){ 
			i++;
			osDelay(DEBOUNCE_DELAY); 
		}
		
		// Key held long enough and isn't noise
		if(i > 2) break;
		else return getKey();
		
	}

	printf("%d made it dad!\n", key);
	return key;		
	
}


/**
   * @brief Reports column of key that was pressed
   * @retval Int of column activated
   */
uint8_t getColumn(void) {
	
	// Initialize pins to allow for columns as input
	init_columns();
	
	// Row selection determined by active low 
	if(!HAL_GPIO_ReadPin(GPIOD, col_pinmap[0])) return 0;
	else if(!HAL_GPIO_ReadPin(GPIOD, col_pinmap[1])) return 1;
	else if(!HAL_GPIO_ReadPin(GPIOD, col_pinmap[2])) return 2;
	else return 9;
	
}


/**
   * @brief Reports row of key that was pressed
   * @retval Int of row activated
   */
uint8_t getRow(void) {
	
	// Initialize pins to allow for rows as input
	init_rows();
	
	// Row selection determined by active low 
	if(!HAL_GPIO_ReadPin(GPIOD, row_pinmap[3])) return 3;
	else if(!HAL_GPIO_ReadPin(GPIOD, row_pinmap[2])) return 2;
	else if(!HAL_GPIO_ReadPin(GPIOD, row_pinmap[1])) return 0;
	else if(!HAL_GPIO_ReadPin(GPIOD, row_pinmap[0])) return 1;
	
	else return 9;

}


/**  Initialization of pins to allow for row read
   * @brief  Sets row pins as input pullup and column pins as output no pull **/
void init_rows(void) {
	
	//initialize rows
	GPIO_row.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7; 
	
	GPIO_row.Mode = GPIO_MODE_IT_RISING;
	GPIO_row.Pull = GPIO_PULLUP;
	GPIO_row.Speed =  GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_row);
	
	// Initialize columns
	GPIO_col.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 ;
	
	GPIO_col.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_col.Pull = GPIO_NOPULL;
	GPIO_col.Speed =  GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_col);
	
}


/**  Initialization of pins to allow for column read
   * @brief  Sets column pins as input pullup and row pins as output no pull **/
void init_columns(void) {
	
	//initialize rows
	GPIO_row.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7; 
	
	GPIO_row.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_row.Pull = GPIO_NOPULL;
	GPIO_row.Speed =  GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_row);
	
	// Initialize columns
	GPIO_col.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 ;
	
	GPIO_col.Mode = GPIO_MODE_IT_RISING;
	GPIO_col.Pull = GPIO_PULLUP;
	GPIO_col.Speed =  GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_col);
	
}


/**  Selects segment digit to display for
   * @brief  Cycles through cases for each segment of display and enables/disables proper pins
	 * @param  Segment digit to be illuminated **/
void selectDigit(int digit) {
	
	switch(digit) {
		case 4:
			// Enable changes for first digit display
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);   // Digit 1
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // Digit 2
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // Digit 3
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); // Digit 4
			break;
		case 3:
			// Enable changes for second digit display
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); // Digit 1
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);   // Digit 2
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // Digit 3
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); // Digit 4
			break;
		case 2:
			// Enable changes for third digit display
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); // Digit 1
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // Digit 2
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   // Digit 3
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); // Digit 4
			break;
		case 1:
			// Enable changes for fourth digit display
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); // Digit 1
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // Digit 2
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // Digit 3
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);   // Digit 4
			break;

		default:
			break;
	} 
	
}


/**  Lights segment digit selected with proper display
   * @brief  Takes in value to be displayed on preselected 7 segment display
	 * @param  Value to be displayed **/
void lightNum(int num) {
	
	switch(num) {
			case 0:
				// Zeroth state, display number zero
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g
				break;
			
			case 1:
				// First state, display number one
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g
				break;

			case 2:
				// Second state, display number two
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g
				break;
			
			case 3:
				// Third state, display number three
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g
				break;
			
			case 4:
				// Fourth state, display number four
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g			
				break;
			
			case 5:
				// Fifth state, display number five
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g
				break;
			
			case 6:
				// Sixth state, display number six
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g
				break;

			case 7:
				// Seventh state, display number seven
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g
				break;
			
			case 8:
				// Eighth state, display number eight
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g
				break;
			
			case 9:
				// Ninth state, display number nine
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //g			
				break;
			
			case 10:
				// Null state, display blank digit
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //a
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //b
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //c
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g		
				break;
				
			default:
				break;
		}
	
}


/**  Resets all display pins
   * @brief  "" **/
void resetPins() {
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); //a
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); //b
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); //c
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //d
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //e
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //f
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //g
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); // Digit 1
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); // Digit 2
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET); // Digit 3
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET); // Digit 4
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); //degree
	
}


/**  Uses timer 3 to generate delay for display
   * @brief  Allows for software delay to be used for display purposes **/
void Delay(uint32_t time){
	
	timingDelay = time;
  while(timingDelay !=0);
	
}


/**  Configures system for user interface
   * @brief  Initializes pins for display and clock for keypad **/
void UserInterface_config(void){
	
	GPIO_InitTypeDef GPIO_InitB, GPIO_InitC;
	
	/* 7-Segment Display Pinout {1:CCD1, 2:CCD2, 3:D, 4:Degree, 5:E, 6:CCD3, 
	   7:DP, 8:CCD4, 9:, 10:, 11:F, 12:, 13:C, 14:A, 15:G, 16:B}
	
		 Keypad Pinout {1:R1, 2:R2, 4:R4, 5:R5, 6:C1, 7:C2, 8:C3}
	
	   GPIO Pin Mapping for Display
		 Segment Ctrls {CCD1:PC1, CCD2:PC2, CCD3:PC4, CCD4:PC5}
	   DP & DC Ctrl  {DP:PC6, DC:PC7}
		 Segments      {A:PB0, B:PB1, C:PB5, D:PB11, E:PB12, F:PB13, G:PB14} 
	
		 GPIO Pin Mapping for Keypad
		 Rows: 		{R1:PD1, R2:PD2, R3:PD6 , R4:PD7}
		 Columns: {C1:PD8, C2:PD9, C3:PD10}
	
	*/
	
	// Enable clocks for ports B,C,& D 
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	// Give initialization values for GPIO B pin sets
	GPIO_InitB.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
	GPIO_InitB.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitB.Pull = GPIO_PULLUP;
	GPIO_InitB.Speed =  GPIO_SPEED_FREQ_HIGH;
	
	// Give initialization values for GPIO C pin sets
	GPIO_InitC.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitC.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitC.Pull = GPIO_PULLUP;
	GPIO_InitC.Speed =  GPIO_SPEED_FREQ_HIGH;

	// Initialize all GPIO pin sets
	HAL_GPIO_Init(GPIOB, &GPIO_InitB);
	HAL_GPIO_Init(GPIOC, &GPIO_InitC);

}

