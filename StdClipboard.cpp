
#include "StdFuncs.h"
#include <string.h>
#include "StdClipboard.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/textclip.h>

#elif defined(__amigaos__)

#include <proto/iffparse.h>

#define  ID_FTXT MAKE_ID('F','T','X','T')
#define  ID_CHRS MAKE_ID('C','H','R','S')

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>

#endif /* QT_GUI_LIB */

/**
 * Open the clipboard for use.
 * Prepares the clipboard for use by opening the underlying OS-specific clipboard implementation.
 *
 * @date	Tuesday 06-Jul-2010 7:42 am
 * @param	a_window		Pointer to the window in which to use the clipboard
 * @return	KErrNone if the clipboard was opened successfully, else KErrGeneral
 */

TInt RClipboard::open(CWindow *a_window)
{
	TInt RetVal;

	ASSERTM((a_window != NULL), "RClipboard::open() => Window passed in is not open");
	ASSERTM((a_window->m_poWindow != NULL), "RClipboard::open() => Native window passed in is not open");

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	(void) a_window;

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

	RetVal = (OpenClipboard(a_window->m_poWindow)) ? KErrNone : KErrGeneral;

	if (RetVal != KErrNone)
	{
		Utils::info("RClipboard::open() => Unable to open clipboard");
	}

#endif /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	return(RetVal);
}

/**
 * Close the clipboard after use.
 * Closes the underlying OS-specific clipboard implementation.
 *
 * @date	Tuesday 06-Jul-2010 7:44 am
 */

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

/**
 * Get the next line of available text.
 * Returns a pointer to the next line of text in the clipboard. The text will be returned in the format that was
 * requested when GetDataStart() was called. The text returned is only valid until the next call to this method,
 * or until close() is called.
 *
 * @pre		RClipboard::GetDataStart() has been called
 *
 * @date	Wednesday 07-Jul-2010 6:37 am
 * @param	a_size			Pointer to variable into which to place the number of bytes returned
 * @param	a_hasEOL		Pointer to variable into which to place ETrue if the text returned contains EOL markers
 * @return	A pointer to the text
 */

