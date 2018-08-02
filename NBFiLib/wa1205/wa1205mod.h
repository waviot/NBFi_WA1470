#ifndef _WA1205MOD_H
#define _WA1205MOD_H

#define MOD_SPI_OFFSET                  0x38

//Write registers definitions
#define MOD_CONFIG              (MOD_SPI_OFFSET + 0x0)
#define MOD_PER0                (MOD_SPI_OFFSET + 0x1)
#define MOD_PER1                (MOD_SPI_OFFSET + 0x2)
#define MOD_PER2                (MOD_SPI_OFFSET + 0x3)
#define MOD_DATA_START          (MOD_SPI_OFFSET + 0x4)
#define MOD_HOP_TBL_START       (MOD_SPI_OFFSET + 0x2C)

#define MOD_CONF_HOP_EN                 0x80
#define MOD_CONF_TX_ABORT               0x40
#define MOD_CONF_PROT_E_EN              0x20
#define MOD_CONF_IRQ_ON_TX_END_EN       0x10
//#define MOD_CONF_IRQ_ON_TX_ERR_EN       0x08
#define MOD_CONF_CLEAR_IRQ              0x02
#define MOD_CONF_TX_START               0x01

//Read registers definitions

#define MOD_CURR_TX_BITS_SENT   (MOD_SPI_OFFSET + 0x0)
#define MOD_STATUS              (MOD_SPI_OFFSET + 0x1)
#define MOD_SENT_TOTAL          (MOD_SPI_OFFSET + 0x2)
#define MOD_ERR_TOTAL           (MOD_SPI_OFFSET + 0x3)
#define MOD_TX_LAST_BIT         (MOD_SPI_OFFSET + 0x5)

#define MOD_STATUS_TX_IN_PROGRESS       0x80
#define MOD_STATUS_HOP_REPEAT_WARNING   0x40
#define MOD_STATUS_IRQ_ON_TX_FLAG       0x10
//#define MOD_STATUS_IRQ_ON_ERR_FLAG      0x08

typedef enum
{
        MINUS97000              = 0,
        MINUS89000              = 1,
        MINUS83000              = 2,
        MINUS90000              = 3,
	MINUS79000              = 4,
	MINUS73000              = 5,
	MINUS59000      	= 6,
	MINUS65000      	= 7,
        MINUS53000      	= 8,
        MINUS47000      	= 9,
        MINUS37000      	= 10,
        MINUS40000      	= 11,
        MINUS29000              = 12,
        MINUS19000              = 13,
        MINUS11000              = 14,
        MINUS15000              = 15,
        PLUS15000               = 16,
        PLUS11000               = 17,
        PLUS19000               = 18,
        PLUS29000               = 19,
        PLUS40000               = 20,
        PLUS37000               = 21,
        PLUS47000               = 22,
        PLUS53000       	= 23,
        PLUS65000       	= 24,
	PLUS59000       	= 25,
	PLUS73000       	= 26,
        PLUS79000               = 27,
        PLUS90000               = 28,
        PLUS83000               = 29,
        PLUS89000               = 30,
        PLUS97000               = 31 
}mod_hop_channels_t;

typedef enum
{
        MOD_DBPSK_50_PROT_D         = 21,
        MOD_DBPSK_400_PROT_D        = 24,
        MOD_DBPSK_3200_PROT_D       = 26,
        MOD_DBPSK_25600_PROT_D      = 28,
        MOD_DBPSK_50_PROT_E	    = 30,
        MOD_DBPSK_400_PROT_E	    = 31,
	MOD_DBPSK_3200_PROT_E	    = 32,
	MOD_DBPSK_25600_PROT_E	    = 33,
        MOD_DBPSK_100H_PROT_D       = 34,
        MOD_DBPSK_100H_PROT_E       = 35,
}mod_bitrate_s;

void wa1205mod_init();
void wa1205mod_isr(void);
void wa1205mod_set_hop_table(mod_hop_channels_t *hop_table);
void wa1205mod_set_bitrate(mod_bitrate_s bitrate);
void wa1205mod_send(uint8_t* data, mod_bitrate_s bitrate);
void wa1205mod_set_freq(uint32_t freq);
//void wa1205mod_PROTE_send(uint8_t* data, mod_bitrate_s bitrate);
















#endif