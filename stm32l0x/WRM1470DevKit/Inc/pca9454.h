#ifndef PCA9454_H
#define PCA9454_H


#define EXT_OUTPIN_POWER_LED    0
#define EXT_OUTPIN_NBACKLIGHT   1
#define EXT_OUTPIN_LCD_A0       2
#define EXT_OUTPIN_LCD_RESET    3
#define EXT_OUTPIN_LCD_CS       4
#define EXT_OUTPIN_LCD_PWR      5
#define EXT_OUTPIN_ANT_SEL_V1   6
#define EXT_OUTPIN_ANT_SEL_V2   7


void PCA9454_init();

void PCA9454_set_out_pin(uint8_t pin);
void PCA9454_reset_out_pin(uint8_t pin);

#endif //PCA9454_H