
#include "StdFuncs.h"
#include "StdWildcard.h"

/* Written: Tuesday 17-Aug-2010 7:02 am */

TInt RWildcard::Open(const char *a_pccWildcard)
{
	m_pccWildcard = a_pccWildcard;

	return(KErrNone);
}

/* Written: Tuesday 17-Aug-2010 7:03 am */

void RWildcard::Close()
{
}

/* Written: Tuesday 17-Aug-2010 7:09 am */

TBool RWildcard::Match(const char *a_pccFileName)
{
	char NextChar;
	const char *Char, *Wildcard;
	TBool RetVal;

	RetVal = EFalse;

	Char = a_pccFileName;
	Wildcard = m_pccWildcard;

	do
	{
		if (*Wildcard == '*')
		{
			++Wildcard;
			NextChar = *Wildcard;

			while ((*Char) && (*Char != NextChar))
			{
				++Char;
			}
		}

		while ((*Char) && (*Wildcard) && (*Char == *Wildcard))
		{
			++Char;
			++Wildcard;
		}

		if ((*Char == '\0') && (*Wildcard == '\0'))
		{
			RetVal = ETrue;
		}
		else if ((*Char == '\0') || (*Wildcard == '\0'))
		{
			break;
		}
		else if ((*Wildcard != '*') && (*Char != *Wildcard))
		{
			break;
		}
	}
	while (!(RetVal));

	return(RetVal);
}
