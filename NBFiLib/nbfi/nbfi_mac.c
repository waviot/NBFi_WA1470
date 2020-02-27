#include "nbfi.h"
#include "zcode.h"
#include "pcode.h"
#include <string.h>
#include <stdlib.h> 

#define memset_xdata memset
#define memcpy_xdata memcpy
#define memcpy_xdatageneric memcpy
#define memcpy_genericxdata memcpy

uint32_t last_pkt_crc = 0;

void NBFi_ParseReceivedPacket(nbfi_transport_frame_t *phy_pkt, nbfi_mac_info_packet_t* info);


static uint32_t NBFi_DL_ID()
{
  if(nbfi.dl_ID == 0) return *((uint32_t*)FULL_ID);
  else return nbfi.dl_ID;
}

void NBFi_MAC_RX_ProtocolD(nbfi_mac_protd_packet_t* packet, nbfi_mac_info_packet_t* info)
{
	if(NBFi_Crypto_Available())
	{
		if (!NBFI_Crypto_mic_check(&packet->flags, 9, packet->mic, &nbfi_iter.dl, packet->iter))
			return;
              
		NBFi_Crypto_Decode(&packet->flags, NBFi_DL_ID(), nbfi_iter.dl, 9);
		NBFi_MAC_Set_Iterator();
	}

	NBFi_ParseReceivedPacket((nbfi_transport_frame_t *)(&packet->flags), info);
}

static uint32_t NBFi_MAC_get_UL_freq(uint16_t lastcrc, _Bool parity)
{
	uint32_t ul_freq;
	switch(nbfi.tx_phy_channel)
	{
	case UL_DBPSK_50_PROT_D:
        case UL_DBPSK_400_PROT_D:
                ul_freq = nbfi.ul_freq_base + (((*((uint32_t*)FULL_ID)+lastcrc)%226)*100);
		if(parity) ul_freq = ul_freq + 27500;
                break;
	case UL_DBPSK_3200_PROT_D:
		ul_freq = nbfi.ul_freq_base + 1600 + (((*((uint32_t*)FULL_ID)+lastcrc)%210)*100);
		if(parity) ul_freq = ul_freq + 27500 - 1600;
		break;
	case UL_DBPSK_25600_PROT_D:
		ul_freq = nbfi.ul_freq_base + 25600;
		break;
	default:
          {
                uint32_t width = 6400 * (1 << nbfi.nbfi_freq_plan.ul_width);
                int32_t band_offset = width * nbfi.nbfi_freq_plan.ul_offset;
                if(nbfi.nbfi_freq_plan.ul_sign) band_offset = -band_offset;
                uint32_t bitrate = NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel);
                uint32_t gap = (width > (bitrate*2 + 2000))?(width - bitrate*2 - 2000)/2:0;
                ul_freq = nbfi.ul_freq_base + band_offset;
                uint32_t offset = ((*((uint32_t*)FULL_ID)+lastcrc)%256)*gap/255;
                if(parity) ul_freq = ul_freq + offset;
                else ul_freq = ul_freq - offset;

              break;
          }
	}

	return ul_freq;
}

static uint32_t NBFi_MAC_get_DL_freq()
{
  uint32_t dl_freq;
  if(nbfi.rx_freq == 0) 
  {
      uint32_t width = 102400 * (1 << nbfi.nbfi_freq_plan.dl_width);
      int32_t band_offset = width * nbfi.nbfi_freq_plan.dl_offset;
      if(nbfi.nbfi_freq_plan.dl_sign) band_offset = -band_offset;
      uint32_t bitrate = NBFi_Phy_To_Bitrate(nbfi.rx_phy_channel);
      uint32_t gap = (width > (bitrate*2 + 2000))?(width - bitrate*2 - 2000)/2:0;
      dl_freq = nbfi.dl_freq_base + band_offset;
      uint32_t offset = (NBFi_DL_ID()%256)*gap/255;
      if(NBFi_DL_ID()%2) dl_freq = dl_freq + offset;
                else dl_freq = dl_freq - offset;
  }  
  else 
	dl_freq = nbfi.rx_freq;
        
  return dl_freq;
}


