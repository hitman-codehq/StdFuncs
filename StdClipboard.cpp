
#include "StdFuncs.h"
#include "StdClipboard.h"
#include "StdWindow.h"

/* Written: Tuesday 06-Jul-2010 7:42 am */

int RClipboard::Open(CWindow *a_poWindow)
{
	int RetVal;

	ASSERTM((a_poWindow != NULL), "RClipboard::Open() => Window passed in is not open");
	ASSERTM((a_poWindow->m_poWindow != NULL), "RClipboard::Open() => Native window passed in is not open");

#ifdef __amigaos4__

	RetVal = KErrGeneral;

#else /* ! __amigaos4__ */

	RetVal = (OpenClipboard(a_poWindow->m_poWindow)) ? KErrNone : KErrGeneral;

	if (RetVal != KErrNone)
	{
		Utils::Info("RClipboard::Open() => Unable to open clipboard");
	}

#endif /* ! RClipboard::UnlockData */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:44 am */

void RClipboard::Close()
{

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	DEBUGCHECK(CloseClipboard());

#endif /* __amigaos4__ */

}

/* Written: Wednesday 07-Jul-2010 6:37 am */

const char *RClipboard::GetNextLine(TInt *a_piLength, TBool *a_bHasEOL)
{
	const char *NextChar, *RetVal;

	/* Assume we are going to return the current line and that it has no EOL characters */

	NextChar = RetVal = m_pccCurrentData;
	*a_bHasEOL = EFalse;

	/* Find the end of the current line, being either NULL terminated or the EOL characters */

	while ((*NextChar) && (*NextChar != 0x0d) && (*NextChar != 0x0a))
	{
		++NextChar;
	}

	/* Save the length for the calling code */

	*a_piLength = (NextChar - m_pccCurrentData);

	/* Check for CR and LF characters and if present, note their presence and skip them */

	if (*NextChar == 0x0d)
	{
		*a_bHasEOL = ETrue;
		++NextChar;
	}

	if (*NextChar == 0x0a)
	{
		*a_bHasEOL = ETrue;
		++NextChar;
	}

	/* If neither characters nor any EOL characters were found then we are at the end of the */
	/* clipboard data so return that there are no more lines */

	if (NextChar == RetVal)
	{
		RetVal = NULL;
	}

	/* Save the current position in the string for use in the next call */

	m_pccCurrentData = NextChar;

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:47 am */

const char *RClipboard::LockData()
{
	const char *RetVal;
	HANDLE Handle;

	/* Assume failure */

	RetVal = NULL;

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	/* Check to see if there is any plain text available on the clipboard and if so, get a handle to it */

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if ((Handle = GetClipboardData(CF_TEXT)) != NULL)
		{
			/* Lock the handle into memory and return a ptr to it */

			if ((RetVal = m_pccData = m_pccCurrentData = (const char *) GlobalLock(Handle)) == NULL)
			{
				Utils::Info("RClipboard::LockData() => Unable to lock keyboard data into memory");
			}
		}
		else
		{
			Utils::Info("RClipboard::LockData() => Unable to get handle to clipboard data");
		}
	}
	else
	{
		Utils::Info("RClipboard::LockData() => No clipboard data available");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:49 am */

void RClipboard::UnlockData()
{
	ASSERTM((m_pccData != NULL), "RClipboard::UnlockData() => UnlockData() must not be called without LockData() being called");

#ifdef __amigaos4__

	GlobalUnlock(m_pccData);
	m_pccData = NULL;

#endif /* __amigaos4__ */

}
