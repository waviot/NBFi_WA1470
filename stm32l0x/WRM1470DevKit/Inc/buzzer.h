#ifndef BUZZER_H
#define BUZZER_H


extern  _Bool buzzer_enabled;

void BUZZER_Enable(_Bool en);
void BUZZER_Init(void);
void BUZZER_Set_Freq(uint32_t freq);


#endif