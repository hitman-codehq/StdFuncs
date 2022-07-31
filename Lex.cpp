
#include "StdFuncs.h"
#include <string.h>
#include "Lex.h"

/**
 * Initialises the TLex class for destructive extraction of tokens.
 * When initialising the TLex instance with this constructor, subsequent parsing
 * of the tokens will be destructive in nature.  That is, the parsing can only be
 * done once and the contents of the string will be altered.  This method of
 * parsing is less flexible than non destructive parsing but is useful in situations
 * where one wishes to extract a simple series of white space separated tokens
 * without needing to allocate memory into which to place the tokens after extraction.
 *
 * @date	Saturday 24-Nov-2012 9:34 am
 * @param	a_pcString Ptr to string to be parsed for tokens
 */

TLex::TLex(char *a_pcString)
{
	/* Remember to set these in the other constructor as well */

	m_pccString = m_pccOriginalString = NULL;
	m_pcString = a_pcString;
	m_pccQuotes = "\"'";
	m_pccWhitespace = " \t";
	m_iLength = m_iOriginalLength = (TInt) strlen(a_pcString);
	m_iQuotesLength = 2;
	m_iWhitespaceLength = 2;
	m_bKeepQuotes = m_bKeepWhitespace = m_bKeepNonAlphaNum = EFalse;
}

/**
 * Initialises the TLex class for non destructive extraction of tokens.
 * When initialising the TLex instance with this constructor, subsequent parsing
 * of the tokens will be non destructive in nature.  That is, the parsing can be
 * done as many times desired and the contents of the string will not be altered.
 * This method of parsing is more flexible than destructive parsing but requires
 * the caller to subsequently store the extracted tokens somewhere.
 *
 * @date	Saturday 24-Nov-2012 9:30 am
 * @param	a_pccString	Ptr to string to be parsed for tokens
 * @param	a_iLength	The length of the string passed in, excluding NULL terminator
 */

TLex::TLex(const char *a_pccString, TInt a_iLength)
{
	/* Remember to set these in the other constructor as well */

	m_pccString = m_pccOriginalString = a_pccString;
	m_pcString = NULL;
	m_pccQuotes = "\"'";
	m_pccWhitespace = " \t";
	m_iLength = m_iOriginalLength = a_iLength;
	m_iQuotesLength = 2;
	m_iWhitespaceLength = 2;
	m_bKeepQuotes = m_bKeepWhitespace = m_bKeepNonAlphaNum = EFalse;
}

/**
 * Checks to see if a character is white space.
 * Checks a character against the contents of the white space list to see
 * if it is white space.  This function uses an internal (and user definable)
 * list to define what white space is, but also treats CR and LF implicitly
 * as white space.
 *
 * @date	Wednesday 05-Dec-2012 5:37 am
 * @param	a_cCharacter Character to be checked
 * @return	ETrue if a_cCharacter is white space, else EFalse
 */

TBool TLex::CheckWhitespace(char a_cCharacter)
{
	TInt Index;

	/* Scan through the white space list and see if the character passed */
	/* in is valid white space */

	for (Index = 0; Index < m_iWhitespaceLength; ++Index)
	{
		if ((a_cCharacter == m_pccWhitespace[Index]) || (a_cCharacter == '\r') || (a_cCharacter == '\n'))
		{
			break;
		}
	}

	return(Index < m_iWhitespaceLength);
}

/**
 * Counts the number of tokens contained in the TLex instance.
 * Iterates through the data loaded into the TLex instance and parses the tokens
 * without extracting them, in order to count how many there are.  When doing so,
 * this routine respects any whitespace settings that have been applied to the
 * instance.  Note that this is an expensive routine as it must dynamically parse
 * the contents of the TLex instance, and it will affect the state of the TLex
 * instance as it goes through the tokens, so it should not be used for such things
 * as the counter for a for loop, but its value should be instead be cached.
 * Note that this function can only operate on TLex instances that have been
 * initialised in non destructive mode.
 *
 * @date	Tuesday 23-04-2013 7:19 am
 * @return	Number of tokens contained in the TLex instance
 */

TInt TLex::Count()
{
	const char *Token;
	TInt Length, RetVal;

	ASSERTM((m_pccString != NULL), "TLex::Count() => Function operates only in non destructive mode");

	RetVal = 0;

	/* Reinitialise the instance data so that scanning starts at the beginning */

	m_pccString = m_pccOriginalString;
	m_iLength = m_iOriginalLength;

	/* Determine how many tokens are present */

	while ((Token = NextToken(&Length)) != NULL)
	{
		++RetVal;
	}

	/* Reinitialise the instance data so that future scanning starts at the beginning */

	m_pccString = m_pccOriginalString;
	m_iLength = m_iOriginalLength;

	return(RetVal);
}

