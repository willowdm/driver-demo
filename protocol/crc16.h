#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>

#define CRC_INIT (0xFFFF)

uint16_t step_crc16_ccitt(uint8_t byte, uint16_t crc_pr);

uint16_t crc16_ccitt(uint8_t* bytes, uint16_t len);

#endif