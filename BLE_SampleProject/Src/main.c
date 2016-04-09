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
#include "NucleoSPI.h"

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
#define SPI_ACK_BYTES             0xA5A5
#define SPI_NACK_BYTES            0xDEAD
#define SPI_TIMEOUT_MAX           0x1000
#define SPI_SLAVE_SYNBYTE         0x53
#define SPI_MASTER_SYNBYTE        0xAC

/* Defines used for tranfer communication*/
#define ADDRCMD_MASTER_READ                         ((uint16_t)0x1234)
#define ADDRCMD_MASTER_WRITE                        ((uint16_t)0x5678)
#define CMD_LENGTH                                  ((uint16_t)0x0004)
#define DATA_LENGTH                                 ((uint16_t)0x0020)

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
extern AxesRaw_t axes_data;
uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */
uint8_t DISCOVERY_SPI_FLAG = 0;
SPI_HandleTypeDef SpiHandleDiscovery;

FlagStatus TestReady = RESET;

/* Buffer used for transmission */
uint8_t aTxMasterBuffer[] = "SPI - MASTER - Transmit message";
uint8_t aTxSlaveBuffer[]  = "SPI - SLAVE - Transmit message ";
/* Buffer used for reception */
uint8_t aRxBuffer[DATA_LENGTH];

/* Private function prototypes -----------------------------------------------*/
static void Master_Synchro(void);
static void Error_Handler(void);
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);

/**
 * @}
 */

/** @defgroup MAIN_Private_Function_Prototypes
 * @{
 */
/* Private function prototypes -----------------------------------------------*/
void User_Process(AxesRaw_t* p_axes);
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
int main(void)
{
  const char *name = "BTLE_G4";
  uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};
  uint8_t bdaddr[BDADDR_SIZE];
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
	
  
  uint8_t  hwVersion;
  uint16_t fwVersion;
  
  int ret;  
  
  uint8_t addrcmd[CMD_LENGTH] = {0};
  uint16_t ackbytes = 0x0000;
  
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
  
#if NEW_SERVICES
  /* Configure LED2 */
  BSP_LED_Init(LED2); 
#endif
  
  /* Configure the User Button in GPIO Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  
  /* Configure the system clock */
	/* SYSTEM CLOCK = 32 MHz */
  SystemClock_Config();

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
  
  ret = Add_Acc_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Acc service added successfully.\n");
  else
    PRINTF("Error while adding Acc service.\n");
  
  ret = Add_Environmental_Sensor_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Environmental Sensor service added successfully.\n");
  else
    PRINTF("Error while adding Environmental Sensor service.\n");

#if NEW_SERVICES
  /* Instantiate Timer Service with two characteristics:
   * - seconds characteristic (Readable only)
   * - minutes characteristics (Readable and Notifiable )
   */
  ret = Add_Time_Service(); 
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Time service added successfully.\n");
  else
    PRINTF("Error while adding Time service.\n");  
  
  /* Instantiate LED Button Service with one characteristic:
   * - LED characteristic (Readable and Writable)
   */  
  ret = Add_LED_Service();

  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("LED service added successfully.\n");
  else
    PRINTF("Error while adding LED service.\n");  
