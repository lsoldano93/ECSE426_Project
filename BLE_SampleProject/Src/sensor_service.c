/**
  ******************************************************************************
  * @file    sensor_service.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   Add a sample service using a vendor specific profile.
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
#include "sensor_service.h"

/** @addtogroup X-CUBE-BLE1_Applications
 *  @{
 */

/** @addtogroup SensorDemo
 *  @{
 */
 
/** @defgroup SENSOR_SERVICE
 * @{
 */

/** @defgroup SENSOR_SERVICE_Private_Variables
 * @{
 */
/* Private variables ---------------------------------------------------------*/
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE;

uint16_t sampleServHandle, TXCharHandle, RXCharHandle;
uint16_t ledServHandle, ledStateCharHandle;
uint16_t sensorServHandle, tempCharHandle, pitchCharHandle, rollCharHandle;

extern uint16_t ledState;
uint16_t ledStateBytes[] = {0, 0};


/** @defgroup SENSOR_SERVICE_Private_Macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)


  // LED service
  #define COPY_LED_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x02,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
  #define COPY_LED_STATE_CHAR_UUID(uuid_struct)          COPY_UUID_128(uuid_struct,0x34,0x0a,0x1b,0x80, 0xcf,0x4b, 0x11,0xe1, 0xac,0x36, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
 
  #define COPY_SENSOR_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x42,0x82,0x1a,0x40, 0xe4,0x77, 0x11,0xe2, 0x82,0xd0, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
  #define COPY_TEMP_CHAR_UUID(uuid_struct)         COPY_UUID_128(uuid_struct,0xa3,0x2e,0x55,0x20, 0xe4,0x77, 0x11,0xe2, 0xa9,0xe3, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
  #define COPY_PITCH_CHAR_UUID(uuid_struct)        COPY_UUID_128(uuid_struct,0xcd,0x20,0xc4,0x80, 0xe4,0x8b, 0x11,0xe2, 0x84,0x0b, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
  #define COPY_ROLL_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x01,0xc5,0x0b,0x60, 0xe4,0x8c, 0x11,0xe2, 0xa0,0x73, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)


/* Store Value into a buffer in Little Endian Format */
#define STORE_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )


uint16_t readLedState(){
	return ledState;
}

tBleStatus Add_Led_Service(void){
	tBleStatus ret;
  uint8_t uuid[16];
  uint16_t uuid16;
  charactFormat charFormat;
  uint16_t descHandle;
	
	COPY_LED_SERVICE_UUID(uuid);
  ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 10,
                          &ledServHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;
	
	/* Led State Characteristic */
  COPY_LED_STATE_CHAR_UUID(uuid);  
  ret =  aci_gatt_add_char(ledServHandle, UUID_TYPE_128, uuid, 2,
                           CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                           16, 1, &ledStateCharHandle);
  if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding LED service\n");
		 goto fail;
	}
  
  charFormat.format = FORMAT_UINT16;
	charFormat.exp = -1;
	charFormat.unit = UNIT_UNITLESS;
	charFormat.name_space = 0;
	charFormat.desc = 0;
  
  uuid16 = CHAR_FORMAT_DESC_UUID;
  
  ret = aci_gatt_add_char_desc(ledServHandle,
                               ledStateCharHandle,
                               UUID_TYPE_16,
                               (uint8_t *)&uuid16, 
                               7,
                               7,
                               (void *)&charFormat, 
                               ATTR_PERMISSION_NONE,
                               ATTR_ACCESS_WRITE_ANY,
                               0,
                               16,
                               FALSE,
                               &descHandle);
  if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding LED STATE characteristic\n");
		 goto fail;
	}
	
  return BLE_STATUS_SUCCESS;
											 
fail:
  return BLE_STATUS_ERROR ;
}
/**
 * @brief  Add the Environmental Sensor service.
 *
 * @param  None
 * @retval Status
 */
