#ifndef _wa1470MOD_H
#define _wa1470MOD_H


//----------------------------------------------------------
// MODULATOR REGs
//----------------------------------------------------------
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
        MOD_MINUS97000          = 0,
        MOD_MINUS89000          = 1,
        MOD_MINUS83000          = 2,
        MOD_MINUS90000          = 3,
	MOD_MINUS79000          = 4,
	MOD_MINUS73000          = 5,
	MOD_MINUS59000      	= 6,
	MOD_MINUS65000      	= 7,
        MOD_MINUS53000      	= 8,
        MOD_MINUS47000      	= 9,
        MOD_MINUS37000      	= 10,
        MOD_MINUS40000      	= 11,
        MOD_MINUS29000          = 12,
        MOD_MINUS19000          = 13,
        MOD_MINUS11000          = 14,
        MOD_MINUS15000          = 15,
        MOD_PLUS15000           = 16,
        MOD_PLUS11000           = 17,
        MOD_PLUS19000           = 18,
        MOD_PLUS29000           = 19,
        MOD_PLUS40000           = 20,
        MOD_PLUS37000           = 21,
        MOD_PLUS47000           = 22,
        MOD_PLUS53000       	= 23,
        MOD_PLUS65000       	= 24,
	MOD_PLUS59000       	= 25,
	MOD_PLUS73000       	= 26,
        MOD_PLUS79000           = 27,
        MOD_PLUS90000           = 28,
        MOD_PLUS83000           = 29,
        MOD_PLUS89000           = 30,
        MOD_PLUS97000           = 31 
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

void wa1470mod_init();
void wa1470mod_isr(void);
void wa1470mod_set_hop_table(mod_hop_channels_t *hop_table);
void wa1470mod_set_bitrate(mod_bitrate_s bitrate);
void wa1470mod_send(uint8_t* data, mod_bitrate_s bitrate);
void wa1470mod_set_freq(uint32_t freq);
//void wa1470mod_PROTE_send(uint8_t* data, mod_bitrate_s bitrate);
















#endif