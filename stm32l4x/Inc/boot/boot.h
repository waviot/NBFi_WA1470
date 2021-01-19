#ifndef BOOT_H
#define BOOT_H

#include <main.h>

typedef void (*pFunction)(void);
__ramfunc  void BOOT_boot(uint32_t start_addr);

#endif
