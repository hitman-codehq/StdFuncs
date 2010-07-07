
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

	DEBUGCHECK(CloseClipboard());

#endif /* __amigaos4__ */

}

/* Written: Tuesday 06-Jul-2010 7:47 am */

const char *RClipboard::LockData()
{
	char *RetVal;
	HANDLE Handle;

	RetVal = NULL;

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	/* Check to see if there is any plain text available on the clipboard and if so, get a handle to it */

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if ((Handle = GetClipboardData(CF_TEXT)) != NULL)
		{
			/* Lock the handle into memory and return a ptr to it */

			if ((RetVal = m_pcData = (char *) GlobalLock(Handle)) == NULL)
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
	ASSERTM((m_pcData != NULL), "RClipboard::UnlockData() => UnlockData() must not be called without LockData() being called");

#ifdef __amigaos4__

	GlobalUnlock(m_pcData);
	m_pcData = NULL;

#endif /* __amigaos4__ */

}
