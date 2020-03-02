#include "main.h"
#include "radio.h"
#include "wa1470_hal.h"
#include "scheduler_hal.h"
#include "string.h"
#include "nbfi.h"
//#include "nbfi_mac.h"
//#include "nbfi_rf.h"
//#include "nbfi_config.h"
//#include "stm32l0xx_hal_conf.h"
#include "defines.h"

#define MODEM_ID  *((const uint32_t*)0x0801ff80)  
#define KEY  ((const uint32_t*)0x0801ff84)            

#define MANUFACTURER_ID         0x8888 //Waviot
#define HARDWARE_TYPE_ID        0x3       //ASIC_PROTOTYPE
#define PROTOCOL_ID             0       //undefined
#define TX_MAX_POWER 15
#define TX_MIN_POWER -13
#define SEND_INFO_PERIOD	2592000         //one time per month
#define BAND         UL868800_DL869100          //UL868800_DL864000



#if BAND == UL868800_DL446000
#define NBFI_UL_FREQ_BASE       868800000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868800_DL864000
#define NBFI_UL_FREQ_BASE       868800000
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND == UL868800_DL446000_DL864000
#define NBFI_UL_FREQ_BASE       868800000
#define NBFI_DL_FREQ_BASE       864000000
#elif BAND == UL867950_DL446000
#define NBFI_UL_FREQ_BASE       867950000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868500_DL446000
#define NBFI_UL_FREQ_BASE       868500000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL868100_DL446000
#define NBFI_UL_FREQ_BASE       868100000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL864000_DL446000
#define NBFI_UL_FREQ_BASE       864000000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL863175_DL446000
#define NBFI_UL_FREQ_BASE       863175000
#define NBFI_DL_FREQ_BASE       446000000
#elif BAND == UL864000_DL875000
#define NBFI_UL_FREQ_BASE       864000000
#define NBFI_DL_FREQ_BASE       875000000
#elif BAND == UL868800_DL868000
#define NBFI_UL_FREQ_BASE       868800000
#define NBFI_DL_FREQ_BASE       868800000
#elif BAND == UL868800_DL869100
#define NBFI_UL_FREQ_BASE       868800000    
#define NBFI_DL_FREQ_BASE       869150000//869100000
#endif 


#ifdef BANKA
const nbfi_settings_t nbfi_set_default =
{
    CRX,//mode;
    UL_DBPSK_50_PROT_D,//UL_DBPSK_50_PROT_D, // tx_phy_channel;
    DL_DBPSK_3200_PROT_D, // rx_phy_channel;
    HANDSHAKE_NONE,
    MACK_1,             //mack_mode
    2,                  //num_of_retries;
    8,                  //max_payload_len;
    0,                  //dl_ID[4];
    868800000 + 20000,                  //tx_freq;
    0,//858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
    PCB,                //tx_antenna;
    PCB,                //rx_antenna;
    TX_MAX_POWER,       //tx_pwr;
    1,//3600*6,             //heartbeat_interval
    0,                //heartbeat_num
    NBFI_FLG_FIXED_BAUD_RATE,                  //additional_flags
    NBFI_UL_FREQ_BASE,
    NBFI_DL_FREQ_BASE,
    NBFI_FREQ_PLAN_DEFAULT,
    {
      NBFI_VOID_ALTERNATIVE,
      NBFI_VOID_ALTERNATIVE,
      NBFI_VOID_ALTERNATIVE,
      NBFI_VOID_ALTERNATIVE
    }
};
#else

#define TRY_LOW_PHY_ALTERNATIVE   {4, UL_DBPSK_50_PROT_E, DL_DBPSK_50_PROT_D, NBFI_UL_FREQ_PLAN_NO_CHANGE + NBFI_DL_FREQ_PLAN_NO_CHANGE} 

#define TRY_MINIMAL_UL_BAND_AND_LOW_PHY_ALTERNATIVE   {8, UL_DBPSK_50_PROT_E, DL_DBPSK_50_PROT_D, NBFI_FREQ_PLAN_MINIMAL + NBFI_DL_FREQ_PLAN_NO_CHANGE}

const nbfi_settings_t nbfi_set_default =
{
    CRX,//mode;
    UL_DBPSK_400_PROT_E,//UL_DBPSK_50_PROT_D, // tx_phy_channel;
    DL_DBPSK_400_PROT_D, // rx_phy_channel;
    HANDSHAKE_SIMPLE,
    MACK_1,             //mack_mode
    0x82,                  //num_of_retries;
    8,                  //max_payload_len;
    0,                  //dl_ID;
    0,                  //tx_freq;
    0,//858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
    PCB,                //tx_antenna;
    PCB,                //rx_antenna;
    TX_MAX_POWER,       //tx_pwr;
    30,//3600*6,             //heartbeat_interval
    255,                //heartbeat_num
    0,//NBFI_FLG_FIXED_BAUD_RATE,                  //additional_flags
    NBFI_UL_FREQ_BASE,
    NBFI_DL_FREQ_BASE,
    NBFI_UL_FREQ_PLAN_51200_0 + NBFI_FREQ_PLAN_MINIMAL,
    {
      TRY_MINIMAL_UL_BAND_AND_LOW_PHY_ALTERNATIVE,
      TRY_LOW_PHY_ALTERNATIVE,
      NBFI_VOID_ALTERNATIVE,
      NBFI_VOID_ALTERNATIVE
    }
};
#endif



void nbfi_before_tx()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_RESET);
  #endif
}

void nbfi_before_rx()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_SET);
  #endif
}

