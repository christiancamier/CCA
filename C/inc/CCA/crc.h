/*
 * #@ "crc.h"
 *
 * Author : Christian CAMIER (chcamier@free.fr)
 *
 * This code is an adaptation of the Michael Barr's implementation for
 * the DHPU project.
 *
 * Form more informations see :
 *	http://www.netrino.com/Connecting/2000-01/index.php
 *
 *
 * History :
 * ---------
 *
 * Jul 26, 2007: Christian CAMIER
 *	v1.0 :	Creation
 */

#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

typedef uint16_t cc_crc16_t;
typedef uint32_t cc_crc32_t;

/* CCITT_CRC 16 functions */
extern cc_crc16_t  cc_ccitt_crcSlow(const void *data, size_t nBytes);
extern cc_crc16_t  cc_ccitt_crcFast(const void *data, size_t nBytes);
extern cc_crc16_t *cc_ccitt_crcStart(cc_crc16_t *context);
extern cc_crc16_t *cc_ccitt_crcAppnd(cc_crc16_t *context, const void *data, size_t nBytes);
extern cc_crc16_t  cc_ccitt_crcClose(cc_crc16_t *context);

/* CRC 16 functions */
extern cc_crc16_t  cc_crc16_crcSlow(const void *data, size_t nBytes);
extern cc_crc16_t  cc_crc16_crcFast(const void *data, size_t nBytes);
extern cc_crc16_t *cc_crc16_crcStart(cc_crc16_t *context);
extern cc_crc16_t *cc_crc16_crcAppnd(cc_crc16_t *context, const void *data, size_t nBytes);
extern cc_crc16_t  cc_crc16_crcClose(cc_crc16_t *context);

/* CRC 32 functions */
extern cc_crc32_t  cc_crc32_crcSlow(const void *data, size_t nBytes);
extern cc_crc32_t  cc_crc32_crcFast(const void *data, size_t nBytes);
extern cc_crc32_t *cc_crc32_crcStart(cc_crc32_t *context);
extern cc_crc32_t *cc_crc32_crcAppnd(cc_crc32_t *context, const void *data, size_t nBytes);
extern cc_crc32_t  cc_crc32_crcClose(cc_crc32_t *context);

#ifdef __cplusplus
}
#endif

#endif /* !__CRC_H__ */