#endif

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);


  	GPIO_InitTypeDef GPIO_InitStructure;

  /* SPI configuration -------------------------------------------------------*/
	/* Enable the SPI periph */
  __HAL_RCC_SPI2_CLK_ENABLE();
	
  HAL_SPI_DeInit(&SpiHandleDiscovery);
  SpiHandleDiscovery.Instance 							= SPI2;
  SpiHandleDiscovery.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  SpiHandleDiscovery.Init.Direction 				= SPI_DIRECTION_2LINES; // set full duplex communication
  SpiHandleDiscovery.Init.CLKPhase 					= SPI_PHASE_2EDGE;
  SpiHandleDiscovery.Init.CLKPolarity 			= SPI_POLARITY_LOW;
  SpiHandleDiscovery.Init.CRCCalculation		= SPI_CRCCALCULATION_DISABLED;
  SpiHandleDiscovery.Init.CRCPolynomial 		= 7;
  SpiHandleDiscovery.Init.DataSize 					= SPI_DATASIZE_8BIT;
  SpiHandleDiscovery.Init.FirstBit 					= SPI_FIRSTBIT_MSB;
  SpiHandleDiscovery.Init.NSS 							= SPI_NSS_SOFT;
  SpiHandleDiscovery.Init.TIMode 						= SPI_TIMODE_DISABLED;
  SpiHandleDiscovery.Init.Mode 							= SPI_MODE_MASTER;
		
	if (HAL_SPI_Init(&SpiHandleDiscovery) != HAL_OK) {printf ("ERROR: Error in initialising SPIDiscovery \n");};
  
	__HAL_SPI_ENABLE(&SpiHandleDiscovery);
  
  while(1){
		
     /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Recieve Data from the Slave ###########################################*/ 
    addrcmd[0] = (uint8_t) (ADDRCMD_MASTER_READ >> 8);
    addrcmd[1] = (uint8_t) ADDRCMD_MASTER_READ;
    addrcmd[2] = (uint8_t) (DATA_LENGTH >> 8);
    addrcmd[3] = (uint8_t) DATA_LENGTH;
    /* Send Master READ command to slave */
    if(HAL_SPI_Transmit_IT(&SpiHandleDiscovery, addrcmd, CMD_LENGTH) != HAL_OK)
    {
      Error_Handler();
    }
     /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it’s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
    {}
    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Receive ACK from the Slave */
    ackbytes = 0;
    if(HAL_SPI_Receive_IT(&SpiHandleDiscovery, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
    {}
    /* Check the received ACK */
    if(ackbytes == SPI_ACK_BYTES)
    {
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Receive the requested data from the slave */
      if(HAL_SPI_Receive_IT(&SpiHandleDiscovery, aRxBuffer, DATA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
      {}
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Send ACK to the Slave */
      ackbytes = SPI_ACK_BYTES;
      if(HAL_SPI_Transmit_IT(&SpiHandleDiscovery, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
      {}
    }    
    else
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
    
    /* Compare received buffer with one expected from slave */
    if(Buffercmp((uint8_t*)aTxSlaveBuffer, (uint8_t*)aRxBuffer, CMD_LENGTH))
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
    else
    {
      /* Turn LED6 on: Reception is correct */
    
    }

    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Transmit Data To Slave ################################################*/
    addrcmd[0] = (uint8_t) (ADDRCMD_MASTER_WRITE >> 8);
    addrcmd[1] = (uint8_t) ADDRCMD_MASTER_WRITE;
    addrcmd[2] = (uint8_t) (DATA_LENGTH >> 8);
    addrcmd[3] = (uint8_t) DATA_LENGTH;
    /* Send Master WRITE command to the slave */
    if(HAL_SPI_Transmit_IT(&SpiHandleDiscovery, addrcmd, CMD_LENGTH) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
    {}
    /* Synchronization between Master and Slave */
    Master_Synchro();
    
    /* Receive ACK from the Slave */
    ackbytes = 0;
    if(HAL_SPI_Receive_IT(&SpiHandleDiscovery, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
    {}
    /* Check the received ACK */
    if(ackbytes == SPI_ACK_BYTES)
    {
      /* Synchronization between Master and Slave */
      Master_Synchro();
      /* Send the requested data from the slave */
      if(HAL_SPI_Transmit_IT(&SpiHandleDiscovery, aTxMasterBuffer, DATA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
      {}
      /* Synchronization between Master and Slave */
      Master_Synchro();
      
      /* Receive ACK from the Slave */
      ackbytes = 0;
      if(HAL_SPI_Receive_IT(&SpiHandleDiscovery, (uint8_t *)&ackbytes, sizeof(ackbytes)) != HAL_OK)
      {
        Error_Handler();
      }
      while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
      {}
    }    
    else
    {
      /* Transfer error in transmission process */
      Error_Handler();
    }
   
    /* Flush Rx buffer for next transmission */
    Flush_Buffer(aRxBuffer, DATA_LENGTH);
    
    /* Toggle LED4 */
 
    
    /* This delay permit to user to see LED4 toggling*/
    HAL_Delay(100);

// Uncomment this for BT functionality		
    HCI_Process();
    User_Process(&axes_data);
#if NEW_SERVICES
    Update_Time_Characteristics();
#endif
			
  }
}


/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send the updated acceleration data to the remote client.
 *
 * @param  AxesRaw_t* p_axes
 * @retval None
 */
void User_Process(AxesRaw_t* p_axes)
{
  if(set_connectable){
    setConnectable();
    set_connectable = FALSE;
  }  

  /* Check if the user has pushed the button */
  if(BSP_PB_GetState(BUTTON_KEY) == RESET)
  {
    while (BSP_PB_GetState(BUTTON_KEY) == RESET);
    
    //BSP_LED_Toggle(LED2); //used for debugging (BSP_LED_Init() above must be also enabled)
    
    if(connected)
    {
      /* Update acceleration data */
      p_axes->AXIS_X += 1;
      p_axes->AXIS_Y -= 1;
      p_axes->AXIS_Z += 2;
      //PRINTF("ACC: X=%6d Y=%6d Z=%6d\r\n", p_axes->AXIS_X, p_axes->AXIS_Y, p_axes->AXIS_Z);
      Acc_Update(p_axes);
    }
  }
}

/**
 * @brief  EXTI line detection callback.
 * @param  Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  
	if (GPIO_Pin == GPIO_PIN_0) HCI_Isr();
	else if (GPIO_Pin == KEY_BUTTON_PIN) {
     TestReady = SET;
  } 
	
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
	
	if (hspi->Instance == SPI2){
		
		/* Enable proper clock lines */
    __SPI2_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();
		
    
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = NUCLEO_SPI_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      
    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Pin       = NUCLEO_SPI_MISO_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin       = NUCLEO_SPI_MOSI_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*##-3- Enable SPI peripheral Clock ########################################*/
    __SPI2_CLK_ENABLE();

    /*##-4- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
  
   
		
	}
  else if(hspi->Instance==BNRG_SPI_INSTANCE)
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
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
  
  if (hspi->Instance == SPI2){

    /*##-1- Reset peripherals ##################################################*/
    __SPI2_FORCE_RESET();
    __SPI2_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Configure SPI SCK as alternate function  */
    HAL_GPIO_DeInit(GPIOB, NUCLEO_SPI_SCK_PIN);
    /* Configure SPI MISO as alternate function  */
    HAL_GPIO_DeInit(GPIOB, NUCLEO_SPI_MISO_PIN);
    /* Configure SPI MOSI as alternate function  */
    HAL_GPIO_DeInit(GPIOB, NUCLEO_SPI_MOSI_PIN);

    /*##-3- Disable the NVIC for SPI ###########################################*/
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
  }
}

static void Error_Handler(void)
{
  /* Turn LED5 (RED) on */
 // BSP_LED_On(LED5);
  printf("in error_handler\n");
  while(1) {
  }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if((*pBuffer1) != *pBuffer2)
    {
      return BufferLength;
    }
    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}

/**
  * @brief  Flushes the buffer
  * @param  pBuffer: buffers to be flushed.
  * @param  BufferLength: buffer's length
  * @retval None
  */
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    *pBuffer = 0;

    pBuffer++;
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @brief Master Synchronization with Slave.
  * @param None
  * @retval None
  */
static void Master_Synchro(void)
{
  uint8_t txackbytes = SPI_MASTER_SYNBYTE, rxackbytes = 0x00;
  do
  {
    /* Call SPI write function to send command to slave */
    if(HAL_SPI_TransmitReceive_IT(&SpiHandleDiscovery, (uint8_t *)&txackbytes, (uint8_t *)&rxackbytes, 1) != HAL_OK)
    {
      Error_Handler();
    }
    while(HAL_SPI_GetState(&SpiHandleDiscovery) != HAL_SPI_STATE_READY)
    {}
  }while(rxackbytes != SPI_SLAVE_SYNBYTE);
}

/**
  * @brief  SPI error callbacks
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  /* call error handler */
  Error_Handler();
}