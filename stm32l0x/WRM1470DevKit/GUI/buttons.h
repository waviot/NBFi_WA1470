#ifndef BUTTONS_H
#define BUTTONS_H

#include "stdbool.h"

typedef enum
{
  NO_BUTTON_PRESSED,
  SB1,
  SB2,
  SB3,
  SB4
} button_e;

button_e GUI_get_button();
bool GetButton1();
bool GetButton2();
bool GetButton3();
bool GetButton4();
//void ResetButtonFlags(uint8_t flag);
//uint8_t GetButtonState();

#endif