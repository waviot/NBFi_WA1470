
#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "wtimer.h"
#include "radio.h"




struct wtimer_desc test_desc;
uint32_t fft[32];
uint32_t noise_tbl[32];


extern void (*__wa1205_enable_pin_irq)(void);
extern void (*__wa1205_disable_pin_irq)(void);

void send_data(struct wtimer_desc *desc) {

  //if(!NBFi_Packets_To_Send())
      //NBFi_Send("Hello everybody!", sizeof("Hello everybody!"));
  if(__wa1205_disable_pin_irq) __wa1205_disable_pin_irq();
  
  wa1205_spi_read(DEM_FFT_REAF_BUF, ((uint8_t*)fft), 32*4); 
  wa1205_spi_write8(DEM_FFT_MSB, 0x80 + 23);
  wa1205_spi_write8(DEM_FFT_REAF_BUF, 0);
  
  for(int i = 0; i!= 32; i++)
  {
    wa1205_spi_write8(DEM_NOISE_RD_CHAN, i);
    wa1205_spi_read(DEM_NOISE_VALUE, (uint8_t*)(&noise_tbl[i]), 3); 
    noise_tbl[i]&=0xffffff;
    
  }
  
   if(__wa1205_enable_pin_irq) __wa1205_enable_pin_irq();  
  
  ScheduleTask(desc, 0, RELATIVE, SECONDS(1));

}

extern uint8_t nbfi_lock;
void HAL_SYSTICK_Callback(void)
{
  if(!nbfi_lock) wtimer_runcallbacks();
}

int main(void)
{

  HAL_Init();

  SystemClock_Config();
  
  MX_GPIO_Init();
   
  radio_init();

  ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));

  
    
  while (1) 
  {     
      
      NBFi_ProcessRxPackets(1);
      
      //wa1205_test();
      
      if (wa1205_cansleep()&& NBFi_can_sleep()) 
      {
          //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      }
  }
}



