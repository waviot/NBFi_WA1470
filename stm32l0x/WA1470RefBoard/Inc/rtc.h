#ifndef RTC_H
#define RTC_H

extern volatile uint32_t rtc_counter;

void RTC_init();
void RTC_GPIO_RTC_IRQ(void);
#endif