#include "wa1205dem.h"
#include "wtimer.h"
#define DEM_MAS_SIZE	8

dem_packet_st dem_mas[DEM_MAS_SIZE];

dem_packet_info_st dem_info_mas[DEM_MAS_SIZE];

uint8_t	dem_mess_received;

struct wtimer_desc dem_processMessages_desc;

extern void (*__wa1205_enable_pin_irq)(void);
extern void (*__wa1205_disable_pin_irq)(void);
extern void (*__wa1205_data_received)(dem_packet_st *pkt, dem_packet_info_st * info);

static void dem_process_messages(struct wtimer_desc *desc)
{

        dem_packet_st tmp_dem_mas[DEM_MAS_SIZE];
        dem_packet_info_st tmp_dem_info_mas[DEM_MAS_SIZE];
        uint8_t	tmp_dem_mess_received;
        
        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
	
        tmp_dem_mess_received = dem_mess_received;
        memcpy(tmp_dem_mas, dem_mas, sizeof(tmp_dem_mas)); 
        memcpy(tmp_dem_info_mas, dem_info_mas, sizeof(tmp_dem_info_mas)); 
        dem_mess_received = 0;
        memset(dem_mas, 0 , sizeof(dem_mas));
	memset(dem_info_mas, 0 , sizeof(dem_info_mas));

        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();      
        

	if(__wa1205_data_received)
		for(uint8_t i = 0; i != tmp_dem_mess_received; i++) __wa1205_data_received(&tmp_dem_mas[i], &tmp_dem_info_mas[i]);
	
}


void wa1205dem_irs(void) 
{
  
        dem_packet_st new_packet;
	
        //new_packet = read_from_spi();
        
        dem_mas[dem_mess_received] = new_packet;
        

	if(dem_mess_received == (DEM_MAS_SIZE - 1))  return;

	
        ScheduleTask(&dem_processMessages_desc, dem_process_messages, RELATIVE, MILLISECONDS(5));
        
        uint16_t id = (*((uint16_t*)(&dem_mas[dem_mess_received])));
	uint8_t i;
	for(i = 0; i < dem_mess_received; i++)
	{
		if( (*((uint16_t*)&(dem_mas[i]))) == id)
			if(dem_mas[i].packet.flags == dem_mas[dem_mess_received].packet.flags)
			{
				break;
			}
	}

	if(new_packet.crc_or_zigzag)
	{
		dem_info_mas[i].num_of_zigzag++;
	}
	else dem_info_mas[i].num_of_crc++;
        
	if(i == dem_mess_received)
	{
		dem_mess_received++;
	}
	else
	{
		if((dem_mas[i].rssi_39_32 < new_packet.rssi_39_32))
		{
			dem_mas[i] = dem_mas[dem_mess_received];
		}
		else if((dem_mas[i].rssi_39_32 == new_packet.rssi_39_32) && (dem_mas[i].rssi < new_packet.rssi))
		{
			dem_mas[i] = dem_mas[dem_mess_received];
		}
	}

	//dem_ack = 0;
}


