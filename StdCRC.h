
#ifndef STDCRC_H
#define STDCRC_H

/**
 * Class for calculating CRC values.
 * This class is used for calculating 32 bit CRC values from arbitrarily sized blocks of data.
 * The CRC values calculated are compatible with those calculated by the zlib crc32() function,
 * although this class is a clean room implementation of that function.
 *
 * To use it, the user must first call the RCRC::Init() function once in order to generate a lookup
 * table that is used to speed up the CRC generation process, and then RCRC::CRC32() may be called
 * as many times as desired in order to calculate the CRC value of one or more blocks of data.
 */

class RCRC
{
	TBool	m_bInitialised;		/**< ETrue if RCRC::Init() has been called */
	TUint	m_auiTable[256];	/**< Array of precalculated binary division remainders */

public:

	RCRC()
	{
		m_bInitialised = EFalse;
	}

	void Init();

	TUint CRC32(unsigned char *a_pucBuffer, TUint a_uiSize);
};

#endif /* ! STDCRC_H */
