#ifndef NBFI_RF_H
#define NBFI_RF_H

#define PA_DIFFERENTIAL (0x1)
#define PA_SINGLE_ENDED (0x2)
#define PA_SHAPING      (0x4)

typedef enum
{
    STATE_OFF,
    STATE_RX,
    STATE_TX,
    STATE_CHANGED
}nbfi_rf_state_s;


typedef enum
{
    NONBLOCKING,
    BLOCKING,
}rf_blocking_t;


extern  struct axradio_address fastdladdress;

extern nbfi_rf_state_s rf_state;
extern nbfi_phy_channel_t nbfi_phy_channel;
extern _Bool rf_busy;
extern _Bool transmit;

nbfi_status_t   NBFi_RF_Init(nbfi_phy_channel_t  phy_channel,
                        nbfi_rf_antenna_t        antenna,
                        int8_t              power,
                        uint32_t            freq);
nbfi_status_t   NBFi_RF_Deinit();
nbfi_status_t   NBFi_RF_Transmit(uint8_t* pkt, uint8_t len, nbfi_phy_channel_t  phy_channel,  rf_blocking_t blocking);
void            NBFi_RF_TX_Finished();
_Bool           NBFi_RF_is_TX_in_Progress();
_Bool           NBFi_RF_can_Sleep();
uint32_t        NBFi_DL_ID();


float NBFi_RF_get_noise();

#endif // NBFI_RF_H
