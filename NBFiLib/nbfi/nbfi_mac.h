#ifndef NBFI_MAC_H
#define NBFI_MAC_H

#pragma pack(push, 1)
typedef struct {
	uint8_t iter;
	uint8_t	flags;
	uint8_t payload[8];
	uint8_t mic[3];
	uint8_t crc[3];
} nbfi_mac_protd_packet_t;
#pragma pack(pop)

typedef struct {
	nbfi_phy_channel_t phy;
	int16_t rssi;
	uint8_t snr;
} nbfi_mac_info_packet_t;

void            NBFi_MAC_RX_ProtocolD(nbfi_mac_protd_packet_t* pkt, nbfi_mac_info_packet_t* info);
nbfi_status_t   NBFi_MAC_TX_ProtocolE(nbfi_transport_packet_t* pkt);
nbfi_status_t   NBFi_MAC_TX_ProtocolD(nbfi_transport_packet_t* pkt);
nbfi_status_t   NBFi_MAC_RX();
nbfi_status_t   NBFi_MAC_TX(nbfi_transport_packet_t* pkt);
void            NBFi_MAC_Set_Iterator();
void            NBFi_MAC_Get_Iterator();
nbfi_prot_t     NBFi_MAC_get_protocol_type(nbfi_phy_channel_t phy);
#endif