/**
 * Extracts the next available token from the string.
 * Parses the string with which the TLex was initialised and returns a ptr to the
 * next available token in the string.  This token will be NULL terminated.  The
 * first call to this routine will return the first token in the string and successive
 * calls will return successive tokens.  This is a destructive routine in that it
 * "destroys" the string passed in by writing NULL terminators into it, moving
 * through the source string as tokens are extracted.
 *
 * @date	Monday 21-Jun-2010 6:51 am
 * @return	The next token available in the TLex instance
 */

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

/**
 * Extracts the next available token from the string.
 * Parses the string with which the TLex was initialised and returns a ptr to the
 * next available token in the string.  This token will NOT be NULL terminated but
 * its length will instead be returned in the variable pointed to by a_piLength.
 * In the case where no token is returned, the contents of a_piLength will be set
 * to 0.  This is a non destructive routine.
 *
 * @pre		a_piLength must not be NULL
 *
 * @date	Thursday 22-Nov-2012 6:12 am
 * @param	a_piLength	A ptr to a variable into which to place the length of the token
 * @return	The next token available in the TLex instance, or NULL if there are none
 */

const char *TLex::NextToken(TInt *a_piLength)
{
	char QuoteChar;
	const char *NextToken, *RetVal;
	TBool FoundQuotes;
	TInt Index, QuoteIndex;

	ASSERTM((m_pccString != NULL), "TLex::NextToken() => String to parse not initialised");
	ASSERTM((a_piLength != NULL), "TLex::NextToken() => Address of length variable must be given");

	/* Assume the token will be extracted from the start of the string */

	NextToken = RetVal = m_pccString;
	Index = 0;
	FoundQuotes = EFalse;
	QuoteChar = '"';

	/* Skip past any white space at the start of the string */

	while ((Index < m_iLength) && (CheckWhitespace(*NextToken)))
	{
		++NextToken;
		++Index;
	}

	/* If there was white space and the class is configured to keep white space then */
	/* we already have a token to return */

	if ((Index == 0) || (!(m_bKeepWhitespace)))
	{
		/* If the new token starts with a quote then extract up until the next quote and include any */
		/* white space found between the start and end quote characters */

		RetVal = NextToken;

		/* Check against the user defined list of quote characters */

		for (QuoteIndex= 0; QuoteIndex < m_iQuotesLength; ++QuoteIndex)
		{
			if (*NextToken == m_pccQuotes[QuoteIndex])
			{
				break;
			}
		}

		if (QuoteIndex < m_iQuotesLength)
		{
			QuoteChar = *NextToken;
			++NextToken;
			++Index;
			FoundQuotes = ETrue;

			/* Only skip the beginning " or ' if we are not configured to keep it */

			if (!(m_bKeepQuotes))
			{
				++RetVal;
			}

			/* Extract the string itself as the token */

			while ((Index < m_iLength) && (*NextToken != QuoteChar))
			{
				++NextToken;
				++Index;
			}

			/* Only skip the end " or ' if we are configured to keep it and if it actually exists */

			if ((m_bKeepQuotes) && (*NextToken == QuoteChar))
			{
				++NextToken;
				++Index;
			}
		}

		/* Otherwise just extract up until the next non alpha numeric or white space character */

		else if (m_bKeepNonAlphaNum)
		{
			/* If the next character is alpha numeric then extract the run of alpha numeric characters */
			/* as a token */

			if (isalnum((unsigned char) *NextToken))
			{
				while ((Index < m_iLength) && (isalnum((unsigned char) *NextToken)))
				{
					++NextToken;
					++Index;
				}
			}

			/* Otherwise extract the run of non alpha numeric characters as a token */

			else
			{
				while ((Index < m_iLength) && (!(isalnum((unsigned char) *NextToken)) && (!CheckWhitespace(*NextToken)) &&
					(*NextToken != '\"') && (*NextToken != '\'')))
				{
					++NextToken;
					++Index;
				}
			}
		}

		/* Otherwise just extract until the next white space character is found */

		else
		{
			while ((Index < m_iLength) && (!(CheckWhitespace(*NextToken))))
			{
				++NextToken;
				++Index;
			}
		}
	}

	/* If the token found contains any characters then determine its length */

	if (NextToken > RetVal)
	{
		*a_piLength = (TInt) (NextToken - RetVal);

		/* If we have found some white space then skip past it.  This is only */
		/* required for use by the destructive token extractor, which will put */
		/* a NULL terminator into what is currently pointed to by NextToken, */
		/* thus causing the next call to this function to fail */

		if (CheckWhitespace(*NextToken))
		{
			/* Only skip if we are not configured to treat white space as a */
			/* token.  Note that this is incompatible with the destructive */
			/* token extractor! */

			if (!(m_bKeepWhitespace))
			{
				++NextToken;
				++Index;
			}
		}

		/* If this is an end quote then skip it, but only if we are not keeping */
		/* quotes.  Otherwise if two quote separated tokens are next to one */
		/* another without whitespace, the start of the second token will be */
		/* skipped instead */

		else if ((*NextToken == QuoteChar) && (FoundQuotes) && (!(m_bKeepQuotes)))
		{
			++NextToken;
			++Index;
		}
	}

	/* Otherwise signal that no more tokens were found */

	else
	{
		*a_piLength = 0;
		RetVal = NULL;
	}

	/* Save the current position in the string for use in the next call */

	m_pccString = NextToken;
	m_iLength -= Index;

	return(RetVal);
}

