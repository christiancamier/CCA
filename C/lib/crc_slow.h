/*
 * #@ "dhpu_crc_slow.c"
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
 * Function:    crcSlow()
 * 
 * Description: Compute the CRC of a given message.
 *
 * Notes:		
 *
 * Returns:		The CRC of the message.
 *
 *********************************************************************/
CRC_TYPE
crcSlow(const void *data, size_t nBytes)
{
	register uint8_t   *message;
	register CRC_TYPE   remainder;
	register size_t     byte;
	register uint8_t    bit;

	/*
	 * Perform modulo-2 division, a byte at a time.
	 */
	remainder = INITIAL_REMAINDER;
	message = (uint8_t *)data;
	for (byte = 0; byte < nBytes; byte += 1, message += 1)
	{
		/*
		 * Bring the next byte into the remainder.
		 */
		remainder ^= (REFLECT_DATA(*message) << (WIDTH - 8));

		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; bit -= 1)
		{
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT)	remainder = (remainder << 1) ^ POLYNOMIAL;
			else			remainder = (remainder << 1);
		}
	}

	/*
	 * The final remainder is the CRC result.
	 */
	return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
}
