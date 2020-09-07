/*
 * #@ "crc_fast.h"
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

/*********************************************************************
 *
 * Function:    crcFast()
 * 
 * Description: Compute the CRC of a given message.
 *
 * Notes:		crcInit() must be called first.
 *
 * Returns:		The CRC of the message.
 *
 *********************************************************************/
CRC_TYPE
crcFast(const void *data, size_t nBytes)
{
	register uint8_t  *message;
	register CRC_TYPE  remainder;
	register uint8_t   cdat;
	register size_t    byte;

	remainder = INITIAL_REMAINDER;
	message   = (uint8_t *)data;

	/*
	 * Divide the message by the polynomial, a byte at a time.
	 */
	for (byte = 0; byte < nBytes; byte += 1, message += 1)
	{
		cdat = REFLECT_DATA(*message) ^ (remainder >> (WIDTH - 8));
		remainder = CRC_TABLE[cdat] ^ (remainder << 8);
	}

	/*
	 * The final remainder is the CRC.
	 */
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
}
