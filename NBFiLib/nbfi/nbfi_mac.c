#include "nbfi.h"
#include "zcode.h"
#include "pcode.h"
#include "preambula.h"


void NBFi_ParseReceivedPacket(nbfi_transport_frame_t *phy_pkt, nbfi_mac_info_packet_t* info);



void NBFi_MAC_RX_ProtocolD(nbfi_mac_protd_packet_t* packet, nbfi_mac_info_packet_t* info)
{
	if(NBFi_Crypto_Available())
	{
		if (!NBFI_Crypto_mic_check(&packet->flags, 9, packet->mic, &nbfi_iter.dl, packet->iter))
			return;

		NBFi_Crypto_Decode(&packet->flags, NBFi_DL_ID(), nbfi_iter.dl, 9);
		NBFi_MAC_Set_Iterator();
	}
        else
        {
                uint32_t crc32 = CRC32(&packet->flags, 9);
                for(uint8_t i = 0; i != 3; i++)
                  if(((uint8_t*)&crc32)[i] != packet->mic[2-i]) return;
        }

	NBFi_ParseReceivedPacket((nbfi_transport_frame_t *)(&packet->flags), info);
}

nbfi_prot_t  NBFi_MAC_get_protocol_type(nbfi_phy_channel_t phy)
{
	switch(phy)
	{
    case UL_DBPSK_50_PROT_C:
	case UL_DBPSK_400_PROT_C:
		return PROT_C;
	case UL_DBPSK_50_PROT_D:
	case UL_DBPSK_400_PROT_D:
	case UL_DBPSK_3200_PROT_D:
	case UL_DBPSK_25600_PROT_D:
	case DL_DBPSK_50_PROT_D:
	case DL_DBPSK_400_PROT_D:
	case DL_DBPSK_3200_PROT_D:
	case DL_DBPSK_25600_PROT_D:
    case DL_DBPSK_100H_PROT_D:
    case UL_DBPSK_100H_PROT_D:
		return PROT_D;
	case UL_DBPSK_50_PROT_E:
	case UL_DBPSK_400_PROT_E:
	case UL_DBPSK_3200_PROT_E:
	case UL_DBPSK_25600_PROT_E:
	case DL_DBPSK_50_PROT_E:
	case DL_DBPSK_400_PROT_E:
	case DL_DBPSK_3200_PROT_E:
	case DL_DBPSK_25600_PROT_E:
    case UL_DBPSK_100H_PROT_E:
		return PROT_E;
	case UL_PSK_FASTDL:
	case UL_PSK_200:
	case UL_PSK_500:
	case UL_PSK_5000:
        return PROT_AXSEM;
	}
	return PROT_VOID;
}

static uint32_t NBFi_MAC_get_UL_freq(uint16_t lastcrc, _Bool parity)
{
	uint32_t ul_freq;

    uint32_t width;

    if((nbfi.tx_phy_channel == UL_DBPSK_50_PROT_D) || (nbfi.tx_phy_channel == UL_DBPSK_400_PROT_D)||(nbfi.tx_phy_channel == UL_DBPSK_3200_PROT_D)||(nbfi.tx_phy_channel == UL_DBPSK_25600_PROT_D))
    {
        switch(nbfi.tx_phy_channel)
        {
            case  UL_DBPSK_50_PROT_D:
            case  UL_DBPSK_400_PROT_D:
                  width = 1600;
                  break;
            case  UL_DBPSK_3200_PROT_D:
                  width = 3200;
                  break;
            case  UL_DBPSK_25600_PROT_D:
                  width = 25600;
                  break;
        }
        int32_t band_offset = width * nbfi.nbfi_freq_plan.ul_offset;
        if(nbfi.nbfi_freq_plan.ul_sign) band_offset = -band_offset;
        uint32_t bitrate = NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel);
        uint32_t gap = 1500;
        uint32_t adjust_step = 800;
        ul_freq = nbfi.ul_freq_base + band_offset;
        uint32_t offset = ((*((uint32_t*)FULL_ID)+lastcrc)%256)*gap/255;
        int8_t adjust = nbfi.nbfi_freq_plan.ul_width;
        if(adjust&0x4) adjust|=0xF8;
        ul_freq = ul_freq + offset - gap/2 + adjust*adjust_step;

    }
    else
    {
       width = 6400 * (1 << nbfi.nbfi_freq_plan.ul_width);
       int32_t band_offset = width * nbfi.nbfi_freq_plan.ul_offset;
       if(nbfi.nbfi_freq_plan.ul_sign) band_offset = -band_offset;
       uint32_t bitrate = NBFi_Phy_To_Bitrate(nbfi.tx_phy_channel);
       uint32_t gap = (width > (bitrate*2 + 2000))?(width - bitrate*2 - 2000)/2:0;
       ul_freq = nbfi.ul_freq_base + band_offset;
       uint32_t offset = ((*((uint32_t*)FULL_ID)+lastcrc)%256)*gap/255;
       if(parity) ul_freq = ul_freq + offset;
       else ul_freq = ul_freq - offset;
    }

	return ul_freq;
}

