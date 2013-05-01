
#ifndef LEX_H
#define LEX_H

/**
 * A class for parsing text strings and extracting parts of them.
 * This class allows the parsing of text strings and the extraction of tokens from
 * those strings.  A token is defined as a group of printable characters surrounded
 * by whitespace.  It is possible for the user to redefine the characters that are
 * considered to be whitespace.  For example setting the whitespace to ; will allow
 * the extractions of tokens which are separated by this character.  This is useful
 * for parsing configuration lists.
 *
 * This class can operate in two ways: Destructive and non destructive.  Each method
 * has its pros and cons.  Destructive extraction results in the original string being
 * parsed actually being altered to have NULL terminators inserted in it.  This means
 * that the string can only be parsed once, but the tokens extracted can be used as is
 * without the caller having to allocate space for them.  Non destructive extraction
 * does not alter the original string, but the caller will have to find somewhere to
 * store the extracted tokens and they will not be NULL terminated.
 */

class TLex
{
private:

	const char	*m_pccString;			/**< Ptr to string to be parsed by non destructive routine */
	const char	*m_pccOriginalString;	/**< Cached copy of m_pccString */
	char		*m_pcString;			/**< Ptr to string to be parsed by destructive routine */
	const char	*m_pcWhitespace;		/**< Whitespace characters for which to check */
	TBool		m_bKeepQuotes;			/**< ETrue to keep the quote marks around strings */
	TBool		m_bKeepWhiteSpace;		/**< ETrue to treat white space as a token */
	TInt		m_iLength;				/**< Length of the string to be parsed */
	TInt		m_iOriginalLength;		/**< Cached copy of m_iLength */
	TInt		m_iWhitespaceLength;	/**< # of characters of white space to be checked */

public:

	TLex(const char *a_pccString, TInt a_iLength);

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
