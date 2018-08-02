

#ifndef LOG_H_
#define LOG_H_
#include <stdint.h>

extern char   log_string[];

void log_init(void);
void log_send_str(const char *str);
void log_send_str_len(const char *str, uint16_t len);
void log_send_to_sfmonitor(uint32_t *mas, uint16_t len);

#endif 
