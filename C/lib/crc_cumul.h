/*
 * #@ "crc_slow.c"
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

#include "crc_internal.h"

CRC_TYPE *
crcStart(CRC_TYPE *context)
{
	*context = INITIAL_REMAINDER;
	return context;
}

CRC_TYPE *
crcAppnd(CRC_TYPE *context, const void *data, size_t nBytes)
{
	register uint8_t  *message;
	register CRC_TYPE  remainder;
	register uint8_t   cdat;
	register size_t    byte;

	remainder = *context;

	message = (uint8_t *)data;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (byte = 0; byte < nBytes; byte += 1, message += 1)
	{
		cdat = REFLECT_DATA(*message) ^ (remainder >> (WIDTH - 8));
		remainder = CRC_TABLE[cdat] ^ (remainder << 8);
	}

	*context = remainder;
	return context;
}

CRC_TYPE
crcClose(CRC_TYPE *context)
{
	return (REFLECT_REMAINDER(*context) ^ FINAL_XOR_VALUE);
}
