
#include "StdFuncs.h"
#include "BaUtils.h"
#include "File.h"
#include <string.h>

#ifdef __unix__

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#endif /* __unix__ */

/* Written: Friday 02-Jan-2009 9:03 pm */

RFile::RFile()
{

#ifdef __amigaos__

	m_uiFileMode = m_oHandle = 0;

#elif defined(__unix__)

	m_oHandle = -1;

#else /* ! __unix__ */

	m_oHandle = INVALID_HANDLE_VALUE;

#endif /* ! __unix__ */

}

/**
 * Creates a new file for writing.
 * Creates a new file that can subsequently be used for writing operations.  If a file already
 * exists with the same name then the function will fail.  The a_pccFileName parameter can additionally
 * include a path to the file but the path must already exist.  It will not be created if it does
 * not already exist.
 *
 * @date	Friday 02-Jan-2009 8:54 pm
 * @param	a_pccFileName	Ptr to the name of the file to be created
 * @param	a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian
 *							API and is ignored (but should be EFileWrite for consistency); one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt RFile::Create(const char *a_pccFileName, TUint a_uiFileMode)
{
	TInt RetVal;

	(void) a_uiFileMode;

#ifdef __amigaos__

	TEntry Entry;

	/* Only create the file if it does not already exist.  Amiga OS does not */
	/* have a mode that allows us to do this so we have to manually perform a */
	/* check ourselves */

	if (Utils::GetFileInfo(a_pccFileName, &Entry) == KErrNotFound)
	{
		if ((m_oHandle = IDOS->Open(a_pccFileName, MODE_NEWFILE)) != 0)
		{
			RetVal = KErrNone;

			m_uiFileMode = EFileWrite;
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_pccFileName);
		}
	}
	else
	{
		RetVal = KErrAlreadyExists;
	}

#elif defined(__unix__)

	TInt Flags;

	Flags = (O_CREAT | O_EXCL | O_RDWR);

	/* Create a new file in read/write mode */

	if ((m_oHandle = open(a_pccFileName, Flags, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) != -1)
	{
		/* Now lock the file so that it cannot be re-opened.  The RFile API does not support having */
		/* multiple locks on individual files */

		if (flock(m_oHandle, (LOCK_EX | LOCK_NB)) == 0)
		{
			RetVal = KErrNone;
		}
		else
		{
			Utils::info("RFile::Create() => Unable to lock file for exclusive access");

			RetVal = KErrGeneral;

			close();
		}
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);

		/* For UNIX we need some special handling as trying to open a file for */
		/* writing when it already exists will return EEXIST but we want to treat */
		/* it as an EBUSY, thus returning KErrInUse as on the other platforms.  So try */
		/* to open the file in read only mode, and if it returns KErrInUse then change */
		/* the error.  Otherwise the error is valid so we want to retain it as it is */

		if (RetVal == KErrAlreadyExists)
		{
			if (open(a_pccFileName, EFileRead) == KErrInUse)
			{
				RetVal = KErrInUse;
			}

			/* Close the file as it may have been successfully opened by the above call */

			close();
		}
	}

