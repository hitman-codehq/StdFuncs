
#include "StdFuncs.h"
#include <string.h>
#include "StdClipboard.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/textclip.h>

#elif defined(__amigaos__)

#include <proto/iffparse.h>

#define  ID_FTXT        MAKE_ID('F','T','X','T')
#define  ID_CHRS        MAKE_ID('C','H','R','S')

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>

#endif /* QT_GUI_LIB */

/* Written: Tuesday 06-Jul-2010 7:42 am */

TInt RClipboard::open(CWindow *a_poWindow)
{
	TInt RetVal;

	ASSERTM((a_poWindow != NULL), "RClipboard::open() => Window passed in is not open");
	ASSERTM((a_poWindow->m_poWindow != NULL), "RClipboard::open() => Native window passed in is not open");

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	(void) a_poWindow;

#if defined(__amigaos__) && !defined(__amigaos4__)

	RetVal = KErrGeneral;

	if ((m_poHandle = AllocIFF()) != NULL)
	{
		if ((m_poHandle->iff_Stream = (ULONG) OpenClipboard(PRIMARY_CLIP)) != 0)
		{
			RetVal = KErrNone;

			InitIFFasClip(m_poHandle);
		}
	}

#else /* ! defined(__amigaos__) && !defined(__amigaos4__) */

	RetVal = KErrNone;

#endif /* ! defined(__amigaos__) && !defined(__amigaos4__) */

#else /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	RetVal = (OpenClipboard(a_poWindow->m_poWindow)) ? KErrNone : KErrGeneral;

	if (RetVal != KErrNone)
	{
		Utils::info("RClipboard::open() => Unable to open clipboard");
	}

#endif /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:44 am */

void RClipboard::close()
{

#if defined(__amigaos__) && !defined(__amigaos4__)

	if (m_poHandle)
	{
		if (m_poHandle->iff_Stream)
		{
			CloseClipboard((ClipboardHandle *) m_poHandle->iff_Stream);
		}

		FreeIFF(m_poHandle);
	}

#endif /* defined(__amigaos__) && !defined(__amigaos4__) */

#if defined(WIN32) && !defined(QT_GUI_LIB)

	DEBUGCHECK(CloseClipboard(), "RClipboard::close() => Unable to close clipboard");

#endif /* if defined(WIN32) && !defined(QT_GUI_LIB) */

}

/* Written: Wednesday 07-Jul-2010 6:37 am */

const char *RClipboard::GetNextLine(TInt *a_piLength, TBool *a_bHasEOL)
{
	const char *NextChar, *RetVal;

	ASSERTM((m_pccGetData != NULL), "RClipboard::GetDataEnd() => GetDataStart() must be called first");

	/* Assume we are going to return the current line and that it has no EOL characters */

	NextChar = RetVal = m_pccCurrentGetData;
	*a_bHasEOL = EFalse;

	/* Find the end of the current line, being either NULL terminated or the EOL characters */

	while ((*NextChar) && (*NextChar != 0x0d) && (*NextChar != 0x0a))
	{
		++NextChar;
	}

	/* Save the length for the calling code */

	*a_piLength = (TInt) (NextChar - m_pccCurrentGetData);

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

int RClipboard::SetDataStart(size_t a_stMaxLength)
{
	int RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	/* Allocate a temporary buffer into which the client can write its data */

	if ((m_pcSetData = new char[a_stMaxLength + 1]) != NULL)
	{
		RetVal = KErrNone;
		m_stDataSize = a_stMaxLength;

		/* Qt needs char * strings to be NULL terminated in order to convert them */
		/* into a QString so we allocate an extra byte and terminate it here */

		m_pcSetData[m_stDataSize] = '\0';
	}

#else /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	/* Empty the clipboard of its previous contents, thus also taking ownership of it */

	if (EmptyClipboard())
	{
		/* Allocate a global moveable memory block into which to copy the data being inserted, */
		/* lock it temporarily into memory and copy the data into it.  We allocate an extra */
		/* byte to NULL terminate the memory block to indicate the end, or Windows will to do */
		/* funny things to the end of the data, like overwriting an LF with a NULL terminator */

		if ((m_poHandle = GlobalAlloc(GMEM_MOVEABLE, (a_stMaxLength + 1))) != NULL)
		{
			if ((m_pcSetData = (char *) GlobalLock(m_poHandle)) != NULL)
			{
				RetVal = KErrNone;
				m_pcSetData[a_stMaxLength] = '\0';
			}
			else
			{
				Utils::info("RClipboard::SetDataStart() => Unable to copy clipboard data");

				GlobalFree(m_poHandle);
				m_poHandle = NULL;
			}
		}
		else
		{
			Utils::info("RClipboard::SetDataStart() => Unable to allocate memory for clipboard");
		}
	}
	else
	{
		Utils::info("RClipboard::SetDataStart() => Unable to claim ownership of clipboard");
	}

#endif /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	return(RetVal);
}

/* Written: Saturday 10-Jul-2010 12:43 pm */

void RClipboard::AppendData(const char *a_pcData, TInt a_iOffset, size_t a_stLength)
{
	ASSERTM((m_pcSetData != NULL), "RClipboard::AppendData() => SetDataStart() must be called first");

	memcpy((m_pcSetData + a_iOffset), a_pcData, a_stLength);
}

/* Written: Saturday 10-Jul-2010 1:34 pm */

void RClipboard::SetDataEnd()
{
	ASSERTM((m_pcSetData != NULL), "RClipboard::SetDataEnd() => SetDataStart() must be called first");

#ifdef __amigaos__

	/* Write the block of data to the clipboard */

#ifdef __amigaos4__

	DEBUGCHECK(WriteClipVector(m_pcSetData, m_stDataSize), "RClipboard::SetDataEnd() => Unable to write clipboard data");

#else /* ! __amigaos4__ */

	ASSERTM((m_poHandle != NULL), "RClipboard::SetDataEnd() => open() must be called first");

	if (OpenIFF(m_poHandle, IFFF_WRITE) == 0)
	{
		if (PushChunk(m_poHandle, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
		{
			if (PushChunk(m_poHandle, 0, ID_CHRS, IFFSIZE_UNKNOWN) == 0)
			{
				DEBUGCHECK((WriteChunkBytes(m_poHandle, m_pcSetData, m_stDataSize) == (LONG) m_stDataSize),
					"RClipboard::SetDataEnd() => Unable to write clipboard data");
			}
		}

		CloseIFF(m_poHandle);
	}

#endif /* ! __amigaos4__ */

	/* And free the temporary buffer */

	delete [] m_pcSetData;
	m_pcSetData = NULL;

#elif defined(QT_GUI_LIB)

	ASSERTM((m_pcSetData[m_stDataSize] == '\0'), "RClipboard::SetDataEnd() => End of clipboard buffer has been overwritten");

	/* Write the block of data to the clipboard */

	QApplication::clipboard()->setText(m_pcSetData);

	/* And free the temporary buffer */

	delete [] m_pcSetData;
	m_pcSetData = NULL;

#else /* ! QT_GUI_LIB */

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
		Utils::info("RClipboard::SetDataEnd() => Unable to set clipboard data");
	}

#endif /* ! QT_GUI_LIB */

}

/* Written: Tuesday 06-Jul-2010 7:47 am */

const char *RClipboard::GetDataStart(TEncoding a_eEncoding)
{
	const char *RetVal;

	/* Assume failure */

	RetVal = NULL;

#ifdef __amigaos4__

	ULONG Size;

	/* Check to see if there is any plain text available on the clipboard and if so, get a ptr to it */

	if (ReadClipVector((STRPTR *) &RetVal, &Size))
	{
		m_pccGetData = m_pccCurrentGetData = RetVal;
	}

#elif defined(__amigaos__)

	char *GetData;
	LONG Result, Size;
	ContextNode *Chunk;

	ASSERTM((m_poHandle != NULL), "RClipboard::GetDataStart() => open() must be called first");

	if (OpenIFF(m_poHandle, IFFF_READ) == 0)
	{
		if (StopChunk(m_poHandle, ID_FTXT, ID_CHRS) == 0)
		{
			if ((Result = ParseIFF(m_poHandle, IFFPARSE_SCAN)) == 0)
			{
				if ((Chunk = CurrentChunk(m_poHandle)) != NULL)
				{
					if ((Chunk->cn_Type == ID_FTXT) && (Chunk->cn_ID == ID_CHRS))
					{
						if ((GetData = AllocVec((Chunk->cn_Size + 1), 0)) != NULL)
						{
							if ((Size = ReadChunkBytes(m_poHandle, GetData, Chunk->cn_Size)) == Chunk->cn_Size)
							{
								GetData[Size] = '\0';
								RetVal = m_pccGetData = m_pccCurrentGetData = GetData;
							}
							else
							{
								FreeVec(GetData);
							}
						}
					}
				}
			}
		}

		if (!RetVal)
		{
			CloseIFF(m_poHandle);
		}
	}

#elif defined(QT_GUI_LIB)

	/* The data returned by clipboard()->text() needs to be persistent for the life of the RClipboard class, */
	/* so copy it into a temporary QByteArray.  Also, the data must be returned in a format appropriate for */
	/* the currently used encoding */

	if (a_eEncoding == EEncoding8859)
	{
		m_oGetData = QApplication::clipboard()->text().toLatin1();
	}
	else
	{
		m_oGetData = QApplication::clipboard()->text().toLocal8Bit();
	}

	/* Now return a ptr to the start of the clipboard data */

	RetVal = m_pccGetData = m_pccCurrentGetData = m_oGetData.constData();

#else /* ! QT_GUI_LIB */

	HANDLE Handle;

	/* Check to see if there is any plain text available on the clipboard and if so, get a handle to it */

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if ((Handle = GetClipboardData(CF_TEXT)) != NULL)
		{
			/* Lock the handle into memory and return a ptr to it */

			if ((RetVal = m_pccGetData = m_pccCurrentGetData = (const char *) GlobalLock(Handle)) == NULL)
			{
				Utils::info("RClipboard::GetDataStart() => Unable to lock keyboard data into memory");
			}
		}
		else
		{
			Utils::info("RClipboard::GetDataStart() => Unable to get handle to clipboard data");
		}
	}
	else
	{
		Utils::info("RClipboard::GetDataStart() => No clipboard data available");
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/* Written: Tuesday 06-Jul-2010 7:49 am */

void RClipboard::GetDataEnd()
{
	ASSERTM((m_pccGetData != NULL), "RClipboard::GetDataEnd() => GetDataStart() must be called first");

#ifdef __amigaos4__

	DisposeClipVector((STRPTR) m_pccGetData);

#elif defined(__amigaos__)

	ASSERTM((m_poHandle != NULL), "RClipboard::GetDataEnd() => close() must be called first");
	CloseIFF(m_poHandle);

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	GlobalUnlock((void *) m_pccGetData);

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	m_pccGetData = NULL;
}