/**
 * Moves the internal text ptr forwards by a certain number of characters.
 * This function should only be used if you really know what you are doing
 * as it is breaking C++ abstraction rules and is use-at-your-own-risk!
 * It is supplied only so that the Lex class can be used by more advanced
 * tokenising code.
 *
 * @date	Tuesday 08-Jan-2013 6:34 am, Vis à Vis Hotel, Lindau
 * @param	a_iLength	Number of characters to skip
 */

void TLex::MoveForwards(TInt a_iLength)
{
	m_pccString += a_iLength;
	m_iLength -= a_iLength;

	ASSERTM((m_iLength >= 0), "TLex::MoveForwards() => Moved forwards too far");
}

/**
 * Moves the internal text ptr backwards by a certain number of characters.
 * This function should only be used if you really know what you are doing
 * as it is breaking C++ abstraction rules and is use-at-your-own-risk!
 * It is supplied only so that the Lex class can be used by more advanced
 * tokenising code.
 *
 * @date	Tuesday 08-Jan-2013 6:24 am, Vis à Vis Hotel, Lindau
 * @param	a_iLength	Number of characters to skip backwards
 */

void TLex::MoveBackwards(TInt a_iLength)
{
	m_pccString -= a_iLength;
	m_iLength += a_iLength;
}

/**
 * Configures the TLex class such that it retains white space, quotes or both.
 * Note that if you use the destructive version of the TLex::NextToken() then
 * this function can cause incompatibilities with it, as the destructive NextToken()
 * depends on being able to write its NULL terminator into the white space.  If
 * you need to extract white space then you will need to use the non destructive
 * version of TLex::NextToken().
 *
 * @date	Tuesday 27-Nov-2012 5:52 am
 * @param	a_bKeepQuotes		ETrue to retain the " or ' quotation marks in extracted strings
 * @param	a_bKeepWhitespace	ETrue to return white space as a token
 * @param	a_bKeepNonAlphaNum	ETrue to return non alpha numeric characters as a token
 */

void TLex::SetConfig(TBool a_bKeepQuotes, TBool a_bKeepWhitespace, TBool a_bKeepNonAlphaNum)
{
	m_bKeepQuotes = a_bKeepQuotes;
	m_bKeepWhitespace = a_bKeepWhitespace;
	m_bKeepNonAlphaNum = a_bKeepNonAlphaNum;
}

/**
 * Sets the user definable quoted character list.
 * Sets the list of characters that are treated as delimeters for quoted strings.  By
 * default, these are the " and ' characters.
 *
 * @date	Friday 21-Mar-2014 7:14 am
 * @param	a_pccQuotes		Ptr to string containing the new quote characters.
 *							Contents must be valid for the duration of the class's use
 */

void TLex::SetQuotes(const char *a_pccQuotes)
{
	ASSERTM((a_pccQuotes != NULL), "TLex::SetQuotes() => Ptr to quote string can not be NULL");

	m_pccQuotes = a_pccQuotes;
	m_iQuotesLength = (TInt) strlen(a_pccQuotes);
}

/**
 * Sets the user definable white space character list.
 * Sets the white space character list so that other characters can be treated as
 * white space.  For example, to parse the string ".cpp;.c;.h" into its separate
 * tokens you would use a white space string of ";".  Note that white space is
 * treated specially by the destructive extraction routines and thus setting the
 * white space separator is only allowed for the non destructive routines.
 *
 * @date	Wednesday 05-Dec-2012 5:30 am
 * @param	a_pccWhitespace	Ptr to string containing the new white space characters
 *							Contents must be valid for the duration of the class's use
 */

void TLex::SetWhitespace(const char *a_pccWhitespace)
{
	ASSERTM((a_pccWhitespace != NULL), "TLex::SetWhitespace() => Ptr to white space can not be NULL");
	ASSERTM((m_pcString == NULL), "TLex::SetWhitespace() => Alternate white space can only be used for non destructive extraction");

	m_pccWhitespace = a_pccWhitespace;
	m_iWhitespaceLength = (TInt) strlen(a_pccWhitespace);
}
