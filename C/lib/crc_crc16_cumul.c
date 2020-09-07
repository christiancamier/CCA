/*
 * #@"crc_crc16_cumul.c"
 */

#define CRC_CRC16

#define crcStart(a)		cc_crc16_crcStart(a)
#define crcAppnd(a, b, c)	cc_crc16_crcAppnd(a, b, c)
#define crcClose(a)		cc_crc16_crcClose(a)

#include "crc_cumul.h"

