
#include "StdFuncs.h"
#include "Lex.h"

/* Written: Monday 21-Jun-2010 6:51 am */
/* @return	The next token available in the TLex instance */
/* Parses the string with which the TLex was initialised and returns a ptr to the */
/* next available token in the string.  This token will be NULL terminated.  the */
/* first call to this routine will return the first token in the string and successive */
/* calls will return successive tokens.  This is a destructive routine in that it */
/* "destroys" the string passed in by writing NULL terminators into it, moving */
/* through the source string as tokens are extraced */

char *TLex::NextToken()
{
	char *RetVal;
	TInt Offset;

	ASSERTM((m_pcString != NULL), "TLex::NextToken() => String to parse not initialised");

	/* In order to use the class for destructive and non destructive parsing, while */
	/* reusing parsing code ensuring that constness is respected by the non destructive */
	/* parsing, we do some trickery here.  It is ok to case these results as we know */
	/* the strings we are working with really are writeable */

	m_pccString = m_pcString;
	RetVal = (char *) NextToken(&Offset);
	m_pcString = (char *) m_pccString;

	/* If a token was found then NULL terminate it */

	if (RetVal)
	{
		RetVal[Offset] = '\0';
	}

	return(RetVal);
}

/* Written: Thursday 22-Nov-2012 6:12 am */
/* @passed	A ptr to a variable into which to place the length of the token */
/* @return	The next token available in the TLex instance */
/* Parses the string with which the TLex was initialised and returns a ptr to the */
/* next available token in the string.  This token will NOT be NULL terminated but */
/* its length will instead be returned in the variable pointed to by a_piLength. */
/* This is a non destructive routine */

const char *TLex::NextToken(TInt *a_piLength)
{
	const char *NextToken, *RetVal;
	TInt Index;

	ASSERTM((m_pccString != NULL), "TLex::NextToken() => String to parse not initialised");

	/* Assume the token will be extracted from the start of the string */

	RetVal = m_pccString;
	Index = 0;

	/* Skip past any white space at the start of the string */

	while ((Index < m_iLength) && ((*RetVal == ' ') || (*RetVal == '\t')))
	{
		++RetVal;
		++Index;
	}

	/* If the new token start with a " then extract up until the next " and include and */
	/* white space found between the start and end " characters */

	NextToken = RetVal;

	if (*NextToken == '"')
	{
		++NextToken;
		++RetVal;
		++Index;

		while ((Index < m_iLength) && (*NextToken != '"'))
		{
			++NextToken;
			++Index;
		}
	}

	/* Otherwise just extract up until the next white space character */

	else
	{
		while ((Index < m_iLength) && (*NextToken != ' ') && (*NextToken != '\t'))
		{
			++NextToken;
			++Index;
		}
	}

	/* If the token found contains any characters then determine its length and point past */
	/* the separator so the ptr can be used in the next call */

	if (NextToken > RetVal)
	{
		*a_piLength = (NextToken - RetVal);
		++NextToken;
		++Index;
	}

	/* Otherwise signal that no more tokens were found */

	else
	{
		RetVal = NULL;
	}

	/* Save the current position in the string for use in the next call */

	m_pccString = NextToken;
	m_iLength -= Index;

	return(RetVal);
}
