/**
  ******************************************************************************
  * @file    main.c 
  * @author  CL
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   This application contains an example which shows how implementing
  *          a proprietary Bluetooth Low Energy profile: the sensor profile.
  *          The communication is done using a Nucleo board and a Smartphone
  *          with BTLE.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/


#include "cube_hal.h"

#include "osal.h"
#include "sensor_service.h"
#include "debug.h"
#include "stm32_bluenrg_ble.h"
#include "bluenrg_utils.h"
#include "DopeComs.h"

#include <string.h>
#include <stdio.h>

/** @addtogroup X-CUBE-BLE1_Applications
 *  @{
 */

/** @defgroup SensorDemo
 *  @{
 */

/** @defgroup MAIN 
 * @{
 */

/** @defgroup MAIN_Private_Defines 
 * @{
 */
/* Private defines -----------------------------------------------------------*/
#define BDADDR_SIZE 6

/**
 * @}
 */
 
/* Private macros ------------------------------------------------------------*/

/** @defgroup MAIN_Private_Variables
 * @{
 */
/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t set_connectable;
extern volatile int connected;

uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */
uint8_t doubleTap_flag = 0;
uint16_t ledState = 0; 
float temperature, pitch, roll;
/**
 * @}
 */

/** @defgroup MAIN_Private_Function_Prototypes
 * @{
 */
/* Private function prototypes -----------------------------------------------*/
void User_Process();
/**
 * @}
 */

/**
 * @brief  Main function to show how to use the BlueNRG Bluetooth Low Energy
 *         expansion board to send data from a Nucleo board to a smartphone
 *         with the support BLE and the "BlueNRG" app freely available on both
 *         GooglePlay and iTunes.
 *         The URL to the iTunes for the "BlueNRG" app is
 *         http://itunes.apple.com/app/bluenrg/id705873549?uo=5
 *         The URL to the GooglePlay is
 *         https://play.google.com/store/apps/details?id=com.st.bluenrg
 *         The source code of the "BlueNRG" app, both for iOS and Android, is
 *         freely downloadable from the developer website at
 *         http://software.g-maps.it/
 *         The board will act as Server-Peripheral.
 *
 *         After connection has been established:
 *          - by pressing the USER button on the board, the cube showed by
 *            the app on the smartphone will rotate.
 *          
 *         The communication is done using a vendor specific profile.
 *
 * @param  None
 * @retval None
 */
 
 //TODO: DELETE THIS
