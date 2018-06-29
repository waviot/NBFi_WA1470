#ifndef _WA1205DEM_H
#define _WA1205DEM_H

#include <stdint.h>

/*
#define PER_SIZE          512
#define AXI_WR_SIZE       16*2
#define AXI_START_ADDR (DMEM_SIZE/2 + PER_SIZE)
#define AXI_RW_ADDR AXI_START_ADDR
#define AXI_RD_ADDR (AXI_START_ADDR+AXI_WR_SIZE)
#define AXI_LENGTH        16*2
#define AXI_RX_FD_ADDR (AXI_START_ADDR+AXI_WR_SIZE+AXI_LENGTH)
*/

typedef enum {
	DEMOD_CRC = 0,
	DEMOD_1_2 = 1
} DEMODE_TYPE;

#pragma pack(push, 1)
typedef struct {
	uint8_t	id_0;
	uint8_t	id_1;
	uint8_t	id_2;
	uint8_t	flags;
	uint8_t payload[8];
	uint8_t payload_crc;
	uint8_t crc0;
	uint8_t crc1;
	uint8_t crc2;
} dem_protd_st;


typedef struct {
	dem_protd_st packet;
	uint8_t freq;
	uint8_t dummy;
	uint8_t rot;
	uint32_t rssi;
	uint8_t rssi_39_32;
	uint8_t   noise0;
	uint8_t   noise1;
	uint8_t   noise2;
	uint8_t crc_or_zigzag;
} dem_packet_st;
#pragma pack(pop)

typedef struct {
	uint8_t	num_of_crc;
	uint8_t	num_of_zigzag;
} dem_packet_info_st;


typedef enum
{
	DBPSK_50_PROT_D		= 10,
	DBPSK_400_PROT_D	= 11,
	DBPSK_3200_PROT_D	= 12,
	DBPSK_25600_PROT_D	= 13,
        DBPSK_100H_PROT_D       = 18
}dem_bitrate_s;



//----------------------------------------------------------
// DEMODULATOR REGs
//----------------------------------------------------------
#define  DEM_RECEIVED_MES_BUF   0
#define  DEM_RESET              0x20
#define  DEM_RX_MODE            0x21
#define  DEM_DET_TRESHOLD       0x22
#define  DEM_ALPHA_DECIM_F      0x24
#define  DEM_ALPHA_SHIFT        0x25
#define  DEM_HOP_LENGTH         0x26
#define  DEM_NOISE_RD_CHAN      0x27
#define  DEM_NOISE_VALUE        0x28
#define  DEM_FFT_READY          0x2B
#define  DEM_FFT_MSB            0x2C
#define  DEM_PREAMBLE_ID        0x2D
#define  DEM_CRC_POLY           0x2E
#define  DEM_HOP_TABLE          0x32
#define  DEM_FFT_REAF_BUF       0x80

#define  DEM_BS3_FREQ           0x64
#define  DEM_BS3_FREQ_APPLY     0x7C



void wa1205dem_init();
void wa1205dem_isr(void);
void wa1205dem_set_bitrate(dem_bitrate_s bitrate);
void wa1205dem_set_alpha(uint8_t decim, uint8_t shift);
void wa1205dem_set_hop_table(uint32_t hop_table);

#endif