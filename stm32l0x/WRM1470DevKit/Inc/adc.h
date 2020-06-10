#ifndef ADC_H
#define ADC_H

void ADC_init();
int ADC_get(uint32_t * voltage, uint32_t * temp, uint32_t *ch8);

#endif