int debug = 0;
int main(void)
{
  const char *name = "BTLE_G4";
  uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};
  uint8_t bdaddr[BDADDR_SIZE];
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	
	uint16_t NumByteToWrite;
  
  uint8_t  hwVersion;
  uint16_t fwVersion;
	
	
	float returnArray[4];
  
  int ret;  
  
  /* STM32Cube HAL library initialization:
   *  - Configure the Flash prefetch, Flash preread and Buffer caches
   *  - Systick timer is configured by default as source of time base, but user 
   *    can eventually implement his proper time base source (a general purpose 
   *    timer for example or other time source), keeping in mind that Time base 
   *    duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
   *    handled in milliseconds basis.
   *  - Low Level Initialization
   */
  HAL_Init();
  
 
  /* Configure the system clock */
	/* SYSTEM CLOCK = 32 MHz */
  SystemClock_Config();

	/*Configure Dope Coms*/
	DopeComs_Config();

	/* Configure Nucleo for SPI communications with Discovery */
  //NucleoSPI_Config();

  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* Initialize the BlueNRG HCI */
  HCI_Init();

  /* Reset BlueNRG hardware */
  BlueNRG_RST();
    
  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);

  /* 
   * Reset BlueNRG again otherwise we won't
   * be able to change its MAC address.
   * aci_hal_write_config_data() must be the first
   * command after reset otherwise it will fail.
   */
  BlueNRG_RST();
  
  PRINTF("HWver %d, FWver %d", hwVersion, fwVersion);
	PRINTF("\n\n");
  
  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    bnrg_expansion_board = IDB05A1; 
    /*
     * Change the MAC address to avoid issues with Android cache:
     * if different boards have the same MAC address, Android
     * applications unless you restart Bluetooth on tablet/phone
     */
    SERVER_BDADDR[5] = 0x02;
  }

  /* The Nucleo board must be configured as SERVER */
  Osal_MemCpy(bdaddr, SERVER_BDADDR, sizeof(SERVER_BDADDR));
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  CONFIG_DATA_PUBADDR_LEN,
                                  bdaddr);
  if(ret){
    PRINTF("Setting BD_ADDR failed.\n");
  }
  
  ret = aci_gatt_init();    
  if(ret){
    PRINTF("GATT_Init failed.\n");
  }

  if (bnrg_expansion_board == IDB05A1) {
    ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x03, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  else {
    ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }

  if(ret != BLE_STATUS_SUCCESS){
    PRINTF("GAP_Init failed.\n");
  }

  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
                                   strlen(name), (uint8_t *)name);

  if(ret){
    PRINTF("aci_gatt_update_char_value failed.\n");            
    while(1);
  }
  
  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     123456,
                                     BONDING);
  if (ret == BLE_STATUS_SUCCESS) {
    PRINTF("BLE Stack Initialized.\n");
  }
  
  PRINTF("SERVER: BLE Stack Initialized\n");
  
  ret = Add_Sensor_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Sensor service added successfully.\n");
  else
    PRINTF("Error while adding Sensor service.\n");
  
  ret = Add_Led_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("LED service added successfully.\n");
  else
    PRINTF("Error while adding LED service.\n");

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);
  

  while(1){

//		if(debug == 1){
//			HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET) printf("Discovery to Nucleo HS reading high\n");
//			
//			HAL_GPIO_WritePin(NUCLEO_DATAo0_GPIO_PORT, NUCLEO_DATAo0_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi0_GPIO_PORT, NUCLEO_DATAi0_PIN) == GPIO_PIN_SET) printf("Input zero is reading high\n");
//			
//			HAL_GPIO_WritePin(NUCLEO_DATAo1_GPIO_PORT, NUCLEO_DATAo1_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi1_GPIO_PORT, NUCLEO_DATAi1_PIN) == GPIO_PIN_SET) printf("Input one is reading high\n");
//			
//			HAL_GPIO_WritePin(NUCLEO_DATAo2_GPIO_PORT, NUCLEO_DATAo2_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi2_GPIO_PORT, NUCLEO_DATAi2_PIN) == GPIO_PIN_SET) printf("Input two is reading high\n");
//			
//			HAL_GPIO_WritePin(NUCLEO_DATAo3_GPIO_PORT, NUCLEO_DATAo3_PIN, GPIO_PIN_SET);
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi3_GPIO_PORT, NUCLEO_DATAi3_PIN) == GPIO_PIN_SET) printf("Input three is reading high\n");
//			
//			Reset_DataLines();
//			debug = 0;
//		}
//		else if(debug == 2){
////			HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(NUCLEO_DATAo0_GPIO_PORT, NUCLEO_DATAo0_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(NUCLEO_DATAo1_GPIO_PORT, NUCLEO_DATAo1_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(NUCLEO_DATAo2_GPIO_PORT, NUCLEO_DATAo2_PIN, GPIO_PIN_SET);
////			HAL_GPIO_WritePin(NUCLEO_DATAo3_GPIO_PORT, NUCLEO_DATAo3_PIN, GPIO_PIN_SET);
//			
//			Reset_DataLines();
//			HAL_GPIO_WritePin(NUCLEO_TO_DISCOVERY_GPIO_PORT, NUCLEO_TO_DISCOVERY_PIN, GPIO_PIN_RESET);
//			
//			if(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) == GPIO_PIN_SET) printf("Discovery to Nucleo HS reading high\n");
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi0_GPIO_PORT, NUCLEO_DATAi0_PIN) == GPIO_PIN_SET) printf("Input zero is reading high\n");
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi1_GPIO_PORT, NUCLEO_DATAi1_PIN) == GPIO_PIN_SET) printf("Input one is reading high\n");
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi2_GPIO_PORT, NUCLEO_DATAi2_PIN) == GPIO_PIN_SET) printf("Input two is reading high\n");
//			if(HAL_GPIO_ReadPin(NUCLEO_DATAi3_GPIO_PORT, NUCLEO_DATAi3_PIN) == GPIO_PIN_SET) printf("Input three is reading high\n");
//			
//			Reset_DataLines();
//			debug = 0;
//		}
		
		if(HAL_GPIO_ReadPin(DISCOVERY_TO_NUCLEO_GPIO_PORT, DISCOVERY_TO_NUCLEO_PIN) ==  GPIO_PIN_SET){
			// Currently just attempts to read temperature
			Master_Communication(ledState, returnArray);
			temperature = returnArray[0];
			pitch = returnArray[1];
			roll = returnArray[2];
			if(((int)returnArray[3]) == 1);// {doubleTap_flag = 1; printf("BRAPPPP\n");}
//			printf("Temperature value: %f\n", returnArray[0]);
//			printf("Pitch value: %f\n", returnArray[1]);
//			printf("Roll value: %f\n", returnArray[2]);
		}


