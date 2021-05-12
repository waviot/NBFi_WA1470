
#ifndef NBFI_CRC_H
#define NBFI_CRC_H

uint16_t                            CRC16(uint8_t *buf, uint16_t len, uint16_t crc);
uint8_t                             CRC8(uint8_t* data, uint8_t len);
uint32_t                            CRC32(const uint8_t *buf, uint8_t len);

#endif // NBFI_CRC_H