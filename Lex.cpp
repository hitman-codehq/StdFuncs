
#include "StdFuncs.h"
#include "Lex.h"

/* Written: Monday 21-Jun-2010 6:51 am */

char *TLex::NextToken()
{
	char *NextToken, *RetVal;

	/* Assume the token will be extracted from the start of the string */

	RetVal = m_pcString;

	/* Skip past any white space at the start of the string */

	while ((*RetVal) && ((*RetVal == ' ') || (*RetVal == '\t')))
	{
		++RetVal;
	}

	/* If the new token start with a " then extract up until the next " and include and */
	/* white space found between the start and end " characters */

	NextToken = RetVal;

	if (*NextToken == '"')
	{
		++NextToken;
		++RetVal;

		while ((*NextToken) && (*NextToken != '"'))
		{
			++NextToken;
		}
	}

	/* Otherwise just extract up until the next white space character */

	else
	{
		while ((*NextToken) && (*NextToken != ' ') && (*NextToken != '\t'))
		{
			++NextToken;
		}
	}

	/* Save the current position in the string for use in the next call */

	m_pcString = NextToken;

	/* If the token found contains any characters then NULL terminate it and point past the NULL */
	/* terminator so the ptr can be used in the next call */

	if (NextToken > RetVal)
	{
		*NextToken = '\0';
		++m_pcString;
	}

	/* Otherwise signal that no more tokens were found */

	else
	{
		RetVal = NULL;
	}

	return(RetVal);
}
