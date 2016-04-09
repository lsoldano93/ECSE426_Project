
/* Includes ------------------------------------------------------------------*/
#include "Thread_SPICommunication.h"


/* Private Variables ---------------------------------------------------------*/

osThreadId tid_Thread_SPICommunication;   
SPI_HandleTypeDef NucleoSpiHandle;

osThreadDef(Thread_SPICommunication, osPriorityNormal, 1, NULL); 


/* Private Functions ---------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SPI_ACK_BYTES             0xA5A5
#define SPI_NACK_BYTES            0xDEAD
#define SPI_SLAVE_SYNBYTE         0x53
#define SPI_MASTER_SYNBYTE        0xAC
#define SPI_TIMEOUT_MAX           0x1000

/* Defines used for tranfer communication*/
#define ADDRCMD_MASTER_READ                         ((uint16_t)0x1234)
#define ADDRCMD_MASTER_WRITE                        ((uint16_t)0x5678)
#define CMD_LENGTH                                  ((uint16_t)0x0004)
#define DATA_LENGTH                                 ((uint16_t)0x0020)

/* Private macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* SPI handler declaration */
SPI_HandleTypeDef SpiDiscoveryHandle;

/* Buffer used for transmission */
uint8_t aTxMasterBuffer[] = "SPI - MASTER - Transmit message";
uint8_t aTxSlaveBuffer[]  = "SPI - SLAVE - Transmit message ";
/* Buffer used for reception */
uint8_t aRxBuffer[DATA_LENGTH];

uint32_t  SPI_Timeout = SPI_Timeout_Flag;

static void Slave_Synchro(void);
static void Error_Handler(void);
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
static void Flush_Buffer(uint8_t* pBuffer, uint16_t BufferLength);


/**  Initiates SPI Communication thread
   * @brief  Builds thread and starts it
   * @param  None
   * @retval Integer inidicating failure or success of thread initiation
   */
int start_Thread_SPICommunication (void) {

  tid_Thread_SPICommunication = osThreadCreate(osThread(Thread_SPICommunication ), NULL); 
  if (!tid_Thread_SPICommunication){
		printf("Error starting SPI communication thread!");
		return(-1); 
	}
  return(0);
}


/**  
   * @brief  Runs SPI communication thread which communicates with Nucleo-64 via SPI
	 * @param  None
   * @retval None
   */
