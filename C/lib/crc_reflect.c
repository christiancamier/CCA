/*
 * #@ "dhpu_crc.c"
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

#define CRC_CRC32
#include "crc_internal.h"

/*********************************************************************
 *
 * Function:    __dhpu_crc_reflect()
 * 
 * Description: Reorder the bits of a binary sequence, by reflecting
 *		them about the middle position.
 *
 * Notes:	No checking is done that nBits <= 32.
 *
 * Returns:	The reflection of the original data.
 *
 *********************************************************************/

uint32_t
cc_crc_reflect(uint32_t data, uint8_t nBits)
{
	uint32_t reflection;
	uint8_t  bit;

	reflection = 0x00000000;

	/*
	 * Reflect the data about the center bit.
	 */
	for (bit = 0; bit < nBits; bit += 1)
	{
		/*
		 * If the LSB bit is set, set the reflection of it.
		 */
		if (data & 0x01)
		{
			reflection |= (1 << ((nBits - 1) - bit));
		}

		data = (data >> 1);
	}

	return reflection;

} /* reflect() */
