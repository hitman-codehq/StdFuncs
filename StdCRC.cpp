
#include "StdFuncs.h"
#include "StdCRC.h"

#define POLYNOMIAL 0xd8
#define WIDTH 32
#define TOPBIT (1 << (WIDTH - 1))

/**
 * Initialises the CRC calculation class for use.
 * The CRC calculation class uses a precalculated table of 256 binary division remainders
 * to speed up the calculation of CRC values.  This table must be initialised before the
 * class can be used, by calling this function once.
 *
 * @date	Saturday 17-May-2014 10:50 am
 */

void RCRC::Init()
{
	TInt Dividend;
	TUint Bit, Remainder;

	/* Indicate that the class has been initialised */

	m_bInitialised = ETrue;

	/* Iterate through the 256 possible divisors, calculate the binary division remainder and */
	/* store it in the precalculation table for later use */

	for (Dividend = 0; Dividend < 256; ++Dividend)
	{
		Remainder = (Dividend << (WIDTH - 8));

		for (Bit = 8; Bit > 0; --Bit)
		{
			if (Remainder & TOPBIT)
			{
				Remainder = ((Remainder << 1) ^ POLYNOMIAL);
			}
			else
			{
				Remainder = (Remainder << 1);
			}
		}

		m_auiTable[Dividend] = Remainder;
	}
}

/**
 * Calculates the CRC of a block of data.
 * This function will iterate over an arbitrary sized block of data passed in and will calculate
 * its 32 bit CRC value.  This is a clean room implementation of a CRC calculation function, but the
 * value returned is the same as that returned by zlib's crc32() function.
 *
 * @pre		RCRC::Init() must have been previously called
 *
 * @date	Saturday 17-May-2014 11:00 am
 * @param	a_uiStartCRC	Value of CRC with which to start.  Use 0 to begin with
 * @param	a_pucBuffer		Ptr to the block of data for which to calculate the CRC
 * @param	a_uiSize		Size of the block of data passed in, in bytes
 * @return	The 32 CRC value of the block of data
 */

TUint RCRC::CRC32(TUint a_uiStartCRC, unsigned char *a_pucBuffer, TUint a_uiSize)
{
	unsigned char Data;
	TUint Index, RetVal;

	/* Check precondition for validity */

	ASSERTM(m_bInitialised, "RCRC::CRC32() => RCRC::Init() must be called before a CRC can be calculated");

	/* Iterate through the buffer passed in and calculate its CRC */

	RetVal = a_uiStartCRC;

	for (Index = 0; Index < a_uiSize; ++Index)
	{
		Data = (unsigned char) (a_pucBuffer[Index] ^ (RetVal >> (WIDTH - 8)));
		RetVal = (m_auiTable[Data] ^ (RetVal << 8));
	}

	return(RetVal);
}
