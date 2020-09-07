/*
 * #@"crc_ccitt_cumul.c"
 */

#define CRC_CCITT

#define crcSlow(a, b) cc_ccitt_crcSlow(a, b)

#include "crc_slow.h"

