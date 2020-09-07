/*
 * #@"crc_ccitt_cumul.c"
 */

#define CRC_CCITT

#define crcStart(a)		cc_ccitt_crcStart(a)
#define crcAppnd(a, b, c)	cc_ccitt_crcAppnd(a, b, c)
#define crcClose(a)		cc_ccitt_crcClose(a)

#include "crc_cumul.h"

