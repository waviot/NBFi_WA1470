#include "radio.h"
#include "spi.h"
#include "WVT_EEPROM.h"
#include "update.h"

#define DEFAULT_SETTINGS_MEMORY (0x0801ff80)

#define MODEM_ID ((uint32_t *)DEFAULT_SETTINGS_MEMORY)
#define KEY ((uint32_t *)(DEFAULT_SETTINGS_MEMORY + 4))

#define MANUFACTURER_ID 0x8888 //Waviot
#define HARDWARE_TYPE_ID 0x37   //Aqua3
#define PROTOCOL_ID 11          //undefined
#define TX_MAX_POWER 15
#define TX_MIN_POWER -13
#define SEND_INFO_PERIOD 2592000 //one time per month

#define BLOCK_SIZE 0x800

//#define BAND UL868800_DL869150

#if BAND == UL868800_DL446000
#define NBFI_UL_FREQ_BASE 868800000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL868800_DL864000
#define NBFI_UL_FREQ_BASE 868800000
#define NBFI_DL_FREQ_BASE 864000000
#elif BAND == UL868800_DL446000_DL864000
#define NBFI_UL_FREQ_BASE 868800000
#define NBFI_DL_FREQ_BASE 864000000
#elif BAND == UL867950_DL446000
#define NBFI_UL_FREQ_BASE 867950000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL868500_DL446000
#define NBFI_UL_FREQ_BASE 868500000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL868100_DL446000
#define NBFI_UL_FREQ_BASE 868100000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL864000_DL446000
#define NBFI_UL_FREQ_BASE 864000000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL863175_DL446000
#define NBFI_UL_FREQ_BASE 863175000
#define NBFI_DL_FREQ_BASE 446000000
#elif BAND == UL864000_DL875000
#define NBFI_UL_FREQ_BASE 864000000
#define NBFI_DL_FREQ_BASE 875000000
#elif BAND == UL868800_DL868000
#define NBFI_UL_FREQ_BASE 868800000
#define NBFI_DL_FREQ_BASE 868800000
#elif BAND == UL868800_DL869150
#define NBFI_UL_FREQ_BASE 868800000
#define NBFI_DL_FREQ_BASE 869150000
#endif

#define TRY_LOW_PHY_ALTERNATIVE                                                                          \
  {                                                                                                      \
    4, UL_DBPSK_50_PROT_E, DL_DBPSK_50_PROT_D, NBFI_UL_FREQ_PLAN_NO_CHANGE + NBFI_DL_FREQ_PLAN_NO_CHANGE \
  }
#define TRY_MINIMAL_UL_BAND_AND_LOW_PHY_ALTERNATIVE                                                 \
  {                                                                                                 \
    8, UL_DBPSK_50_PROT_E, DL_DBPSK_50_PROT_D, NBFI_FREQ_PLAN_MINIMAL + NBFI_DL_FREQ_PLAN_NO_CHANGE \
  }

const nbfi_settings_t nbfi_default_settings = //@DEFAULT_SETTINGS_MEMORY
    {
        MODEM_ID,
        KEY,
        DRX,                //mode;
        UL_DBPSK_50_PROT_E, //UL_DBPSK_50_PROT_D, // tx_phy_channel;
        DL_DBPSK_50_PROT_D, // rx_phy_channel;
        HANDSHAKE_SIMPLE,
        MACK_1,       //mack_mode
        0x82,         //num_of_retries;
        8,            //max_payload_len;
        0,            //wait_ack_timeout/ need assign to default_modem_id for compilance
        0,    //tx_freq;868775000, dl_base_freq:864000000)
        0,    //858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
        PCB,          //tx_antenna;
        PCB,          //rx_antenna;
        TX_MAX_POWER, //tx_pwr;
        60 * 60 * 12,       //60*5,             //heartbeat_interval
        255,          //heartbeat_num
        0,            //additional_flags
        NBFI_UL_FREQ_BASE,
        NBFI_DL_FREQ_BASE,
        NBFI_UL_FREQ_PLAN_51200_0 + NBFI_FREQ_PLAN_MINIMAL,
        {TRY_MINIMAL_UL_BAND_AND_LOW_PHY_ALTERNATIVE,
         TRY_LOW_PHY_ALTERNATIVE,
         NBFI_VOID_ALTERNATIVE,
         NBFI_VOID_ALTERNATIVE}};

const nbfi_dev_info_t nbfi_info = {TX_MIN_POWER, TX_MAX_POWER, MANUFACTURER_ID, HARDWARE_TYPE_ID, PROTOCOL_ID, BAND, SEND_INFO_PERIOD};

const nbfi_settings_t nbfi_short_range_settings =
    {
        MODEM_ID,
        KEY,
        DRX,                   //mode;
        UL_DBPSK_50_PROT_E,//UL_DBPSK_50_PROT_D, // tx_phy_channel;
        DL_DBPSK_50_PROT_D, // rx_phy_channel;
        HANDSHAKE_SIMPLE,
        MACK_1,             //mack_mode
        0x82,                  //num_of_retries;
        8,                  //max_payload_len;
        0,                  //wait_ack_timeout
        0,                  //tx_freq;
        0,//858090000,//868791000,//0,//868790000,//0,//868735500,//868710000,//868800000,                  //rx_freq;
        PCB,                //tx_antenna;
        PCB,                //rx_antenna;
        TX_MAX_POWER,       //tx_pwr;
        60*5,//60*5,             //heartbeat_interval
        255,                //heartbeat_num
        0,                  //additional_flags
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

void radio_ResetMemoryToZero(void);

void radio_switch_to_from_short_range(_Bool en)
{
  nbfi_crypto_iterator_t it = {0, 0};
  NBFi_switch_to_another_settings((nbfi_settings_t *)&nbfi_short_range_settings, &it, en);
}

void radio_init(void)
{
#ifdef DEBUG
  radio_ResetMemoryToZero();
#endif //DEBUG
  scheduler_HAL_init();
  SPI_Activate();

  wa1470_HAL_reg_data_received_callback((void *)NBFi_MAC_RX_ProtocolD);
  wa1470_HAL_reg_tx_finished_callback((void *)NBFi_RF_TX_Finished);
  wa1470_HAL_init();
  nbfi_HAL_init(&nbfi_default_settings, (nbfi_dev_info_t *)&nbfi_info);
  scheduler_run_callbacks();
  //NBFi_clear_Saved_Configuration(); //if you need to clear previously saved nbfi configuration in EEPROM
}


#ifdef DEBUG
void radio_ResetMemoryToZero(void)
{
   if((*MODEM_ID == 0xffffffff)||(*MODEM_ID == 0x00000000)||(MODEM_ID == 0x00000000))
   {
#pragma pack(8)
       static uint32_t oldData[BLOCK_SIZE/4] = {0x007ED895, 0xC80B92BB, 0x35B954DD, 0x210CFBDF,
       0x5D4C741A, 0x6917C130, 0xAE373CF6, 0x6D8AEC19, 0x9D11AA2C};

       UpdateInternalFlashErase(USR_FLASH_PAGE_NUMBER + 1);
       UpdateInternalFlashWrite((unsigned char *)oldData, ADDR_FLASH_PAGE_63, BLOCK_SIZE);
   }
}
#endif //DEBUG