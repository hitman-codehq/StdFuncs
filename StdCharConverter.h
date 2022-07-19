
#ifndef STDCHARCONVERTER_H
#define STDCHARCONVERTER_H

class QTextCodec;

class CStdCharConverter
{
private:

	QTextCodec	*m_textCodec;

public:

	static CStdCharConverter *New();

	int construct();

	std::pair<uint32_t, int> toUTF8(int a_character);

	QTextCodec *getTextCodec()
	{
		return m_textCodec;
	}
};

#endif /* ! STDCHARCONVERTER_H */