// Uncomment this for BT functionality		
		HCI_Process();
    User_Process();
		
  }
}


/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send the updated acceleration data to the remote client.
 *
 * @param  AxesRaw_t* p_axes
 * @retval None
 */


void User_Process()
{
  if(set_connectable){
    setConnectable();
    set_connectable = FALSE;
  }  

  if(set_connectable){
		setConnectable();
		set_connectable = FALSE;
	}
    
   if(connected)
   {
			Temp_Update((uint16_t)(temperature * 100));
			Pitch_Update((uint16_t)(pitch * 100));
			Roll_Update((uint16_t)(roll * 100));
			//Dtap_Update();
		  //doubleTap_flag = 0;
	 }
}

/**
 * @brief  EXTI line detection callback.
 * @param  Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  
	if (GPIO_Pin == GPIO_PIN_0) HCI_Isr();
//	else if (GPIO_Pin == GPIO_PIN_4){
//		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
//		TEMPERATURE_FLAG = 1;
//	}
//	else if (GPIO_Pin == GPIO_PIN_2){ 
//		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
//		ACCELEROMETER_FLAG = 1;
//	}
//	else if (GPIO_Pin == GPIO_PIN_3){ 
//		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
//		LEDSTATE_FLAG = 1;
//	}
	
}

/**
 * @brief  This function is used for low level initialization of the SPI 
 *         communication with the BlueNRG Expansion Board.
 * @param  hspi: SPI handle.
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;
		
  if(hspi->Instance==BNRG_SPI_INSTANCE)
{
    /* Enable peripherals clock */

    /* Enable GPIO Ports Clock */  
    BNRG_SPI_RESET_CLK_ENABLE();
    BNRG_SPI_SCLK_CLK_ENABLE();
    BNRG_SPI_MISO_CLK_ENABLE();
    BNRG_SPI_MOSI_CLK_ENABLE();
    BNRG_SPI_CS_CLK_ENABLE();
    BNRG_SPI_IRQ_CLK_ENABLE();

    /* Enable SPI clock */
    BNRG_SPI_CLK_ENABLE();

    /* Reset */
    GPIO_InitStruct.Pin = BNRG_SPI_RESET_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_RESET_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_RESET_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_RESET_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_RESET_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_RESET_PORT, &GPIO_InitStruct);	
    HAL_GPIO_WritePin(BNRG_SPI_RESET_PORT, BNRG_SPI_RESET_PIN, GPIO_PIN_RESET);	/*Added to avoid spurious interrupt from the BlueNRG */

    /* SCLK */
    GPIO_InitStruct.Pin = BNRG_SPI_SCLK_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_SCLK_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_SCLK_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_SCLK_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_SCLK_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_SCLK_PORT, &GPIO_InitStruct); 

    /* MISO */
    GPIO_InitStruct.Pin = BNRG_SPI_MISO_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_MISO_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_MISO_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_MISO_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_MISO_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_MISO_PORT, &GPIO_InitStruct);

    /* MOSI */
    GPIO_InitStruct.Pin = BNRG_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_MOSI_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_MOSI_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_MOSI_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_MOSI_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_MOSI_PORT, &GPIO_InitStruct);

    /* NSS/CSN/CS */
    GPIO_InitStruct.Pin = BNRG_SPI_CS_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_CS_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_CS_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_CS_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_CS_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_CS_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(BNRG_SPI_CS_PORT, BNRG_SPI_CS_PIN, GPIO_PIN_SET);

    /* IRQ -- INPUT */
    GPIO_InitStruct.Pin = BNRG_SPI_IRQ_PIN;
    GPIO_InitStruct.Mode = BNRG_SPI_IRQ_MODE;
    GPIO_InitStruct.Pull = BNRG_SPI_IRQ_PULL;
    GPIO_InitStruct.Speed = BNRG_SPI_IRQ_SPEED;
    GPIO_InitStruct.Alternate = BNRG_SPI_IRQ_ALTERNATE;
    HAL_GPIO_Init(BNRG_SPI_IRQ_PORT, &GPIO_InitStruct);

    /* Configure the NVIC for SPI */  
    HAL_NVIC_SetPriority(BNRG_SPI_EXTI_IRQn, 3, 0);    
    HAL_NVIC_EnableIRQ(BNRG_SPI_EXTI_IRQn);
  }

	return;
}