nbfi_status_t NBFi_MAC_TX_ProtocolD(nbfi_transport_packet_t* pkt)
{
	const uint8_t protD_preambula[] = {0x97, 0x15, 0x7A, 0x6F};
	uint8_t ul_buf[64];
	uint8_t len = 0;
	static _Bool parity = 0;
	uint8_t lastcrc8;
	_Bool downlink;
	uint32_t tx_freq;

	memset_xdata(ul_buf,0,sizeof(ul_buf));


	for(int i=0; i<sizeof(protD_preambula); i++)
	{
		ul_buf[len++] = protD_preambula[i];
	}
        uint32_t *dl_id = &nbfi.dl_ID;
	switch(nbfi.tx_phy_channel)
	{
	case DL_DBPSK_50_PROT_D:
	case DL_DBPSK_400_PROT_D:
	case DL_DBPSK_3200_PROT_D:
	case DL_DBPSK_25600_PROT_D:
		ul_buf[len++] = dl_id[2];
		ul_buf[len++] = dl_id[1];
		ul_buf[len++] = dl_id[0];
		downlink = 1;
		break;
	default:
		ul_buf[len++] = FULL_ID[2];
		ul_buf[len++] = FULL_ID[1];
		ul_buf[len++] = FULL_ID[0];
		downlink = 0;
		break;

	}

	if(nbfi.tx_phy_channel == DL_DBPSK_50_PROT_D) 
		nbfi.tx_phy_channel = UL_DBPSK_50_PROT_D;
	else if(nbfi.tx_phy_channel == DL_DBPSK_400_PROT_D) 
		nbfi.tx_phy_channel = UL_DBPSK_400_PROT_D;
	else if(nbfi.tx_phy_channel == DL_DBPSK_3200_PROT_D) 
		nbfi.tx_phy_channel = UL_DBPSK_3200_PROT_D;
	else if(nbfi.tx_phy_channel == DL_DBPSK_25600_PROT_D) 
		nbfi.tx_phy_channel = UL_DBPSK_25600_PROT_D;

	ul_buf[len++] = pkt->phy_data.header;

	memcpy_xdatageneric(&ul_buf[len], pkt->phy_data.payload, pkt->phy_data_length);

	lastcrc8 = CRC8(&ul_buf[len], 8);

	if(NBFi_Crypto_Available())
	{
		NBFi_Crypto_Encode(&ul_buf[len], *((uint32_t*)FULL_ID), 0, 8);
	}
	
	len += 8;

        ul_buf[len++] = lastcrc8;

	last_pkt_crc = CRC32(ul_buf + 4, 13); 

	ul_buf[len++] = (uint8_t)(last_pkt_crc >> 16);
	ul_buf[len++] = (uint8_t)(last_pkt_crc >> 8);
	ul_buf[len++] = (uint8_t)(last_pkt_crc);

	if(nbfi.tx_freq)
	{
		tx_freq = nbfi.tx_freq ;
		parity = (nbfi.tx_freq > (nbfi.ul_freq_base));
	}
	else
		tx_freq = NBFi_MAC_get_UL_freq(lastcrc8, parity);

	if((nbfi.tx_phy_channel < UL_DBPSK_3200_PROT_D) && !downlink)
	{
		ZCODE_Append(&ul_buf[4], &ul_buf[len], parity);
	}
	else
	{
		ZCODE_Append(&ul_buf[4], &ul_buf[len], 1);
	}

	if(!nbfi.tx_freq) 
		parity = !parity;

	if((nbfi.additional_flags&NBFI_FLG_SEND_ALOHA) && parity) // For NRX send in ALOHA mode
	{

		NBFi_RF_Init(nbfi.tx_phy_channel, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

		NBFi_RF_Transmit(ul_buf, len + ZCODE_LEN, nbfi.tx_phy_channel, BLOCKING);

		nbfi_state.UL_total++;

		return NBFi_MAC_TX_ProtocolD(pkt);
	}
	
	NBFi_RF_Init(nbfi.tx_phy_channel, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

	NBFi_RF_Transmit(ul_buf, len + ZCODE_LEN, nbfi.tx_phy_channel, NONBLOCKING);

	nbfi_state.UL_total++;

	return OK;
}

nbfi_status_t NBFi_MAC_TX_ProtocolE(nbfi_transport_packet_t* pkt)
{
	const uint8_t protE_preambula[] = {0x97, 0x15, 0x7A, 0x6F};
	uint8_t ul_buf[20];
	uint8_t ul_buf_encoded[36];
	uint8_t len = 0;
	static _Bool parity = 0;
	uint32_t tx_freq;

	ul_buf[len++] = FULL_ID[3];
	ul_buf[len++] = FULL_ID[2];
	ul_buf[len++] = FULL_ID[1];
	ul_buf[len++] = FULL_ID[0];

	if(nbfi.tx_phy_channel == DL_DBPSK_50_PROT_E)
		nbfi.tx_phy_channel = UL_DBPSK_50_PROT_E;
	else if(nbfi.tx_phy_channel == DL_DBPSK_400_PROT_E)
		nbfi.tx_phy_channel = UL_DBPSK_400_PROT_E;
	else if(nbfi.tx_phy_channel == DL_DBPSK_3200_PROT_E)
		nbfi.tx_phy_channel = UL_DBPSK_3200_PROT_E;
	else if(nbfi.tx_phy_channel == DL_DBPSK_25600_PROT_E)
		nbfi.tx_phy_channel = UL_DBPSK_25600_PROT_E;
	
	ul_buf[len++] = nbfi_iter.ul;
	ul_buf[len++] = pkt->phy_data.header;

	memcpy(&ul_buf[len], pkt->phy_data.payload, pkt->phy_data_length);

	if(NBFi_Crypto_Available())
	{
		//uint32_t modem_id;		
		//modem_id = FULL_ID[0];
		//modem_id |= (uint32_t)FULL_ID[1] << 8;
		//modem_id |= (uint32_t)FULL_ID[2] << 16;
		NBFi_Crypto_Encode(&ul_buf[len - 1], *((uint32_t*)FULL_ID), nbfi_iter.ul, 9);
		len += 8;
	
		uint32_t mic = NBFi_Crypto_UL_MIC(&ul_buf[len - 9], 9);
		nbfi_iter.ul = NBFI_Crypto_inc_iter(nbfi_iter.ul);
		ul_buf[len++] = (uint8_t)(mic >> 16);
		ul_buf[len++] = (uint8_t)(mic >> 8);
		ul_buf[len++] = (uint8_t)(mic);
	}
	else
		len += 11;
        
	last_pkt_crc = CRC32(ul_buf, 17); 

	ul_buf[len++] = (uint8_t)(last_pkt_crc >> 16);
	ul_buf[len++] = (uint8_t)(last_pkt_crc >> 8);
	ul_buf[len++] = (uint8_t)(last_pkt_crc);

	if(nbfi.tx_freq)
	{
		tx_freq = nbfi.tx_freq ;
		parity = (nbfi.tx_freq > (nbfi.ul_freq_base));
	}
	else
		tx_freq = NBFi_MAC_get_UL_freq(ul_buf[len - 4], parity);


	for(int i=0; i<sizeof(protE_preambula); i++)
	{
		ul_buf_encoded[i] = protE_preambula[i];
	}
        
	PCODE_encode(8, &ul_buf[0], &ul_buf_encoded[4]);
        
	if(!nbfi.tx_freq) parity = !parity;

	if((nbfi.additional_flags&NBFI_FLG_SEND_ALOHA) && parity) // For NRX send in ALOHA mode
	{

		NBFi_RF_Init(nbfi.tx_phy_channel, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

		NBFi_RF_Transmit(ul_buf_encoded, 36, nbfi.tx_phy_channel, BLOCKING);

		nbfi_state.UL_total++;

		return NBFi_MAC_TX_ProtocolE(pkt);
	}
	
	NBFi_RF_Init(nbfi.tx_phy_channel, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

	NBFi_RF_Transmit(ul_buf_encoded, 36, nbfi.tx_phy_channel, NONBLOCKING);

	nbfi_state.UL_total++;

	NBFi_MAC_Set_Iterator();

	return OK;
}

/*
_Bool NBFi_MAC_Match_ID(uint8_t * addr)
{
	uint8_t i;
	for( i = 0; i != 3; i++)
		if(nbfi.temp_ID[i] != addr[i])
			break;
	if(i == 3)
		return 1;

	for(i = 0; i != 3; i++) 
		if(nbfi.broadcast_ID[i] != addr[i])
			break;
	if(i == 3)
		return 1;

	return 0;
}*/

nbfi_status_t NBFi_MAC_TX(nbfi_transport_packet_t* pkt)
{
	switch(nbfi.tx_phy_channel)
	{
	case UL_DBPSK_50_PROT_D:
	case UL_DBPSK_400_PROT_D:
	case UL_DBPSK_3200_PROT_D:
	case UL_DBPSK_25600_PROT_D:
	case DL_DBPSK_50_PROT_D:
	case DL_DBPSK_400_PROT_D:
	case DL_DBPSK_3200_PROT_D:
	case DL_DBPSK_25600_PROT_D:
		return NBFi_MAC_TX_ProtocolD(pkt);
	case UL_DBPSK_50_PROT_E:
	case UL_DBPSK_400_PROT_E:
	case UL_DBPSK_3200_PROT_E:
	case UL_DBPSK_25600_PROT_E:
	case DL_DBPSK_50_PROT_E:
	case DL_DBPSK_400_PROT_E:
	case DL_DBPSK_3200_PROT_E:
	case DL_DBPSK_25600_PROT_E:
		return NBFi_MAC_TX_ProtocolE(pkt);
	case UL_PSK_FASTDL:
	case UL_PSK_200:
	case UL_PSK_500:
	case UL_PSK_5000:
		break;
	}

	return OK;
}

nbfi_status_t NBFi_MAC_RX()
{
	return NBFi_RF_Init(nbfi.rx_phy_channel, (nbfi_rf_antenna_t)nbfi.rx_antenna, 0, NBFi_MAC_get_DL_freq());
}

void NBFi_MAC_Set_Iterator()
{
	if (__nbfi_set_iterator)
		__nbfi_set_iterator(&nbfi_iter);
}

void NBFi_MAC_Get_Iterator()
{
	if (__nbfi_get_iterator)
		__nbfi_get_iterator(&nbfi_iter);
}