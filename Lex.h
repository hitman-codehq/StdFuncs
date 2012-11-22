
#ifndef LEX_H
#define LEX_H

/* A class for parsing text string and extracting parts of them */

class TLex
{
private:

	const char	*m_pccString;	/* Ptr to string to be parsed by non destructive routine */
	char		*m_pcString;	/* Ptr to string to be parsed by destructive routine */
	TInt		m_iLength;		/* Length of the string to be parsed */

public:

	TLex(const char *a_pccString, TInt a_iLength)
	{
		m_pccString = a_pccString;
		m_iLength = a_iLength;
	}

	TLex(char *a_pcString)
	{
		m_pcString = a_pcString;
		m_iLength = strlen(a_pcString);
	}

	char *NextToken();

	const char *NextToken(TInt *a_piLength);
};

#endif /* ! LEX_H */
