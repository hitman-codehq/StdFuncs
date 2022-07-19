
#ifndef STDCHARCONVERTER_H
#define STDCHARCONVERTER_H

class QTextCodec;

/**
 * A class enabling conversion of characters between 8859-15 and UTF-8.
 * Client software that needs to convert characters between character sets can do so with the help of this class.
 * The actual conversion is performed by the underlying platform, and is thus not available on all operating systems.
 * For those that do not support conversion, a NULL character will be returned.
 */

class CStdCharConverter
{
private:

	QTextCodec	*m_textCodec8859;		/**< Codec for working with 8859-15 text */
	QTextCodec	*m_textCodecUTF8;		/**< Codec for working with UTF-8 text */

public:

	static CStdCharConverter *New();

	int construct();

	std::pair<uint32_t, int> to8859(uint32_t a_character);

	std::pair<uint32_t, int> toUTF8(uint32_t a_character);

	QTextCodec *get8859TextCodec()
	{
		return m_textCodec8859;
	}

	QTextCodec *getUTF8TextCodec()
	{
		return m_textCodecUTF8;
	}
};

#endif /* ! STDCHARCONVERTER_H */
