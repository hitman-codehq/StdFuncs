
#include "StdFuncs.h"
#include "StdCharConverter.h"

#ifdef QT_GUI_LIB
#include <QTextCodec>
#endif /* QT_GUI_LIB */

/**
 * Creates an instance of the CStdCharConverter class.
 * Allocates and initialises the class, providing two phase construction.
 *
 * @date	Saturday 11-Dec-2021 7:37 am, Santa Cruz de Tenerife holiday apartment
 * @return	A pointer to an initialised class instance if successful, else nullptr
 */

CStdCharConverter *CStdCharConverter::New()
{
	CStdCharConverter *retVal;

	if ((retVal = new CStdCharConverter()) != nullptr)
	{
		if (retVal->construct() != KErrNone)
		{
			delete retVal;
			retVal = nullptr;
		}
	}

	return retVal;
}

/**
 * Second phase constructor.
 * This is called by the New() wrapper to perform initialisation steps that can fail and need to be cleaned
 * up upon failure.
 *
 * @date	Saturday 11-Dec-2021 7:35 am, Santa Cruz de Tenerife holiday apartment
 * @return	KErrNone if successful, else KErrNoMemory
 */

int CStdCharConverter::construct()
{

#ifdef QT_GUI_LIB

	/* Create 2 codecs for converting between 8859-15 and UTF-8.  Each codec is only capable of converting to and */
	/* from its respective codec and 2 byte Unicode, so to convert in both directions between 8859-15 and UTF-8, */
	/* we need instances of both codecs */
	m_textCodec8859 = QTextCodec::codecForName("ISO 8859-15");
	m_textCodecUTF8 = QTextCodec::codecForName("UTF-8");

	return (m_textCodec8859 != nullptr && m_textCodecUTF8 != nullptr) ? KErrNone : KErrNoMemory;

#else /* ! QT_GUI_LIB */

	return KErrNone;

#endif /* ! QT_GUI_LIB */

}

/**
* Determine the size of a character.
* This method is a Unicode aware routine that will examine an in-memory character and determine
* its size in bytes.  For speed, this method uses a shortcut way of testing the size of the
* character, which assumes that the data is indeed UTF-8.  This could lead to errors if used
* with non UTF-8 data.  Thus, this method should only be used on data that is known to be UTF-8.
* There is also no check for buffer overflows - this is a low level routine so it is assumed that
* the caller is checking for this.
*
* Note that the routine assumes that the character passed in is in little endian format, hence the
* use of a memory pointer rather than an integral variable, to avoid problems on big endian systems.
*
* @date		Thursday 01-Feb-2018 9:41 am, Starbucks Friedrichstrasse
* @param	a_pcucBuffer	Pointer to the character to be checked
* @return	Return value	The size of the next character in bytes
*/

int CStdCharConverter::charSize(const unsigned char *a_buffer)
{
	int retVal;

	/* Get the first byte of the character to be checked */

	unsigned char character = *a_buffer;

	/* Apply some shortcut checks to determine the character's size, according to the rules */
	/* for the bitwise layout of UTF-8 characters */

	if ((character & 0xf1) == 0xf0)
	{
		retVal = 4;
	}
	else if ((character & 0xf0) == 0xe0)
	{
		retVal = 3;
	}
	else if ((character & 0xe0) == 0xc0)
	{
		retVal = 2;
	}
	else
	{
		retVal = 1;
	}

	return retVal;
}

/**
 * Converts a multibyte character into a string.
 * This UTF-8 aware function will convert a character containing between 1 and 4 bytes of little endian data
 * into a string.  Characters can usually be treated as an integer, without regard for the number of bytes of
 * data they contain, but sometimes a string representation is needed, to ensure that byte ordering and
 * alignment is machine independent.  The number of valid (non zero) bytes within the character will be
 * calculated by this function.
 *
 * This function will never produce an invalid string, but passing in the special character of 0 (NUL) will
 * result in an empty string being returned.
 *
 * @date	Saturday 09-Jul-2022 8:05 am, Oakwood Hotel & Apartments, Tokyo Azabu
 * @param	a_character		The character to be converted
 * @return	The character, represented as a string
 */

std::string CStdCharConverter::makeString(uint32_t a_character)
{
	int size;
	TUint mask;

	/* Determine the number of bytes that make up the character.  The character is encoded in little endian */
	/* format, even on big endian systems, to ensure that when Unicode is not in use the character byte is in */
	/* the lowest byte.  The upper bytes are zero, allowing easy counting */

	size = 0;
	mask = 0xff;

	while (a_character & mask)
	{
		++size;
		mask <<= 8;
	}

	return makeString(a_character, size);
}

/**
 * Converts a multibyte character into a string.
 * This UTF-8 aware function will convert a character containing between 1 and 4 bytes of little endian data
 * into a string.  Characters can usually be treated as an integer, without regard for the number of bytes of
 * data they contain, but sometimes a string representation is needed, to ensure that byte ordering and
 * alignment is machine independent.
 *
 * This function will never produce an invalid string, but passing in the special character of 0 (NUL) will
 * result in an empty string being returned.
 *
 * @date	Saturday 09-Jul-2022 8:06 am, Oakwood Hotel & Apartments, Tokyo Azabu
 * @param	a_character		The character to be converted
 * @param	a_size			The number of valid (non zero) bytes within the character
 * @return	The character, represented as a string
 */

