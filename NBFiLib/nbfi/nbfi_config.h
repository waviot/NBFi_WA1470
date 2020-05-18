#ifndef NBFI_CONFIG_H
#define NBFI_CONFIG_H

#define READ_PARAM_CMD              0x00
#define WRITE_PARAM_CMD             0x01
#define WRITE_PARAM_WITH_ACK_CMD    0x02
#define WRITE_PARAM_AND_SAVE_CMD    0x03

#define NBFI_PARAM_MODE                 0x00
#define NBFI_PARAM_HANDSHAKE            0x01
#define NBFI_PARAM_MAXLEN               0x02
#define NBFI_PARAM_TXFREQ               0x03
#define NBFI_PARAM_RXFREQ               0x04
#define NBFI_PARAM_ANT                  0x05
#define NBFI_PARAM_DL_ADD               0x06
#define NBFI_PARAM_HEART_BEAT           0x07
#define NBFI_PARAM_TX_BRATES            0x08
#define NBFI_PARAM_RX_BRATES            0x09
#define NBFI_PARAM_VERSION              0x0A
#define NBFI_PARAM_ADD_FLAGS            0x0B
#define NBFI_PARAM_QUALITY              0x0C
#define NBFI_PARAM_UL_BASE_FREQ         0x0D
#define NBFI_PARAM_DL_BASE_FREQ         0x0E
#define NBFI_PARAM_QUALITY_EX           0x0F
#define NBFI_PARAM_BROADCAST_ADD        0x10
#define NBFI_PARAM_APP_IDS              0x11
#define NBFI_PARAM_BSANDSERVER_IDS      0x12
#define NBFI_PARAM_FPLAN                0x13
#define NBFI_PARAM_WAIT_ACK_TIMEOUT     0x14



extern nbfi_settings_t nbfi;
extern nbfi_dev_info_t dev_info;
extern nbfi_crypto_iterator_t nbfi_iter;

extern uint8_t you_should_dl_power_step_up;
extern uint8_t you_should_dl_power_step_down;
extern uint8_t current_tx_rate;
extern uint8_t current_rx_rate;

extern _Bool nbfi_settings_need_to_save_to_flash;

//NBFi flags:
#define NBFI_FLG_FIXED_BAUD_RATE                0x01
#define NBFI_FLG_NO_RESET_TO_DEFAULTS           0x02
#define NBFI_FLG_NO_SENDINFO                    0x04
#define NBFI_FLG_SEND_ALOHA                     0x08
#define NBFI_FLG_SEND_IN_RESPONSE               0x10
#define NBFI_FLG_NO_REDUCE_TX_PWR               0x20
#define NBFI_OFF_MODE_ON_INIT                   0x40
#define NBFI_FLG_DO_NOT_SEND_PKTS_ON_START      0x80
#define NBFI_FLG_SHORT_RANGE_CRYPTO           0x0100



//BAND IDs
#define UL868800_DL446000            0
#define UL868800_DL864000            1
#define UL868800_DL446000_DL864000   2
#define UL867950_DL446000            3
#define UL868500_DL446000            4
#define UL868100_DL446000            5
#define UL864000_DL446000            6
#define UL863175_DL446000            7
#define UL864000_DL875000            8
#define UL868100_DL869550            9
#define UL868500_DL864000            10
#define UL864000_DL864000            11 //KAZ
#define UL868800_DL869150            12 //NEWRU
#define UL866975_DL865000            13 //INDIA


//FREQENCY PLANS
#define NBFI_UL_FREQ_PLAN_NO_CHANGE             (64<<6)
#define NBFI_DL_FREQ_PLAN_NO_CHANGE             8
#define NBFI_FREQ_PLAN_MINIMAL                  0
#define NBFI_DL_FREQ_PLAN_FAULT                 7
#define NBFI_UL_FREQ_PLAN_51200_0               (384<<6)
#define NBFI_DL_FREQ_PLAN_819200_M2457600       59

#define NBFI_VOID_ALTERNATIVE   {0, UNDEFINED, UNDEFINED, NBFI_UL_FREQ_PLAN_NO_CHANGE + NBFI_DL_FREQ_PLAN_NO_CHANGE}



#define FULL_ID     ((uint8_t*)(nbfi.modem_id))

extern NBFi_station_info_s nbfi_station_info;

void    NBFI_Config_Check_State();
_Bool   NBFi_Config_Tx_Power_Change(nbfi_rate_direct_t dir);
void    NBFi_Config_Return();
void    NBFi_Config_Set_Default();
_Bool   NBFi_Config_Parser(uint8_t* buf);
void    NBFi_ReadConfig(nbfi_settings_t * settings);
void    NBFi_Config_Set_TX_Chan(nbfi_phy_channel_t ch);
void    NBFi_Config_Set_RX_Chan(nbfi_phy_channel_t ch);
_Bool   NBFi_Config_is_settings_default();
_Bool   NBFi_Config_Try_Alternative();
#endif // NBFI_CONFIG_H
