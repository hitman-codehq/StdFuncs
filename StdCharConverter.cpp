
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

	m_textCodec = QTextCodec::codecForName("ISO 8859-15");

	return (m_textCodec != nullptr) ? KErrNone : KErrNoMemory;

#else /* ! QT_GUI_LIB */

	return KErrNone;

#endif /* ! QT_GUI_LIB */

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
 * @param	character		The 8859-15 character to be converted
 * @return	The UTF-8 value of the character, if successful, or 0
 */

std::pair<uint32_t, int> CStdCharConverter::toUTF8(int a_character)
{
	std::pair<uint32_t, int> retVal{0, 1};

#ifdef QT_GUI_LIB

	/* Convert the 8859-15 character to a Unicode code point */
	char buffer[2];
	buffer[0] = static_cast<char>(a_character);
	buffer[1] = '\0';

	QString unicode = m_textCodec->toUnicode(buffer);

	/* The returned string will only contain data if the character was successfully converted.  If it was */
	/* not, then just return a 0 character to indicate this */
	if (unicode.length() > 0)
	{
		/* The Unicode string contains the data as code points so convert it to 1 or 2 byte UTF-8 encoded characters */
		QByteArray UTF8 = unicode.toUtf8();

		retVal = std::pair<uint32_t, int>(static_cast<unsigned char>(UTF8[0]), 1);

		if (UTF8.size() > 1)
		{
			retVal.first |= static_cast<unsigned char>(UTF8[1]) << 8;
			++retVal.second;
		}
	}

#else /* ! QT_GUI_LIB */

	(void) a_character;

#endif /* ! QT_GUI_LIB */

	return retVal;
}
