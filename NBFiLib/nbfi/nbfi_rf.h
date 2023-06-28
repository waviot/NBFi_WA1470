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
    STATE_CHANGED,
    STATE_UNDEFINED=255
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
extern uint32_t protd_rx_preambule;

typedef struct
{
	void (*init)(_Bool send_by_bpsk_pin, uint32_t modem_id, wa1470_HAL_st*, ischeduler_st*);
	void (*reinit)(uint32_t preambule);
	void (*deinit)();
	_Bool (*cansleep)();

	void (*rfe_set_mode)(rfe_mode_s mode);
	void (*rfe_set_tx_power)(int8_t power);

	void (*mod_set_freq)(uint32_t freq);
	void (*mod_send)(uint8_t* data, mod_bitrate_s bitrate);
	_Bool (*mod_is_tx_in_progress)();

	void (*dem_rx_enable)(_Bool en);
	void (*dem_set_bitrate)(dem_bitrate_s bitrate);
	void (*dem_set_freq)(uint32_t freq);
	float (*dem_get_rssi)();
	float (*dem_get_noise)();
} nbfi_rf_iface_t;

void NBFI_RF_iface(nbfi_rf_iface_t iface);

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
float NBFi_RF_get_rssi();

#endif // NBFI_RF_H