tBleStatus Add_Sensor_Service(void)
{
  tBleStatus ret;
  uint8_t uuid[16];
  uint16_t uuid16;
  charactFormat charFormat;
  uint16_t descHandle;
  
  COPY_SENSOR_SERVICE_UUID(uuid);
  ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 10,
                          &sensorServHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;
  
  /* Temperature Characteristic */
  COPY_TEMP_CHAR_UUID(uuid);  
  ret =  aci_gatt_add_char(sensorServHandle, UUID_TYPE_128, uuid, 2,
                           CHAR_PROP_READ, ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
                           16, 0, &tempCharHandle);
  if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding TEMP service\n");
		 goto fail;
	}
  
  charFormat.format = FORMAT_UINT16;
	charFormat.exp = -1;
	charFormat.unit = UNIT_UNITLESS;
	charFormat.name_space = 0;
	charFormat.desc = 0;
  
  uuid16 = CHAR_FORMAT_DESC_UUID;
  
  ret = aci_gatt_add_char_desc(sensorServHandle,
                               tempCharHandle,
                               UUID_TYPE_16,
                               (uint8_t *)&uuid16, 
                               7,
                               7,
                               (void *)&charFormat, 
                               ATTR_PERMISSION_NONE,
                               ATTR_ACCESS_READ_ONLY,
                               0,
                               16,
                               FALSE,
                               &descHandle);
  if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding TEMP characteristic\n");
		 goto fail;
	}
  
  /* Pitch Characteristic */
	COPY_PITCH_CHAR_UUID(uuid);  
	ret =  aci_gatt_add_char(sensorServHandle, UUID_TYPE_128, uuid, 2,
													 CHAR_PROP_READ, ATTR_PERMISSION_NONE,
													 GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
													 16, 0, &pitchCharHandle);
	if (ret != BLE_STATUS_SUCCESS) goto fail;
	
	charFormat.format = FORMAT_UINT16;
	charFormat.exp = -1;
	charFormat.unit = UNIT_UNITLESS;
	charFormat.name_space = 0;
	charFormat.desc = 0;
	
	uuid16 = CHAR_FORMAT_DESC_UUID;
	
	ret = aci_gatt_add_char_desc(sensorServHandle,
															 pitchCharHandle,
															 UUID_TYPE_16,
															 (uint8_t *)&uuid16, 
															 7,
															 7,
															 (void *)&charFormat, 
															 ATTR_PERMISSION_NONE,
															 ATTR_ACCESS_READ_ONLY,
															 0,
															 16,
															 FALSE,
															 &descHandle);
	if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding PITCH characteristic\n");
		 goto fail;
	}

  /* Roll Characteristic */
 
	COPY_ROLL_CHAR_UUID(uuid);  
	ret =  aci_gatt_add_char(sensorServHandle, UUID_TYPE_128, uuid, 2,
													 CHAR_PROP_READ, ATTR_PERMISSION_NONE,
													 GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
													 16, 0, &rollCharHandle);
	if (ret != BLE_STATUS_SUCCESS) goto fail;
	
	charFormat.format = FORMAT_UINT16;
	charFormat.exp = -1;
	charFormat.unit = UNIT_UNITLESS;
	charFormat.name_space = 0;
	charFormat.desc = 0;
	
	uuid16 = CHAR_FORMAT_DESC_UUID;
	
	ret = aci_gatt_add_char_desc(sensorServHandle,
															 rollCharHandle,
															 UUID_TYPE_16,
															 (uint8_t *)&uuid16, 
															 7,
															 7,
															 (void *)&charFormat, 
															 ATTR_PERMISSION_NONE,
															 ATTR_ACCESS_READ_ONLY,
															 0,
															 16,
															 FALSE,
															 &descHandle);
	if (ret != BLE_STATUS_SUCCESS){
		printf("ERROR adding ROLL characteristic\n");
		goto fail;
	}  
  return BLE_STATUS_SUCCESS;										 
  
fail:
  return BLE_STATUS_ERROR ;
  
}

/**
 * @brief  Update temperature characteristic value.
 * @param  Temperature in tenths of degree 
 * @retval Status
 */
tBleStatus Temp_Update(int16_t temp)
{  
  tBleStatus ret;
  
  ret = aci_gatt_update_char_value(sensorServHandle, tempCharHandle, 0, 2,
                                   (uint8_t*)&temp);
  
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating TEMP characteristic.\n") ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;
	
}

/**
 * @brief  Update temperature characteristic value.
 * @param  Temperature in tenths of degree 
 * @retval Status
 */
tBleStatus Pitch_Update(int16_t pitch)
{  
  tBleStatus ret;
  
  ret = aci_gatt_update_char_value(sensorServHandle, pitchCharHandle, 0, 2,
                                   (uint8_t*)&pitch);
  
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating PITCH characteristic.\n") ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;
	
}

/**
 * @brief  Update temperature characteristic value.
 * @param  Temperature in tenths of degree 
 * @retval Status
 */
