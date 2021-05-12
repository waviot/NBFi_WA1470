#include "main.h"
#include "boot.h"

__ramfunc  void BOOT_boot(uint32_t start_addr)
{
	uint32_t jump_addr;
	pFunction jump_to;

	SystemInit();

	jump_addr = *(__IO uint32_t*) (start_addr + 4);
	jump_to = (pFunction) jump_addr;
	__set_MSP(*(__IO uint32_t*) start_addr);
	jump_to();
}
