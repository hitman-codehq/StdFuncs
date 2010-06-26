
#ifndef LEX_H
#define LEX_H

/* A class for parsing text string and extracting parts of them */

class TLex
{
private:

	char	*m_pcString;	/* Ptr to string to be parsed */

public:

	TLex(char *a_pcString)
	{
		m_pcString = a_pcString;
	}

	char *NextToken();
};

#endif /* ! LEX_H */