tBleStatus Roll_Update(int16_t roll)
{  
  tBleStatus ret;
  
  ret = aci_gatt_update_char_value(sensorServHandle, rollCharHandle, 0, 2,
                                   (uint8_t*)&roll);
  
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating ROLL characteristic.\n") ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;
	
}




/**
 * @brief  Puts the device in connectable mode.
 *         If you want to specify a UUID list in the advertising data, those data can
 *         be specified as a parameter in aci_gap_set_discoverable().
 *         For manufacture data, aci_gap_update_adv_data must be called.
 * @param  None 
 * @retval None
 */
/* Ex.:
 *
 *  tBleStatus ret;    
 *  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G'};    
 *  const uint8_t serviceUUIDList[] = {AD_TYPE_16_BIT_SERV_UUID,0x34,0x12};    
 *  const uint8_t manuf_data[] = {4, AD_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x05, 0x02, 0x01};
 *  
 *  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
 *                                 8, local_name, 3, serviceUUIDList, 0, 0);    
 *  ret = aci_gap_update_adv_data(5, manuf_data);
 *
 */
void setConnectable(void)
{  
  tBleStatus ret;
  
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','T','L','E','_','G','4'};
  
  /* disable scan response */
  hci_le_set_scan_resp_data(0,NULL);
  PRINTF("General Discoverable Mode.\n");
  
  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                 sizeof(local_name), local_name, 0, NULL, 0, 0);
  if (ret != BLE_STATUS_SUCCESS) {
    PRINTF("Error while setting discoverable mode (%d)\n", ret);    
  }  
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  uint8_t Address of peer device
 * @param  uint16_t Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{  
  connected = TRUE;
  connection_handle = handle;
  
  PRINTF("Connected to device:");
  for(int i = 5; i > 0; i--){
    PRINTF("%02X-", addr[i]);
  }
  PRINTF("%02X\n", addr[0]);
}

/**
 * @brief  This function is called when the peer device gets disconnected.
 * @param  None 
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
  connected = FALSE;
  PRINTF("Disconnected\n");
  /* Make the device connectable again. */
  set_connectable = TRUE;
  notification_enabled = FALSE;
}

/**
 * @brief  Read request callback.
 * @param  uint16_t Handle of the attribute
 * @retval None
 */
void Read_Request_CB(uint16_t handle)
{  
  //EXIT:
  if(connection_handle != 0)
    aci_gatt_allow_read(connection_handle);
}


void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t att_data[]){
	
	if (handle == ledStateCharHandle + 1){
		
		ledStateBytes[0] = att_data[0];
		ledStateBytes[1] = att_data[1];
		
		ledState = (ledStateBytes[0] << 8) + ledStateBytes[1];
		printf("%i :: %i --> %i\n", ledStateBytes[0], ledStateBytes[1], ledState);
	}
}
/**
 * @brief  Callback processing the ACI events.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
  hci_uart_pckt *hci_pckt = pckt;
  /* obtain event packet */
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;
  
  if(hci_pckt->type != HCI_EVENT_PKT)
    return;
  
  switch(event_pckt->evt){
    
  case EVT_DISCONN_COMPLETE:
    {
      GAP_DisconnectionComplete_CB();
    }
    break;
    
  case EVT_LE_META_EVENT:
    {
      evt_le_meta_event *evt = (void *)event_pckt->data;
      
      switch(evt->subevent){
      case EVT_LE_CONN_COMPLETE:
        {
          evt_le_connection_complete *cc = (void *)evt->data;
          GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        }
        break;
      }
    }
    break;
    
  case EVT_VENDOR:
    {
      evt_blue_aci *blue_evt = (void*)event_pckt->data;
      switch(blue_evt->ecode){

      case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:         
        {
          /* this callback is invoked when a GATT attribute is modified
          extract callback data and pass to suitable handler function */
          evt_gatt_attr_modified_IDB04A1 *evt = (evt_gatt_attr_modified_IDB04A1*)blue_evt->data;
          Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data); 
        }
        break; 

      case EVT_BLUE_GATT_READ_PERMIT_REQ:
        {
          evt_gatt_read_permit_req *pr = (void*)blue_evt->data;                    
          Read_Request_CB(pr->attr_handle);                    
        }
        break;
      }
    }
    break;
  }    
}

/* NEW_SERVICES */
/**
 * @}
 */
 
/**
 * @}
 */

/**
 * @}
 */

 /**
 * @}
 */
 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