#else /* ! __unix__ */

	/* Create a new file in read/write mode */

	if ((m_oHandle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);

		/* For Windows we need some special handling as trying to open a file for */
		/* writing when it already exists will return ERROR_FILE_EXISTS but we want */
		/* to treat it as an ERROR_SHARING_VIOLATION, thus returning KErrInUse as on */
		/* the other platforms.  So try to open the file in read only mode, and if */
		/* it returns KErrInUse then change the error.  Otherwise the error is valid */
		/* so we want to retain it as it is */

		if (RetVal == KErrAlreadyExists)
		{
			if (open(a_pccFileName, EFileRead) == KErrInUse)
			{
				RetVal = KErrInUse;
			}

			/* Close the file as it may have been successfully opened by the above call */

			close();
		}
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Creates a new file for writing, deleting any that previously exists.
 * Creates a new file that can subsequently be used for writing operations.  If a file
 * already exists with the same name then the function will replace it.  This function is
 * a convenience wrapper around RFile::Create();  see that function for further details.
 *
 * @date	Monday 19-Apr-2010 6:26 am
 * @param	a_pccFileName	Ptr to the name of the file to be created
 * @param	a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian
 *							API and is ignored (but should be EFileWrite for consistency); one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt RFile::Replace(const char *a_pccFileName, TUint a_uiFileMode)
{
	TInt RetVal;

	/* Try to delete the file specified so that it can be recreated */

	RetVal = BaflUtils::deleteFile(a_pccFileName);

	/* If the file was deleted successfully or if it didn't exist, continue on to create a new file */

	if ((RetVal == KErrNone) || (RetVal == KErrNotFound))
	{
		RetVal = Create(a_pccFileName, a_uiFileMode);
	}

	return(RetVal);
}

/**
 * Opens an existing file for reading and/or writing.
 * Opens an existing file that can subsequently be used for reading or writing operations,
 * or both.  The file can be opened using the file mode flags EFileRead, EFileWrite, or a
 * logical combination of them both to elicit the desired behaviour.  If the file mode
 * EFileWrite is specified then the file will also be writeable.  The a_pccFileName parameter can
 * optionally specify a path to the file and can also be prefixed with an Amiga OS style "PROGDIR:"
 * prefix.
 *
 * @date	Friday 02-Jan-2009 8:57 pm
 * @param	a_pccFileName	Ptr to the name of the file to be opened
 * @param	a_uiFileMode	Mode in which to open the file; one of the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt RFile::open(const char *a_pccFileName, TUint a_uiFileMode)
{
	char *ResolvedFileName;
	TInt RetVal;

	/* If the filename is prefixed with an Amiga OS style "PROGDIR:" then resolve it */

	if ((ResolvedFileName = Utils::ResolveProgDirName(a_pccFileName)) != NULL)
	{

#ifdef __amigaos__

		/* Open the existing file.  We want to open it as having an exclusive lock */
		/* and being read only if EFileWrite is not specified but neither of */
		/* these features are supported by Amiga OS so we will emulate them l8r */

		if ((m_oHandle = IDOS->Open(ResolvedFileName, MODE_OLDFILE)) != 0)
		{
			/* And change the shared lock to an exclusive lock as our API only */
			/* supports opening files exclusively */

			if (IDOS->ChangeMode(CHANGE_FH, m_oHandle, EXCLUSIVE_LOCK) != 0)
			{
				RetVal = KErrNone;

				/* Save the read/write mode for l8r use */

				m_uiFileMode = a_uiFileMode;
			}
			else
			{
				Utils::info("RFile::open() => Unable to lock file for exclusive access");

				RetVal = KErrGeneral;

				close();
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_pccFileName);
		}

#elif defined(__unix__)

		TInt Flags;

		/* Open an existing file in read or read/write mode as requested */

		Flags = (a_uiFileMode & EFileWrite) ? O_RDWR : O_RDONLY;

		if ((m_oHandle = open(ResolvedFileName, Flags, 0)) != -1)
		{
			/* Now lock the file so that it cannot be re-opened.  The RFile API does not support having */
			/* multiple locks on individual files */

			if (flock(m_oHandle, (LOCK_EX | LOCK_NB)) == 0)
			{
				RetVal = KErrNone;
			}
			else
			{
				Utils::info("RFile::open() => Unable to lock file for exclusive access");

				/* UNIX behaves slightly differently to Amiga OS.  Amiga OS will fail to open the file when it is */
				/* locked but UNIX will open it but then the call to lock will fail, so we have to return KErrInUse */
				/* in here rather than when open() fails */

				RetVal = KErrInUse;

				close();
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_pccFileName);
		}

#else /* ! __unix__ */

		DWORD FileMode;

		/* Determine whether to open the file in read or write mode */

		FileMode = GENERIC_READ;

		if (a_uiFileMode & EFileWrite)
		{
			FileMode |= GENERIC_WRITE;
		}

		/* And open the file */

		if ((m_oHandle = CreateFile(ResolvedFileName, FileMode, 0, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
		{
			RetVal = KErrNone;
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_pccFileName);
		}

#endif /* ! __unix__ */

		/* And free the resolved filename, but only if it contained the prefix */

		if (ResolvedFileName != a_pccFileName)
		{
			delete [] ResolvedFileName;
		}
	}
	else
	{
		Utils::info("RFile::open() => Unable to resolve program name for %s", a_pccFileName);

		RetVal = KErrPathNotFound;
	}

	return(RetVal);
}

/**
 * Reads a number of bytes from the file.
 * Reads a number of bytes from the file.  There must be sufficient data in the file to be
 * able to satisfy the read request, or the function will fail.  It is safe to try and write
 * 0 bytes.  In this case 0 will be returned.
 *
 * @pre		The file must be open
 *
 * @date	Friday 02-Jan-2009 10:20 pm
 * @param	a_pucBuffer		Ptr to the buffer to read the data into
 * @param	a_iLength		Number of bytes in the buffer to be read
 * @return	Number of bytes read, if successful, otherwise KErrGeneral
 */

TInt RFile::read(unsigned char *a_pucBuffer, TInt a_iLength) const
{
	TInt RetVal;

	ASSERTM(m_oHandle, "RFile::read() => File is not open");

#ifdef __amigaos__

	RetVal = IDOS->Read(m_oHandle, a_pucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#elif defined(__unix__)

	RetVal = read(m_oHandle, a_pucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD BytesRead;

	/* Only try to read if at least one byte is being read.  Windows acts strangely if you */
	/* try to read 0 bytes (it returns ERROR_ACCESS_DENIED) even though it can handle trying */
	/* to write 0 bytes, so we will handle the 0 length ourselves */

	if (a_iLength > 0)
	{
		if (ReadFile(m_oHandle, a_pucBuffer, a_iLength, &BytesRead, NULL))
		{
			RetVal = BytesRead;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}
	else
	{
		RetVal = 0;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Seeks to a specified position in the file.
 * This is a basic seek function that will seek from the beginning of a file to the position
 * passed in, that position being a number of bytes from the beginning of the file.
 *
 * @pre		The file must be open
 *
 * @date	Saturday 27-May-2017 6:49 am, Tegel Airport, awaiting flight AB 8062 to Gothenburg
 * @param	a_iBytes		The number of bytes from the start of the file to which to seek
 * @return	KErrNone if successful
 * @return	KErrGeneral if the seek could not be performed
 */

TInt RFile::seek(TInt a_iBytes)
{
	TInt RetVal;

	ASSERTM(m_oHandle, "RFile::seek() => File is not open");

	/* Assume failure */

	RetVal = KErrGeneral;

#ifdef __amigaos__

	if (IDOS->ChangeFilePosition(m_oHandle, a_iBytes, OFFSET_BEGINNING))
	{
		RetVal = KErrNone;
	}

#elif defined(__unix__)

	if (lseek(m_oHandle, a_iBytes, SEEK_SET) != -1)
	{
		RetVal = KErrNone;
	}

#else /* ! __unix__ */

	DWORD Position;

	if ((Position = SetFilePointer(m_oHandle, a_iBytes, NULL, FILE_BEGIN)) != INVALID_SET_FILE_POINTER)
	{
		RetVal = KErrNone;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Writes a number of bytes to the file.
 * Writes a number of bytes to the file.  The file must have been opened in a writeable mode,
 * either by using RFile::open(EFileWrite) or with RFile::Replace() or RFile::Create().  In the
 * latter two cases, files are always opened as writeable, regardless of the file mode passed in.
 * It is safe to try and write 0 bytes.  In this case 0 will be returned
 *
 * @pre		The file must be open
 *
 * @date	Friday 02-Jan-2009 10:29 pm
 * @param	a_pcucBuffer	Ptr to the buffer to be written to the file
 * @param	a_iLength		Number of bytes in the buffer to be written
 * @return	Number of bytes written, if successful, otherwise KErrGeneral
 */

TInt RFile::write(const unsigned char *a_pcucBuffer, TInt a_iLength)
{
	TInt RetVal;

	ASSERTM(m_oHandle, "RFile::write() => File is not open");

#ifdef __amigaos__

	/* Only allow writing if the file was opened using the EFileWrite file mode.  Amiga OS */
	/* doesn't support this functionality so we have to emulate it */

	if (m_uiFileMode & EFileWrite)
	{
		/* Now perform the write and ensure all of the bytes were written */

		RetVal = IDOS->Write(m_oHandle, a_pcucBuffer, a_iLength);

		if (RetVal == -1)
		{
			RetVal = KErrGeneral;
		}
	}
	else
	{
		RetVal = KErrGeneral;
	}

#elif defined(__unix__)

	RetVal = write(m_oHandle, a_pcucBuffer, a_iLength);

	if (RetVal != a_iLength)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD BytesWritten;

	/* Perform the write and ensure all of the bytes were written */

	if ((WriteFile(m_oHandle, a_pcucBuffer, a_iLength, &BytesWritten, NULL)) &&
		((TInt) BytesWritten == a_iLength))
	{
		RetVal = a_iLength;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Closes a file when access is no longer required.
 * Closes a file that has been created for reading & writing, or opened for reading.
 *
 * @date	Friday 02-Jan-2009 8:58 pm
 */

void RFile::close()
{

#ifdef __amigaos__

	if (m_oHandle != 0)
	{
		DEBUGCHECK((IDOS->Close(m_oHandle) != 0), "RFile::close() => Unable to close file");
		m_oHandle = 0;
	}

#elif defined(__unix__)

	if (m_oHandle != -1)
	{
		DEBUGCHECK((close(m_oHandle) == 0), "RFile::close() => Unable to close file");
		m_oHandle = -1;
	}

#else /* ! __unix__ */

	if (m_oHandle != INVALID_HANDLE_VALUE)
	{
		DEBUGCHECK((CloseHandle(m_oHandle) != FALSE), "RFile::close() => Unable to close file");
		m_oHandle = INVALID_HANDLE_VALUE;
	}

#endif /* ! __unix__ */

}
