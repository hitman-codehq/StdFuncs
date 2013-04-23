
#ifndef LEX_H
#define LEX_H

/* A class for parsing text string and extracting parts of them */

class TLex
{
private:

	const char	*m_pccString;		/* Ptr to string to be parsed by non destructive routine */
	const char	*m_pccOriginalString; /* Cached copy of m_pccString */
	char		*m_pcString;		/* Ptr to string to be parsed by destructive routine */
	const char	*m_pcWhitespace;	/* Whitespace characters for which to check */
	TBool		m_bKeepQuotes;		/* ETrue to keep the quote marks around strings */
	TBool		m_bKeepWhiteSpace;	/* ETrue to treat white space as a token */
	TInt		m_iLength;			/* Length of the string to be parsed */
	TInt		m_iOriginalLength;	/* Cached copy of m_iLength */
	TInt		m_iWhitespaceLength;/* # of characters of white space to be checked */

public:

	TLex(const char *a_pccString, TInt a_iLength)
	{
		/* Remember to set these in the other constructor as well */

		m_pccString = m_pccOriginalString = a_pccString;
		m_pcString = NULL;
		m_pcWhitespace = " \t";
		m_iLength = m_iOriginalLength = a_iLength;
		m_iWhitespaceLength = 2;
		m_bKeepQuotes = m_bKeepWhiteSpace = EFalse;
	}

	TLex(char *a_pcString);

	TBool CheckWhitespace(char a_cCharacter);

	TInt Count();

	char *NextToken();

	const char *NextToken(TInt *a_piLength);

	void MoveForwards(TInt a_iLength);

	void MoveBackwards(TInt a_iLength);

	void SetConfig(TBool a_bKeepQuotes, TBool a_bKeepWhiteSpace);

	void SetWhitespace(const char *a_pcWhitespace);
};

#endif /* ! LEX_H */
