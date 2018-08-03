#include "main.h"
#include "stm32_init.h"
#include "string.h"
#include "wtimer.h"
#include "radio.h"
#include "log.h"



struct wtimer_desc test_desc;
//uint32_t fft[32];
//uint32_t noise_tbl[32];


extern void (*__wa1205_enable_pin_irq)(void);
extern void (*__wa1205_disable_pin_irq)(void);

void send_data(struct wtimer_desc *desc) {

  if(!NBFi_Packets_To_Send())
     NBFi_Send("Hello everybody!", sizeof("Hello everybody!"));
 
  
  extern dem_bitrate_s current_rx_phy;
  
  if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
  {
    if(++current_rx_phy == 14) current_rx_phy = 10;
    dem_bitrate_s tmp_phy = current_rx_phy;
    wa1205dem_set_bitrate(tmp_phy);
  }

    
  static _Bool led_state = 1;
  
  if(led_state) 
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    led_state = 0;
  }
  else 
  {
    led_state = 1;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
  }
  ScheduleTask(desc, 0, RELATIVE, SECONDS(60));

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

  log_init();
  
  ScheduleTask(&test_desc, send_data, RELATIVE, SECONDS(1));
  
  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  
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



