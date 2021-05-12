#ifndef UPDATE_H_
#define UPDATE_H_

#include <stdint.h>

#define AQUA3_FLASH_UPDATE_HEADER_LEN	128

#define AQUA3_FLASH_APP_START			0x08001000
#define AQUA3_FLASH_UPDATE_START		0x08010000
#define AQUA3_FLASH_UPDATE_END			(0x0801F000 + AQUA3_FLASH_UPDATE_HEADER_LEN)

#define UPDATE_FLASH_TIMEOUT			10000000

#define FLASH_PAGE_SIZE 0x800

typedef struct
{
	uint32_t ver;
	uint32_t ver_ext;
	uint32_t start_add;
	uint32_t end_add;
	uint32_t crc;
	uint32_t reserved[10];
	uint32_t crc_of_this_struct;
} soft_update_t;

enum
{
	UPDATE_CRC_STRUCT_ERROR,
	UPDATE_LEN_ERROR,
	UPDATE_END_ADDR_ERROR,
	UPDATE_CRC_ERROR,
	UPDATE_CRC_MISMATCH,
	UPDATE_APP_IS_ACTUAL,
};

void UpdateInternalFlashErase(unsigned int pageNumber);
void UpdateInternalFlashWrite(unsigned char *data, unsigned int address, unsigned int count);
uint8_t update_check(soft_update_t * su);
uint8_t update_apply(soft_update_t su);
void update_write_page_to_flash(uint16_t page, uint8_t * data);
void update_clear_header(void);
void update_cpy(void);
void update_clear(void);

#endif /* UPDATE_H_ */
