#include "wa1205.h"
#include "wa1205dem.h"
#include "wtimer.h"
#include  <string.h>

#include "stm32l0xx_hal_conf.h"

#define DEM_MAS_SIZE	8

dem_packet_st dem_mas[DEM_MAS_SIZE];

dem_packet_info_st dem_info_mas[DEM_MAS_SIZE];

uint8_t	dem_mess_received;

struct wtimer_desc dem_processMessages_desc;

extern void (*__wa1205_enable_pin_irq)(void);
extern void (*__wa1205_disable_pin_irq)(void);
extern void (*__wa1205_data_received)(dem_packet_st *pkt, dem_packet_info_st * info);


dem_packet_st tmp_dem_mas[DEM_MAS_SIZE];
dem_packet_info_st tmp_dem_info_mas[DEM_MAS_SIZE];
uint8_t	tmp_dem_mess_received;

extern uint8_t (*__wa1205_get_irq_pin_state)(void);

uint8_t mas[128];
void wa1205dem_init()
{
    wa1205_spi_read(0x20, mas, 128);
    wa1205dem_set_hop_table(0x33333333);
    wa1205dem_set_bitrate(DBPSK_50_PROT_D);
    wa1205dem_set_alpha(10, 6);
    wa1205_spi_read(0x20, mas, 128);
}

static void  wa1205dem_process_messages(struct wtimer_desc *desc)
{

       
        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
	
        tmp_dem_mess_received = dem_mess_received;
        memcpy(tmp_dem_mas, dem_mas, sizeof(tmp_dem_mas)); 
        memcpy(tmp_dem_info_mas, dem_info_mas, sizeof(tmp_dem_info_mas)); 
        dem_mess_received = 0;
        memset(dem_mas, 0 , sizeof(dem_mas));
	memset(dem_info_mas, 0 , sizeof(dem_info_mas));

        if(__wa1205_enable_pin_irq) __wa1205_enable_pin_irq();      
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
        delay_ms(1000);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
        
	if(__wa1205_data_received)
		for(uint8_t i = 0; i != tmp_dem_mess_received; i++) __wa1205_data_received(&tmp_dem_mas[i], &tmp_dem_info_mas[i]);
	
}


void wa1205dem_isr(void) 
{
  
        dem_packet_st new_packet;
	
           
        do
        {
          
          wa1205_spi_read(DEM_RECEIVED_MES_BUF, (uint8_t*)(&new_packet), 32);
          
          wa1205_spi_write8(DEM_RECEIVED_MES_BUF, 0);
          
                    
          dem_mas[dem_mess_received] = new_packet;
          

          if(dem_mess_received == (DEM_MAS_SIZE - 1))  return;

          
          ScheduleTask(&dem_processMessages_desc,  wa1205dem_process_messages, RELATIVE, MILLISECONDS(5));
          
          
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
        }
        while (__wa1205_get_irq_pin_state && __wa1205_get_irq_pin_state());

}


void wa1205dem_set_bitrate(dem_bitrate_s bitrate)
{
        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
	switch(bitrate)
	{
	case DBPSK_50_PROT_D:
                wa1205_spi_write8(DEM_RX_MODE, 0);
		break;
	case DBPSK_400_PROT_D:
                wa1205_spi_write8(DEM_RX_MODE, 1);
		break;
	case DBPSK_3200_PROT_D:
                wa1205_spi_write8(DEM_RX_MODE, 2);
		break;
	case DBPSK_25600_PROT_D:
                wa1205_spi_write8(DEM_RX_MODE, 3);
		break;
        case DBPSK_100H_PROT_D:
                wa1205_spi_write8(DEM_RX_MODE, 4);
		break;
	}
        if(__wa1205_enable_pin_irq) __wa1205_enable_pin_irq();      
}

void wa1205dem_set_alpha(uint8_t decim, uint8_t shift)
{
        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
        wa1205_spi_write8(DEM_ALPHA_DECIM_F, decim);
        wa1205_spi_write8(DEM_ALPHA_SHIFT, shift);
        if(__wa1205_enable_pin_irq) __wa1205_enable_pin_irq(); 
}

void wa1205dem_set_hop_table(uint32_t hop_table)
{
        if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
        //uint32_t ht = 0x33333333;
  
        wa1205_spi_write(DEM_HOP_TABLE, ((uint8_t*)&hop_table), 4);
        if(__wa1205_enable_pin_irq) __wa1205_enable_pin_irq(); 
}