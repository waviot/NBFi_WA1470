#ifndef NBFI_MAC_H
#define NBFI_MAC_H
#include "nbfi_misc.h"

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
} nbfi_mac_protd_packet_t;
#pragma pack(pop)

typedef struct {
        nbfi_phy_channel_t phy;
        int16_t rssi;
        uint8_t snr;
} nbfi_mac_info_packet_t;



_Bool           NBFi_MAC_Match_ID(uint8_t * addr);
void            NBFi_MAC_RX_ProtocolD(nbfi_mac_protd_packet_t* pkt, nbfi_mac_info_packet_t* info);
nbfi_status_t   NBFi_MAC_TX_ProtocolE(nbfi_transport_packet_t* pkt);
nbfi_status_t   NBFi_MAC_TX_ProtocolD(nbfi_transport_packet_t* pkt);
nbfi_status_t   NBFi_MAC_RX();
nbfi_status_t   NBFi_MAC_TX(nbfi_transport_packet_t* pkt);

#endif