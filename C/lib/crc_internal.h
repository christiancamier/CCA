/*
 * #@ "dhpu_crc_internal.h"
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

#ifndef __CRC_INTERNAL_H__
#define __CRC_INTERNAL_H__

#include <CCA/crc.h>

/* Default is CRC32 */
#if !defined(CRC_CCITT)
# if !defined(CRC_CRC16)
#  if !defined(CRC_CRC32)
#   error "CRC_CCITT or CRC_16 or CRC_32 must be defined"
#  endif
# endif
#endif

#if defined(CRC_CCITT)

#define CRC_TYPE		cc_crc16_t
#define CRC_PRFX		cc_ccitt
#define CRC_NAME		"CRC-CCITT"
#define POLYNOMIAL		0x1021
#define INITIAL_REMAINDER	0xFFFF
#define FINAL_XOR_VALUE		0x0000
// #undef DO_REFLECT_DATA
// #undef DO_REFLECT_REMAINDER
#define CHECK_VALUE		0x29B1
#define CRC_TABLE		cc_crc_ccitt_table

#elif defined(CRC_CRC16)

#define CRC_TYPE		cc_crc16_t
#define CRC_PRFX		cc_crc16
#define CRC_NAME		"CRC-16"
#define POLYNOMIAL		0x8005
#define INITIAL_REMAINDER	0x0000
#define FINAL_XOR_VALUE		0x0000
#define DO_REFLECT_DATA
#define DO_REFLECT_REMAINDER
#define CHECK_VALUE		0xBB3D
#define CRC_TABLE		cc_crc_crc16_table

#elif defined(CRC_CRC32)

#define CRC_TYPE		cc_crc32_t
#define CRC_PRFX		cc_crc32
#define CRC_NAME		"CRC-32"
#define POLYNOMIAL		0x04C11DB7
#define INITIAL_REMAINDER	0xFFFFFFFF
#define FINAL_XOR_VALUE		0xFFFFFFFF
#define DO_REFLECT_DATA
#define DO_REFLECT_REMAINDER
#define CHECK_VALUE		0xCBF43926
#define CRC_TABLE		cc_crc_crc32_table

#else

#error "One of CRC_CCITT, CRC16, or CRC32 must be #define'd."

#endif

/* Global entities*/
extern uint32_t cc_crc_reflect(uint32_t data, uint8_t nBits);
extern cc_crc16_t const cc_crc_ccitt_table[256];
extern cc_crc16_t const cc_crc_crc16_table[256];
extern cc_crc32_t const cc_crc_crc32_table[256];

#define WIDTH    (8 * sizeof(CRC_TYPE))
#define TOPBIT   (1 << (WIDTH - 1))


#if defined(DO_REFLECT_DATA)
#define REFLECT_DATA(X)		((uint8_t)cc_crc_reflect((X), 8))
#else
#define REFLECT_DATA(X)		(X)
#endif

#if defined(DO_REFLECT_REMAINDER)
#define REFLECT_REMAINDER(X)	((CRC_TYPE)cc_crc_reflect((X), WIDTH))
#else
#define REFLECT_REMAINDER(X)	(X)
#endif

#endif /* !__DHPU_CRC_INTERNAL_H__ */
