/*
 * #@"crc_crc32_cumul.c"
 */

#define CRC_CRC32

#define crcStart(a)		cc_crc32_crcStart(a)
#define crcAppnd(a, b, c)	cc_crc32_crcAppnd(a, b, c)
#define crcClose(a)		cc_crc32_crcClose(a)

#include "crc_cumul.h"

