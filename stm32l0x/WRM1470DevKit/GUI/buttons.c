#include "buttons.h"
#include "adc.h"

//static uint8_t button_event_flags;  

#define BUT_S1_TRESHOLD 500
#define BUT_S2_TRESHOLD 1500
#define BUT_S3_TRESHOLD 2500
#define BUT_S4_TRESHOLD 3500



static uint32_t GetButtonsADCValue()
{
  uint32_t voltage, temp, ch8;
  ADC_get(&voltage, &temp, &ch8);
  return ch8;
}

void GUI_extend_activity();

button_e GUI_get_button()
{
   
  uint32_t ch8_value = GetButtonsADCValue();
  
  if(ch8_value < BUT_S1_TRESHOLD) 
  {
    //if(SB1_old_state) return NO_BUTTON_PRESSED;
    //SB1_old_state = true;
    GUI_extend_activity(true);
    return SB1;
  }
  else if(ch8_value < BUT_S2_TRESHOLD) 
  {
    //if(SB2_old_state) return NO_BUTTON_PRESSED;
    //SB2_old_state = true;
    GUI_extend_activity(true);
    return SB2;
  }
  else if(ch8_value < BUT_S3_TRESHOLD) 
  {
    //if(SB3_old_state) return NO_BUTTON_PRESSED;
    //SB3_old_state = true;
    GUI_extend_activity(true);
    return SB3;
  }
  else if(ch8_value < BUT_S4_TRESHOLD) 
  {
    //if(SB4_old_state) return NO_BUTTON_PRESSED;
    //SB4_old_state = true;
    GUI_extend_activity(true);
    return SB4;
  }
  //SB1_old_state = SB2_old_state = SB3_old_state = SB4_old_state = false;
  return NO_BUTTON_PRESSED;
}

  
bool GetButton1()
{
  static bool old_state = false;
  if(GUI_get_button() == SB1) 
  {
    if(old_state) return false;
    old_state = true;
    return true;
  }
  old_state = false;
  return false;
}

bool GetButton2()
{
  static bool old_state = false;
  if(GUI_get_button() == SB2) 
  {
    if(old_state) return false;
    old_state = true;
    return true;
  }
  old_state = false;
  return false;
}

bool GetButton3()
{
  static bool old_state = false;
  if(GUI_get_button() == SB3) 
  {
    if(old_state) return false;
    old_state = true;
    return true;
  }
  old_state = false;
  return false;
}

bool GetButton4()
{
  static bool old_state = false;
  if(GUI_get_button() == SB4) 
  {
    if(old_state) return false;
    old_state = true;
    return true;
  }
  old_state = false;
  return false;
}