void Thread_SPICommunication (void const *argument){
	
	HAL_StatusTypeDef errorCode;
	
	uint8_t returnValue;
	
  uint16_t addrcmd = 0;
  uint16_t comlength = 0;
  uint8_t pAddrcmd[CMD_LENGTH] = {0x00};
  uint16_t ackbyte = 0x0000;
  
	while(1){

		/* Wait for temperature sensor or accelerometer to update its value.
		   Upon signal, transmit temperature and accelerometer values before
			 receiving current LED state value from Nucleo                  */
		osSignalWait(THREAD_GREEN_LIGHT, THREAD_TIMEOUT);
	
		/* Synchronization between Master and Slave */
    Slave_Synchro();

    /* Receive command from Master */
    if(HAL_SPI_Receive_IT(&SpiDiscoveryHandle, pAddrcmd, CMD_LENGTH) != HAL_OK)
    {
      Error_Handler();
    }

    /*  Before starting a new communication transfer, you need to check the current
    state of the peripheral; if it’s busy you need to wait for the end of current
    transfer before starting a new one.
    For simplicity reasons, this example is just waiting till the end of the
    transfer, but application may perform other tasks while transfer operation
    is ongoing. */
    while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
    {}

    /* Compute command and required data length */
    addrcmd = (uint16_t) ((pAddrcmd[0] << 8) | pAddrcmd[1]);
    comlength = (uint16_t) ((pAddrcmd[2] << 8) | pAddrcmd[3]);

    /* Check if received command correct */
    if(((addrcmd == ADDRCMD_MASTER_READ) || (addrcmd == ADDRCMD_MASTER_WRITE)) && (comlength > 0))
    {
      /* Synchronization between Master and Slave */
      Slave_Synchro();

      /* Send acknowledge to Master */
      ackbyte = SPI_ACK_BYTES;
      if(HAL_SPI_Transmit_IT(&SpiDiscoveryHandle, (uint8_t *)&ackbyte, sizeof(ackbyte)) != HAL_OK)
      {
        Error_Handler();
      }

      while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
      {}

      /* Check if Master requiring data read or write */
      if(addrcmd == ADDRCMD_MASTER_READ)
      {
        /* Synchronization between Master and Slave */
        Slave_Synchro();

        /* Send data to Master */
        if(HAL_SPI_Transmit_IT(&SpiDiscoveryHandle, aTxSlaveBuffer, DATA_LENGTH) != HAL_OK)
        {
          Error_Handler();
        }

        while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
        {}

        /* Synchronization between Master and Slave */
        Slave_Synchro();

        /* Receive acknowledgement from Master */
        ackbyte = 0;
        if(HAL_SPI_Receive_IT(&SpiDiscoveryHandle, (uint8_t *)&ackbyte, sizeof(ackbyte)) != HAL_OK)
        {
          Error_Handler();
        }

        while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
        {}

        /* Check acknowledgement */
        if(ackbyte !=  SPI_ACK_BYTES)
        {
          Error_Handler();
        }
      }
      else if(addrcmd == ADDRCMD_MASTER_WRITE)
      {
        /* Synchronization between Master and Slave */
        Slave_Synchro();

        /* Receive data from Master */
        if(HAL_SPI_Receive_IT(&SpiDiscoveryHandle, aRxBuffer, DATA_LENGTH) != HAL_OK)
        {
          Error_Handler();
        }

        while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
        {}

        /* Synchronization between Master and Slave */
        Slave_Synchro();

        /* Send acknowledgement to Master */
        ackbyte = SPI_ACK_BYTES;
        if(HAL_SPI_Transmit_IT(&SpiDiscoveryHandle, (uint8_t *)&ackbyte, sizeof(ackbyte)) != HAL_OK)
        {
          Error_Handler();
        }

        while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
        {}

        /* In case, Master has sent data, compare received buffer with one expected */
        if(Buffercmp((uint8_t*)aTxMasterBuffer, (uint8_t*)aRxBuffer, DATA_LENGTH))
        {
          /* Transfer error in transmission process */
          Error_Handler();
        }
        else
        {
          /* Toggle LED6 on: Reception is correct */
          //BSP_LED_Toggle(LED6);
        }
      }
    }
    else
    {
      /* Synchronization between Master and Slave */
      Slave_Synchro();

      /* Send acknowledgement to Master */
      ackbyte = SPI_NACK_BYTES;
      if(HAL_SPI_Transmit_IT(&SpiDiscoveryHandle, (uint8_t *)&ackbyte, sizeof(ackbyte)) != HAL_OK)
      {
        Error_Handler();
      }

      while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
      {}

      Error_Handler();
    }

    /* Flush Rx buffer for next transmission */
    Flush_Buffer(aRxBuffer, DATA_LENGTH);
  }

	                                                       
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
  GPIO_InitTypeDef GPIO_InitStructure;

  
	SpiDiscoveryHandle.Instance               = SPI2;
  SpiDiscoveryHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  SpiDiscoveryHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiDiscoveryHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
  SpiDiscoveryHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiDiscoveryHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiDiscoveryHandle.Init.CRCPolynomial     = 7;
  SpiDiscoveryHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiDiscoveryHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiDiscoveryHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiDiscoveryHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
  SpiDiscoveryHandle.Init.Mode              = SPI_MODE_SLAVE;
  
  if(HAL_SPI_Init(&SpiDiscoveryHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
	
  __GPIOB_CLK_ENABLE();
  
  /* SPI SCK GPIO pin configuration  */
  GPIO_InitStructure.Pin       = DISCOVERY_SPI_SCK_PIN;
  GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull      = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* SPI MISO GPIO pin configuration  */
  GPIO_InitStructure.Pull      = GPIO_PULLUP;
  GPIO_InitStructure.Pin       = DISCOVERY_SPI_MISO_PIN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* SPI MOSI GPIO pin configuration  */
  GPIO_InitStructure.Pin       = DISCOVERY_SPI_MOSI_PIN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /*##-3- Enable SPI peripheral Clock ########################################*/
  /* Enable SPI clock */
  __SPI2_CLK_ENABLE();

  /*##-4- Configure the NVIC for SPI #########################################*/
  /* NVIC for SPI */
  HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(SPI2_IRQn);
	
	
}
/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to its default state
  *          - Revert NVIC configuration to its default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  /*##-1- Reset peripherals ##################################################*/
  __SPI2_FORCE_RESET();
  __SPI2_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure SPI SCK as alternate function  */
  HAL_GPIO_DeInit(GPIOB, DISCOVERY_SPI_SCK_PIN);
  /* Configure SPI MISO as alternate function  */
  HAL_GPIO_DeInit(GPIOB, DISCOVERY_SPI_MISO_PIN);
  /* Configure SPI MOSI as alternate function  */
  HAL_GPIO_DeInit(GPIOB, DISCOVERY_SPI_MOSI_PIN);

  /*##-3- Disable the NVIC for SPI ###########################################*/
  HAL_NVIC_DisableIRQ(SPI2_IRQn);
}
/**
  * @brief  Slave synchronization with Master
  * @param  None
  * @retval None
  */
static void Slave_Synchro(void)
{
  uint8_t txackbyte = SPI_SLAVE_SYNBYTE, rxackbyte = 0x00;

  do
  {
    if (HAL_SPI_TransmitReceive_IT(&SpiDiscoveryHandle, (uint8_t *)&txackbyte, (uint8_t *)&rxackbyte, 1) != HAL_OK)
    {
      Error_Handler();
    }

    while (HAL_SPI_GetState(&SpiDiscoveryHandle) != HAL_SPI_STATE_READY)
    {}
  }
  while (rxackbyte != SPI_MASTER_SYNBYTE);
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

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED5 (RED) on */
  //BSP_LED_On(LED5);
  while(1)
  {
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