void nbfi_before_off()
{
  #ifdef PHOBOS_EXT_MODULE
    HAL_GPIO_WritePin(AX_SWITCH_TX_ON_GPIO_Port, AX_SWITCH_TX_ON_Pin,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(AX_SWITCH_NTX_ON_GPIO_Port, AX_SWITCH_NTX_ON_Pin,  GPIO_PIN_RESET);
  #endif
}

void nbfi_lock_unlock_loop_irq(uint8_t lock)
{
  scheduler_HAL_lock_unlock(lock);
}

void nbfi_read_default_settings(nbfi_settings_t* settings)
{
  for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
  {
    ((uint8_t *)settings)[i] = ((uint8_t *)&nbfi_set_default)[i];
  }
}


#define EEPROM_INT_nbfi_data (DATA_EEPROM_BASE + 1024*5)

void nbfi_read_flash_settings(nbfi_settings_t* settings) 
{
	memcpy((void*)settings, ((const void*)EEPROM_INT_nbfi_data), sizeof(nbfi_settings_t));
}

void nbfi_write_flash_settings(nbfi_settings_t* settings)
{	
    if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK) return;
    for(uint8_t i = 0; i != sizeof(nbfi_settings_t); i++)
    {
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_INT_nbfi_data + i, ((uint8_t *)settings)[i]) != HAL_OK) break;
    }
    HAL_FLASHEx_DATAEEPROM_Lock(); 
}

int ADC_get(uint32_t * voltage, uint32_t * temp);
uint32_t nbfi_measure_valtage_or_temperature(uint8_t val)
{
	uint32_t voltage, temp;
	ADC_get(&voltage, &temp);
	return val ? voltage / 10 : temp;
}

uint32_t nbfi_update_rtc()
{
  //you should use this callback when external RTC used
  //return rtc_counter;  
  return 0;
}

void nbfi_rtc_synchronized(uint32_t time)
{
  //you should use this callback for RTC counter correction when external RTC used
  //rtc_counter = time;
  
}


__weak void nbfi_send_complete(nbfi_ul_sent_status_t ul)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}

__weak void nbfi_receive_complete(uint8_t * data, uint16_t length)
{

  /* NOTE : This function Should not be modified, when the callback is needed,
            the nbfi_receive_complete could be implemented in the user file
   */
}


void nbfi_get_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Read iterator from retain storage
	iter->ul = iter->dl = 0;
        
        //iter->dl = 260;
}

void nbfi_set_iterator(nbfi_crypto_iterator_t * iter)
{
	//	Write iterator to retain storage
	//	Cause every send/receive packet
}

void radio_init(void)
{

  
        scheduler_HAL_init();
        //RADIO_GPIO_Init();
        
	//RADIO_LPTIM_Init();       

	//HAL_LPTIM_Counter_Start(&hlptim, 0xffff);

	//RADIO_SPI_Init();

	//RADIO_BPSK_PIN_Init();



	NBFI_reg_func(NBFI_BEFORE_TX, (void*)nbfi_before_tx);
	NBFI_reg_func(NBFI_BEFORE_RX, (void*)nbfi_before_rx);
	NBFI_reg_func(NBFI_BEFORE_OFF, (void*)nbfi_before_off);
        NBFI_reg_func(NBFI_LOCK_UNLOCK_LOOP_IRQ, (void*)nbfi_lock_unlock_loop_irq);
        NBFI_reg_func(NBFI_SEND_COMPLETE, (void*)nbfi_send_complete);
	NBFI_reg_func(NBFI_RECEIVE_COMLETE, (void*)nbfi_receive_complete);
	NBFI_reg_func(NBFI_READ_FLASH_SETTINGS, (void*)nbfi_read_flash_settings);
	NBFI_reg_func(NBFI_WRITE_FLASH_SETTINGS, (void*)nbfi_write_flash_settings);
	NBFI_reg_func(NBFI_READ_DEFAULT_SETTINGS, (void*)nbfi_read_default_settings);
	NBFI_reg_func(NBFI_MEASURE_VOLTAGE_OR_TEMPERATURE, (void*)nbfi_measure_valtage_or_temperature);
	NBFI_reg_func(NBFI_GET_ITERATOR, (void*)nbfi_get_iterator);
	NBFI_reg_func(NBFI_SET_ITERATOR, (void*)nbfi_set_iterator);
        
        //RADIO_LOOPTIM_Init();
        
        
	//register callbacks when external RTC used
	//NBFI_reg_func(NBFI_UPDATE_RTC, (void*)nbfi_update_rtc);
	//NBFI_reg_func(NBFI_RTC_SYNCHRONIZED, (void*)nbfi_rtc_synchronized);

	nbfi_dev_info_t info = {MODEM_ID, (uint32_t*)KEY, TX_MIN_POWER, TX_MAX_POWER, MANUFACTURER_ID, HARDWARE_TYPE_ID, PROTOCOL_ID, BAND, SEND_INFO_PERIOD};

	NBFi_set_Device_Info(&info);

	NBFi_clear_Saved_Configuration(); //if you need to clear previously saved nbfi configuration in EEPROM

	//wa1470_reg_func(WARADIO_DATA_RECEIVED, (void*)NBFi_MAC_RX_ProtocolD);
	//wa1470_reg_func(WARADIO_TX_FINISHED, (void*)NBFi_RF_TX_Finished);
        
        WA1470_HAL_reg_data_received_callback((void*)NBFi_MAC_RX_ProtocolD);
        WA1470_HAL_reg_tx_finished_callback((void*)NBFi_RF_TX_Finished);
        
        WA1470_HAL_init(MODEM_ID);
        
	//wa1470_init(WA1470_SEND_BY_BPSK_PIN/*WA1470_SEND_BY_I_Q_MODULATOR*/, MODEM_ID);
        //wa1470_init(WA1470_SEND_BY_I_Q_MODULATOR, MODEM_ID);
	NBFI_Init();  
}