static uint32_t NBFi_MAC_get_DL_freq()
{
  uint32_t dl_freq;
  if(nbfi.additional_flags&NBFI_FLG_GATEWAY_MODE) return nbfi.ul_freq_base;
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
        uint32_t mic_or_crc32;

        nbfi_phy_channel_t phy;

	_Bool downlink;
	uint32_t tx_freq;

        static uint32_t preamble;
        static uint32_t last_dl_add = 0;

	memset(ul_buf,0,sizeof(ul_buf));


        nbfi_ul_sent_status_t *pkt_status = NBFi_Get_UL_status(pkt->id, 1);
        uint8_t pkt_flags;
        if(pkt_status) pkt_flags = pkt_status->flags;
        else pkt_flags = 0;

	switch(nbfi.tx_phy_channel)
	{
	case DL_DBPSK_50_PROT_D:
	case DL_DBPSK_400_PROT_D:
	case DL_DBPSK_3200_PROT_D:
	case DL_DBPSK_25600_PROT_D:
                downlink = 1;
                if(last_dl_add != NBFi_DL_ID())
                {
                  last_dl_add = NBFi_DL_ID();
                  preamble = preambula(NBFi_DL_ID(), (uint32_t *)0, (uint32_t *)0);
                }
                if(pkt_flags&NBFI_UL_FLAG_DEFAULT_PREAMBLE)
                {
                  for(int i=0; i<sizeof(protD_preambula); i++)
                    ul_buf[len++] = protD_preambula[i];
                }
                else
                {
                  for(int i=0; i<sizeof(protD_preambula); i++)
                    ul_buf[len++] = ((uint8_t *)&preamble)[sizeof(protD_preambula) - 1 - i];
                }

                ul_buf[len++] = nbfi_iter.ul;
                ul_buf[len++] = pkt->phy_data.header;
                memcpy(&ul_buf[len], pkt->phy_data.payload, pkt->phy_data_length);

                if(NBFi_Crypto_Available()&&!(pkt_flags&NBFI_UL_FLAG_UNENCRYPTED))
                {
                  NBFi_Crypto_Encode(&ul_buf[len - 1], NBFi_DL_ID(), nbfi_iter.ul, 9);
                  len += 8;
                  mic_or_crc32 = NBFi_Crypto_UL_MIC(&ul_buf[len - 9], 9);
                  nbfi_iter.ul = NBFI_Crypto_inc_iter(nbfi_iter.ul);
                }
                else
                {
                        len += 8;
                        mic_or_crc32 = CRC32(&ul_buf[len - 9], 9);
                }
                ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 16);
                ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 8);
                ul_buf[len++] = (uint8_t)(mic_or_crc32);

		break;
	default:
          	downlink = 0;
                for(int i=0; i<sizeof(protD_preambula); i++)
                {
                  ul_buf[len++] = protD_preambula[i];
                }
                ul_buf[len++] = FULL_ID[2];
                ul_buf[len++] = FULL_ID[1];
                ul_buf[len++] = FULL_ID[0];

                ul_buf[len++] = pkt->phy_data.header;

                memcpy(&ul_buf[len], pkt->phy_data.payload, pkt->phy_data_length);

                lastcrc8 = CRC8(&ul_buf[len], 8);

                if(NBFi_Crypto_Available()&&!(pkt_flags&NBFI_UL_FLAG_UNENCRYPTED))
                {
                        NBFi_Crypto_Encode_D(&ul_buf[len], 8);

                }

                len += 8;

                ul_buf[len++] = lastcrc8;

                break;
	}

	mic_or_crc32 = CRC32(ul_buf + 4, 13);

	ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 16);
	ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 8);
	ul_buf[len++] = (uint8_t)(mic_or_crc32);

	if(nbfi.tx_freq)
	{
		tx_freq = nbfi.tx_freq ;
	}
	else
        {
          if(!downlink)
          {
            if(pkt_flags&NBFI_UL_FLAG_SEND_ON_CENTRAL_FREQ) tx_freq = nbfi.ul_freq_base;
            else tx_freq = NBFi_MAC_get_UL_freq(lastcrc8, parity);
          }
          else
          {
            if(pkt_flags&NBFI_UL_FLAG_SEND_ON_CENTRAL_FREQ) tx_freq = nbfi.dl_freq_base;
            else tx_freq = NBFi_MAC_get_DL_freq();
          }
        }

        phy = nbfi.tx_phy_channel;
        if(phy == DL_DBPSK_50_PROT_D)
          phy = UL_DBPSK_50_PROT_D;
	else if(phy == DL_DBPSK_400_PROT_D)
          phy = UL_DBPSK_400_PROT_D;
	else if(phy == DL_DBPSK_3200_PROT_D)
          phy = UL_DBPSK_3200_PROT_D;
	else if(phy == DL_DBPSK_25600_PROT_D)
          phy = UL_DBPSK_25600_PROT_D;


	//if((nbfi.tx_phy_channel < UL_DBPSK_3200_PROT_D) && !downlink)
	//{
	//	ZCODE_Append(&ul_buf[4], &ul_buf[len], (tx_freq > (nbfi.ul_freq_base)));
	//}
	//else
	//{
		ZCODE_Append(&ul_buf[4], &ul_buf[len], 1);
	//}

	if(!nbfi.tx_freq)
		parity = !parity;

	if((nbfi.additional_flags&NBFI_FLG_SEND_ALOHA) && parity) // For NRX send in ALOHA mode
	{

		NBFi_RF_Init(phy, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

		NBFi_RF_Transmit(ul_buf, len + ZCODE_LEN, phy, BLOCKING);

		nbfi_state.UL_total++;

		return NBFi_MAC_TX_ProtocolD(pkt);
	}

	NBFi_RF_Init(phy, (nbfi_rf_antenna_t)nbfi.tx_antenna, nbfi.tx_pwr, tx_freq);

	NBFi_RF_Transmit(ul_buf, len + ZCODE_LEN, phy, NONBLOCKING);

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
    uint32_t mic_or_crc32;

	memset(ul_buf,0,sizeof(ul_buf));

    nbfi_ul_sent_status_t *pkt_status = NBFi_Get_UL_status(pkt->id, 1);
    uint8_t pkt_flags;
    if(pkt_status) pkt_flags = pkt_status->flags;
    else pkt_flags = 0;


	ul_buf[len++] = FULL_ID[3];
	ul_buf[len++] = FULL_ID[2];
	ul_buf[len++] = FULL_ID[1];
	ul_buf[len++] = FULL_ID[0];


	ul_buf[len++] = nbfi_iter.ul;
	ul_buf[len++] = pkt->phy_data.header;

	memcpy(&ul_buf[len], pkt->phy_data.payload, pkt->phy_data_length);

	if(NBFi_Crypto_Available()&&!(pkt_flags&NBFI_UL_FLAG_UNENCRYPTED))
	{
		NBFi_Crypto_Encode(&ul_buf[len - 1], *((uint32_t*)FULL_ID), nbfi_iter.ul, 9);
		len += 8;

		mic_or_crc32 = NBFi_Crypto_UL_MIC(&ul_buf[len - 9], 9);
		nbfi_iter.ul = NBFI_Crypto_inc_iter(nbfi_iter.ul);
	}
	else
        {
            len += 8;
            mic_or_crc32 = CRC32(&ul_buf[len - 9], 9);

        }
        ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 16);
	ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 8);
	ul_buf[len++] = (uint8_t)(mic_or_crc32);

	mic_or_crc32 = CRC32(ul_buf, 17);

	ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 16);
	ul_buf[len++] = (uint8_t)(mic_or_crc32 >> 8);
	ul_buf[len++] = (uint8_t)(mic_or_crc32);

	if(nbfi.tx_freq)
	{
		tx_freq = nbfi.tx_freq ;
		parity = (nbfi.tx_freq > (nbfi.ul_freq_base));
	}
	else
        {
            if(pkt_flags&NBFI_UL_FLAG_SEND_ON_CENTRAL_FREQ) tx_freq = nbfi.ul_freq_base;
            else
		  	tx_freq = NBFi_MAC_get_UL_freq(ul_buf[len - 4], parity);
        }

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


nbfi_status_t NBFi_MAC_TX(nbfi_transport_packet_t* pkt)
{
	switch(NBFi_MAC_get_protocol_type(nbfi.tx_phy_channel))
	{
	case PROT_D:
		return NBFi_MAC_TX_ProtocolD(pkt);
	case PROT_E:
		return NBFi_MAC_TX_ProtocolE(pkt);
	default:
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
	if (nbfi_hal->__nbfi_set_iterator)
		nbfi_hal->__nbfi_set_iterator(&nbfi_iter);
}

void NBFi_MAC_Get_Iterator()
{
	if (nbfi_hal->__nbfi_get_iterator)
		nbfi_hal->__nbfi_get_iterator(&nbfi_iter);
}