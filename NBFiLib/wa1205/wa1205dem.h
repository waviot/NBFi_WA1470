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
	DBPSK_25600_PROT_D	= 13
}dem_bitrate_s;

/*
#define dem_ack        (*((volatile uint16_t *)AXI_RX_FD_ADDR))
#define test_ack       (*((volatile uint16_t *)AXI_RD_ADDR))
#define dem_packet       ((volatile dem_packet_st *) (AXI_RX_FD_ADDR))
#define test (uint16_t*)(0x1000)

//----------------------------------------------------------
// DEMODULATOR REGs
//----------------------------------------------------------
#define  DEM_RESET             (*(volatile unsigned char *) 0x01A0) //1 - reset
#define  DEM_CHAN_NUM_POW      (*(volatile unsigned char *) 0x01A1)
#define  DEM_SOFT_SYNC_THR     (*(volatile unsigned int  *) 0x01A2)
#define  DEM_ALPHA_DECIM_F     (*(volatile unsigned char *) 0x01A4)
#define  DEM_ALPHA_SHIFT       (*(volatile unsigned char *) 0x01A5)
#define  DEM_SUB_SAMPLE_DLY    (*(volatile unsigned int  *) 0x01A6)
#define  DEM_SHIFT_GEN_DDS_INC (*(volatile unsigned char *) 0x01A8)
#define  DEM_NOISE_RD_CHAN     (*(volatile unsigned char *) 0x01A9)
#define  DEM_NOISE_DATA_LO     (*(volatile unsigned char *) 0x01AA)
#define  DEM_NOISE_DATA_MI     (*(volatile unsigned char *) 0x01AB)
#define  DEM_NOISE_DATA_HI     (*(volatile unsigned char *) 0x01AC)

#define  DEM_NOISE_DATA_LO16   (*(volatile unsigned int  *) 0x01AA)




#define FREQ_SHIFT_NUM	3


//----------------------------------------------------------
// BS3 synthesizer config
//----------------------------------------------------------
#define  BS3_SYNT_CONFIG   (*(volatile unsigned int *) 0x01B0)



typedef void(*Dem_CALLBACK)(dem_packet_st*);
void omsp_dem_reset();
void omsp_dem_init(Dem_CALLBACK cb);
void omsp_dem_set_bitrate(dem_bitrate_s bitrate);
void omsp_dem_set_alpha(uint8_t decim, uint8_t shift);
uint32_t omsp_dem_read_noise(uint8_t chan);

*/
void wa1205dem_irs(void);
#endif