std::string CStdCharConverter::makeString(uint32_t a_character, int a_size)
{
	std::string retVal;

	/* Unpack the character into a buffer in the order in which it would naturally appear in text, in order */
	/* that it can be printed */

	for (int index = 0; index < a_size; ++index)
	{
		retVal.push_back(a_character & 0xff);
		a_character >>= 8;
	}

	return retVal;
}

/**
 * Convert a UTF-8 character to 8859-15.
 * This method takes a character in the UTF-8 character set and returns its 8859-15 equivalent and
 * the size of that 8859-15 character in bytes.  The character is returned in a std::pair with .first
 * representing the character and .second its size in bytes.  If the character could not be converted
 * then a character value 0 will be returned, that should be considered invalid by calling code.
 *
 * @date	Saturday 07-May-2022 7:20 am, Oakwood Hotel & Apartments, Tokyo Azabu
 * @param	a_character		The UTF-8 character to be converted
 * @return	The 8859-15 value of the character, if successful, or 0
 */

std::pair<uint32_t, int> CStdCharConverter::to8859(uint32_t a_character)
{
	std::pair<uint32_t, int> retVal{0, 1};

#ifdef QT_GUI_LIB

	/* Convert the UTF-8 character to a Unicode code point */
	char buffer[5];
	buffer[0] = static_cast<char>(a_character);
	buffer[1] = static_cast<char>(a_character >> 8);
	buffer[2] = static_cast<char>(a_character >> 16);
	buffer[3] = static_cast<char>(a_character >> 24);
	buffer[4] = '\0';

	QString unicode = m_textCodecUTF8->toUnicode(buffer);

	/* The returned string will only contain data if the character was successfully converted.  If it was */
	/* not, then just return a 0 character to indicate this */
	if (unicode.length() > 0)
	{
		/* The Unicode string contains the data as code points so convert it to 1 or 2 byte UTF-8 encoded characters. */
		/* Because toLocal8Bit() will convert to whatever the current local locale is, and because the current local */
		/* locale is UTF-8, we have to temporarily change it to 8859-15 and then back again afterwards */
		QTextCodec::setCodecForLocale(m_textCodec8859);
		QByteArray UTF8 = unicode.toLocal8Bit();
		QTextCodec::setCodecForLocale(nullptr);

		retVal = std::pair<uint32_t, int>(static_cast<unsigned char>(UTF8[0]), 1);

		if (UTF8.size() > 1)
		{
			retVal.first |= static_cast<unsigned char>(UTF8[1]) << 8;
			++retVal.second;
		}
	}

#else /* ! QT_GUI_LIB */

	retVal.first |= a_character;

#endif /* ! QT_GUI_LIB */

	return retVal;
}

/**
 * Convert an 8859-15 character to UTF-8.
 * This method takes a character in the 8859-15 character set and returns its UTF-8 equivalent and
 * the size of that UTF-8 character in bytes.  The character is returned in a std::pair with .first
 * representing the character and .second its size in bytes.  If the character could not be converted
 * then a character value 0 will be returned, that should be considered invalid by calling code.
 *
 * Note that the character returned is encoded UTF-8 binary data, not a UTF-8 code point.  It can
 * thus be inserted directly into a UTF-8 string.
 *
 * @date	Saturday 11-Dec-2021 7:49 am, Santa Cruz de Tenerife holiday apartment
 * @param	a_character		The 8859-15 character to be converted
 * @return	The UTF-8 value of the character, if successful, or 0
 */

std::pair<uint32_t, int> CStdCharConverter::toUTF8(uint32_t a_character)
{
	std::pair<uint32_t, int> retVal{0, 1};

#ifdef QT_GUI_LIB

	/* Convert the 8859-15 character to a Unicode code point */
	char buffer[2];
	buffer[0] = static_cast<char>(a_character);
	buffer[1] = '\0';

	QString unicode = m_textCodec8859->toUnicode(buffer);

	/* The returned string will only contain data if the character was successfully converted.  If it was */
	/* not, then just return a 0 character to indicate this */
	if (unicode.length() > 0)
	{
		/* The Unicode string contains the data as code points so convert it to UTF-8 encoded character */
		QByteArray UTF8 = unicode.toUtf8();

		uint32_t character = 0;
		int size = UTF8.size() - 1;

		/* Iterate through the array of UTF-8 encoded bytes, packing them into a single little endian encoded */
		/* UTF-8 32 bit character */
		while (size >= 0)
		{
			character |= static_cast<uint8_t>(UTF8[size]) << size * 8;
			--size;
		}

		retVal = std::pair<uint32_t, int>(character, UTF8.size());
	}

#else /* ! QT_GUI_LIB */

	retVal.first |= a_character;

#endif /* ! QT_GUI_LIB */

	return retVal;
}