const char *RClipboard::GetNextLine(TInt *a_size, TBool *a_hasEOL)
{
	const char *NextChar, *RetVal;

	ASSERTM((m_pccGetData != NULL), "RClipboard::GetNextLine() => GetDataStart() must be called first");
	ASSERTM((a_size != NULL), "RClipboard::GetNextLine() => Pointer to size must be passed in");
	ASSERTM((a_hasEOL != NULL), "RClipboard::GetNextLine() => Pointer to EOL flag must be passed in");

	/* Assume we are going to return the current line and that it has no EOL characters */

	NextChar = RetVal = m_pccCurrentGetData;
	*a_hasEOL = EFalse;

	/* Find the end of the current line, being either NULL terminated or the EOL characters */

	while ((*NextChar) && (*NextChar != 0x0d) && (*NextChar != 0x0a))
	{
		++NextChar;
	}

	/* Save the size for the calling code */

	*a_size = (TInt) (NextChar - m_pccCurrentGetData);

	/* Check for CR and LF characters and if present, note their presence and skip them */

	if (*NextChar == 0x0d)
	{
		*a_hasEOL = ETrue;
		++NextChar;
	}

	if (*NextChar == 0x0a)
	{
		*a_hasEOL = ETrue;
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

/**
 * Start saving data to the clipboard.
 * This method should be called to begin the process of copying data into the clipboard. It will allocate a
 * buffer of the given size into which data can later on be copied.
 *
 * @date	Thursday 08-Jul-2010 7:06 am
 * @param	a_maxSize		The size of the buffer, in bytes
 * @return	KErrNone if successful, else KErrNoMemory
 */

int RClipboard::SetDataStart(size_t a_maxSize)
{
	int RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	/* Allocate a temporary buffer into which the client can write its data */

	if ((m_pcSetData = new char[a_maxSize + 1]) != NULL)
	{
		RetVal = KErrNone;
		m_stDataSize = a_maxSize;

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

		if ((m_poHandle = GlobalAlloc(GMEM_MOVEABLE, (a_maxSize + 1))) != NULL)
		{
			if ((m_pcSetData = (char *) GlobalLock(m_poHandle)) != NULL)
			{
				RetVal = KErrNone;
				m_pcSetData[a_maxSize] = '\0';
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

/**
 * Append a block of data.
 * Copies a block of data into the buffer that was previously allocated by SetDataStart(), at the offset given.
 *
 * @pre		SetDataStart() has been called
 *
 * @date	Saturday 10-Jul-2010 12:43 pm
 * @param	a_data			Pointer to the block of data to be copied
 * @param	a_offset		Offset in the allocated block to which to copy
 * @param	a_size			Number of bytes to be copied
 */

void RClipboard::AppendData(const char *a_data, TInt a_offset, size_t a_size)
{
	ASSERTM((m_pcSetData != NULL), "RClipboard::AppendData() => SetDataStart() must be called first");

	memcpy((m_pcSetData + a_offset), a_data, a_size);
}

/**
 * Finish saving data to the clipboard.
 * This method should be called when all data has been copied into the clipboard. It will copy the buffered data into
 * the underlying OS-specific clipboard implementation and will free the buffer that held that data. After this method
 * has returned, it is no longer possible to append any more data unless SetDataStart() is called again.
 *
 * @pre		SetDataStart() has been called
 *
 * @date	Saturday 10-Jul-2010 1:34 pm
 */

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

/**
 * Prepare the clipboard for reading.
 * Calls the underlying OS-specific clipboard implementation to obtain whatever data is available, allocates a buffer
 * for that data and copies the data into it. Client code can then call GetNextLine() to extract the data, one line at
 * a time.
 *
 * Because this uses the OS-specific clipboard implementation, it is not necessary to first place any data into the
 * clipboard (although this is a valid use case). Other applications may have placed data into the clipboard, in which
 * case this data will be returned from this method.
 *
 * @date	Tuesday 06-Jul-2010 7:47 am
 * @param	a_encoding		The text encoding in which to return the data
 * @return	true if the data was obtained successfully, else false
 */

bool RClipboard::GetDataStart(TEncoding a_encoding)
{
	bool RetVal;

	/* Assume failure */

	RetVal = false;

#ifdef __amigaos4__

	(void) a_encoding;

	const char *Data;
	ULONG Size;

	/* Check to see if there is any plain text available on the clipboard and if so, get a ptr to it */

	if (ReadClipVector((STRPTR *) &Data, &Size))
	{
		RetVal = true;
		m_pccGetData = m_pccCurrentGetData = Data;
	}

#elif defined(__amigaos__)

	(void) a_encoding;

	char *Data;
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
						/* The OS3 clipboard routines do not NULL terminate the received data, so allocate an */
						/* extra byte to do this ourselves */

						if ((Data = AllocVec((Chunk->cn_Size + 1), 0)) != NULL)
						{
							if ((Size = ReadChunkBytes(m_poHandle, Data, Chunk->cn_Size)) == Chunk->cn_Size)
							{
								RetVal = true;
								Data[Size] = '\0';
								m_pccGetData = m_pccCurrentGetData = Data;
							}
							else
							{
								FreeVec(Data);
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

	if (a_encoding == EEncoding8859)
	{
		m_oGetData = QApplication::clipboard()->text().toLatin1();
	}
	else
	{
		m_oGetData = QApplication::clipboard()->text().toUtf8();
	}

	/* Now return a ptr to the start of the clipboard data */

	RetVal = true;
	m_pccGetData = m_pccCurrentGetData = m_oGetData.constData();

#else /* ! QT_GUI_LIB */

	(void) a_encoding;

	HANDLE Handle;

	/* Check to see if there is any plain text available on the clipboard and if so, get a handle to it */

	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		if ((Handle = GetClipboardData(CF_TEXT)) != NULL)
		{
			/* Lock the handle into memory and return a ptr to it */

			if ((m_pccGetData = m_pccCurrentGetData = (const char *) GlobalLock(Handle)) != NULL)
			{
				RetVal = true;
			}
			else
			{
				Utils::info("RClipboard::GetDataStart() => Unable to lock clipboard data into memory");
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

/**
 * End the clipboard reading process.
 * Closes the underlying OS-specific clipboard implementation and frees any memory associated with the clipboard
 * data that was read. After calling this method, it is no longer possible to access the clipboard data until
 * GetDataStart() is called again.
 *
 * @pre		GetDataStart() has been called
 *
 * @date	Tuesday 06-Jul-2010 7:49 am
 */

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
