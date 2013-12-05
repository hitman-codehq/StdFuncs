
// TODO: CAW - Not sure if this is the best name for this

#include "StdFuncs.h"
#include "Lex.h"
#include "StdStringList.h"
#include <string.h>

/**
 * Appends one or more strings to the string array.
 * Parses a string for substrings, separated by the ';' character, and copies any
 * substrings found into an internal buffer.  These substrings are then kept track
 * of in an array, which can be accessed via the [] operator.
 *
 * @date	Wednesday 20-Nov-2013 6:37 am, Code HQ Ehinger Tor
 * @param	a_pccString	Ptr to a NULL terminated string to be parsed
 * @return	KErrNone if the string was parsed successfully
 * @return	KErrNoMemory if not enough memory was available
 */

TInt CStdStringList::Append(const char *a_pccString)
{
	char **Strings;
	const char *Token;
	TInt Index, Length, NumStrings, RetVal;

	/* Create a temporary TLex and configure it to use the ';' character as the */
	/* substring separator */

	TLex Lex(a_pccString, strlen(a_pccString));

	Lex.SetWhitespace(";");

	/* Determine how many substring are in this string */

	NumStrings = (m_iNumStrings + Lex.Count());

	/* Allocate an array of ptrs of just the required size and copy any already allocated */
	/* string ptrs into it */

	if ((Strings = new char *[NumStrings]) != NULL)
	{
		memcpy(Strings, m_ppcStrings, (m_iNumStrings * sizeof(char *)));

		/* Iterate through the string and break it up into individual substrings, placing */
		/* them into the array allocated above */

		Index = m_iNumStrings;
		RetVal = KErrNone;

		/* Keywords are separated by the ';' character so create and configure a */
		/* TLex instance for scanning */

		// TODO: CAW - Get rid of this
		//TLex Lex(a_pccString, strlen(a_pccString));

		//Lex.SetWhitespace(";");

		/* Extract all tokens from this line and add them to the array */

		while ((Token = Lex.NextToken(&Length)) != NULL)
		{
			if ((Strings[Index++] = Utils::DuplicateString(Token, Length)) == NULL)
			{
				RetVal = KErrNoMemory;

				break;
			}
		}

		// TODO: CAW - Error handling here + failure checking
		if (RetVal == KErrNone)
		{
			m_ppcStrings = Strings;
			m_iNumStrings = NumStrings;
		}
	}
	else
	{
		RetVal = KErrNoMemory;
	}

	return(RetVal);
}

/**
 * Frees the class's resources and puts it back to its initial state.
 * Frees up the strings owned by the class, as well as the array used to keep track
 * of the string ptrs.  After calling this function, the class instance will be back
 * to how it was when it was first initialised, and is able to be reused.
 *
 * @date	Wednesday 20-Nov-2013 6:45 am, Code HQ Ehinger Tor
 */

void CStdStringList::Reset()
{
	TInt Index;

	/* Free all strings owned by the class */

	for (Index = 0; Index < m_iNumStrings; ++Index)
	{
		delete [] m_ppcStrings[Index];
	}

	/* Also free the array used to retain their ptrs and indicate that there are zero */
	/* strings held by the class */

	delete [] m_ppcStrings;
	m_ppcStrings = NULL;

	m_iNumStrings = 0;
}

/**
 * Retrieves a string at a given index in the array.
 * Once the strings have been parsed into the array of strings, this function can be used
 * to access them.
 *
 * @pre		Index passed in must be between 0 and the value returned by CStdStringList::Count()
 *
 * @date	Tuesday 19-Nov-2013 7:55 am, Code HQ Ehinger Tor
 * @param	a_iIndex	Index of the string to be retrieved
 */

const char *CStdStringList::operator[](TInt a_iIndex)
{
	ASSERTM(((a_iIndex >= 0) && (a_iIndex < m_iNumStrings)), "CStdStringList::operator[]() => Index passed in is out of range");

	return(m_ppcStrings[a_iIndex]);
}
