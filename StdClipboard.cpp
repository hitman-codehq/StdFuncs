
#include "StdFuncs.h"
#include <string.h>
#include "StdClipboard.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/textclip.h>

#endif /* __amigaos4__ */

/* Written: Tuesday 06-Jul-2010 7:42 am */

int RClipboard::Open(CWindow *a_poWindow)
{
	int RetVal;

	ASSERTM((a_poWindow != NULL), "RClipboard::Open() => Window passed in is not open");
	ASSERTM((a_poWindow->m_poWindow != NULL), "RClipboard::Open() => Native window passed in is not open");

#ifdef __amigaos4__

	RetVal = KErrNone;

#else /* ! __amigaos4__ */

	RetVal = (OpenClipboard(a_poWindow->m_poWindow)) ? KErrNone : KErrGeneral;

	if (RetVal != KErrNone)
	{
		Utils::Info("RClipboard::Open() => Unable to open clipboard");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:44 am */

void RClipboard::Close()
{

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	DEBUGCHECK(CloseClipboard(), "RClipboard::Close() => Unable to close clipboard");

#endif /* __amigaos4__ */

}

/* Written: Wednesday 07-Jul-2010 6:37 am */

const char *RClipboard::GetNextLine(TInt *a_piLength, TBool *a_bHasEOL)
{
	const char *NextChar, *RetVal;

	/* Assume we are going to return the current line and that it has no EOL characters */

	NextChar = RetVal = m_pccCurrentGetData;
	*a_bHasEOL = EFalse;

	/* Find the end of the current line, being either NULL terminated or the EOL characters */

	while ((*NextChar) && (*NextChar != 0x0d) && (*NextChar != 0x0a))
	{
		++NextChar;
	}

	/* Save the length for the calling code */

	*a_piLength = (NextChar - m_pccCurrentGetData);

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

	m_pccCurrentGetData = NextChar;

	return(RetVal);
}

/* Written: Thursday 08-Jul-2010 7:06 am */

int RClipboard::SetDataStart(int a_iMaxLength)
{
	int RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#ifdef __amigaos4__

	/* Allocate a temporary buffer into which the client can write its data */

	if ((m_pcSetData = new char[a_iMaxLength]) != NULL)
	{
		RetVal = KErrNone;
		m_iDataSize = a_iMaxLength;
	}

#else /* ! __amigaos4__ */

	/* Empty the clipboard of its previous contents, thus also taking ownership of it */

	if (EmptyClipboard())
	{
		/* Allocate a global moveable memory block into which to copy the data being inserted, */
		/* lock it temporarily into memory and copy the data into it.  We allocate an extra */
		/* byte to NULL terminate the memory block to indicate the end, or Windows will to do */
		/* funny things to the end of the data, like overwriting an LF with a NULL terminator */

		if ((m_poHandle = GlobalAlloc(GMEM_MOVEABLE, (a_iMaxLength + 1))) != NULL)
		{
			if ((m_pcSetData = (char *) GlobalLock(m_poHandle)) != NULL)
			{
				RetVal = KErrNone;
				m_pcSetData[a_iMaxLength] = '\0';
			}
			else
			{
				Utils::Info("RClipboard::SetDataStart() => Unable to copy clipboard data");

				GlobalFree(m_poHandle);
				m_poHandle = NULL;
			}
		}
		else
		{
			Utils::Info("RClipboard::SetDataStart() => Unable to allocate memory for clipboard");
		}
	}
	else
	{
		Utils::Info("RClipboard::SetDataStart() => Unable to claim ownership of clipboard");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 10-Jul-2010 12:43 pm */

void RClipboard::AppendData(const char *a_pcData, int a_iOffset, int a_iLength)
{
	ASSERTM((m_pcSetData != NULL), "RClipboard::AppendData() => SetDataStart() must be called first");

	memcpy((m_pcSetData + a_iOffset), a_pcData, a_iLength);
}

/* Written: Saturday 10-Jul-2010 1:34 pm */

void RClipboard::SetDataEnd()
{
	ASSERTM((m_pcSetData != NULL), "RClipboard::SetDataEnd() => SetDataStart() must be called first");

#ifdef __amigaos4__

	/* Write the block of data to the clipboard */

	ITextClip->WriteClipVector(m_pcSetData, m_iDataSize);

	/* And free the temporary buffer */

	delete [] m_pcSetData;
	m_pcSetData = NULL;

#else /* ! __amigaos4__ */

	ASSERTM((m_poHandle != NULL), "RClipboard::SetDataEnd() => SetDataStart() must be called first");

	/* Unlock the block of memory we have been writing to */

	DEBUGCHECK(GlobalUnlock(m_pcSetData), "RClipboard::SetDataEnd() => Unable to unlock global clipboard data");

	/* And assign ownership of the memory block to the clipboard */

	if (SetClipboardData(CF_TEXT, m_poHandle) != NULL)
	{
		m_poHandle = NULL;
	}
	else
	{
		Utils::Info("RClipboard::SetDataEnd() => Unable to set clipboard data");
	}

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 06-Jul-2010 7:47 am */

const char *RClipboard::GetDataStart()
{
	const char *RetVal;

	/* Assume failure */

	RetVal = NULL;

#ifdef __amigaos4__

	ULONG Size;

	/* Check to see if there is any plain text available on the clipboard and if so, get a ptr to it */

	if (ITextClip->ReadClipVector((STRPTR *) &RetVal, &Size))
	{
		m_pccGetData = m_pccCurrentGetData = (const char *) RetVal;
	}

#else /* ! __amigaos4__ */

	HANDLE Handle;

	/* Check to see if there is any plain text available on the clipboard and if so, get a handle to it */

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if ((Handle = GetClipboardData(CF_TEXT)) != NULL)
		{
			/* Lock the handle into memory and return a ptr to it */

			if ((RetVal = m_pccGetData = m_pccCurrentGetData = (const char *) GlobalLock(Handle)) == NULL)
			{
				Utils::Info("RClipboard::GetDataStart() => Unable to lock keyboard data into memory");
			}
		}
		else
		{
			Utils::Info("RClipboard::GetDataStart() => Unable to get handle to clipboard data");
		}
	}
	else
	{
		Utils::Info("RClipboard::GetDataStart() => No clipboard data available");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:49 am */

void RClipboard::GetDataEnd()
{
	ASSERTM((m_pccGetData != NULL), "RClipboard::GetDataEnd() => SetDataStart() must be called first");

#ifdef __amigaos4__

	ITextClip->DisposeClipVector((STRPTR) m_pccGetData);

#else /* ! __amigaos4__ */

	GlobalUnlock((void *) m_pccGetData);

#endif /* ! __amigaos4__ */

	m_pccGetData = NULL;
}
