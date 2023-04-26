
#include "StdFuncs.h"
#include "File.h"
#include "FileUtils.h"
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

	m_fileMode = m_handle = 0;

#elif defined(__unix__)

	m_handle = -1;

#else /* ! __unix__ */

	m_handle = INVALID_HANDLE_VALUE;

#endif /* ! __unix__ */

}

/**
 * Creates a new file for writing.
 * Creates a new file that can subsequently be used for writing operations.  If a file already
 * exists with the same name then the function will fail.  The a_fileName parameter can additionally
 * include a path to the file but the path must already exist.  It will not be created if it does
 * not already exist.
 *
 * @date	Friday 02-Jan-2009 8:54 pm
 * @param	a_fileName		Ptr to the name of the file to be created
 * @param	a_fileMode		Mode in which to create the file; one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RFile::create(const char *a_fileName, TUint a_fileMode)
{
	TInt RetVal;

#ifdef __amigaos__

	TEntry Entry;

	/* Only create the file if it does not already exist.  Amiga OS does not */
	/* have a mode that allows us to do this so we have to manually perform a */
	/* check ourselves */

	if (Utils::GetFileInfo(a_fileName, &Entry) == KErrNotFound)
	{
		if ((m_handle = Open(a_fileName, MODE_READWRITE)) != 0)
		{
			RetVal = KErrNone;

			m_fileMode = EFileWrite;

			/* And change the shared lock to an exclusive lock, if exclusive mode has been requested */

			if (a_fileMode & EFileExclusive)
			{
				if (ChangeMode(CHANGE_FH, m_handle, EXCLUSIVE_LOCK) == 0)
				{
					Utils::info("RFile::create() => Unable to lock file for exclusive access");

					RetVal = KErrInUse;

					close();
				}
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_fileName);
		}
	}
	else
	{
		RetVal = KErrAlreadyExists;
	}

#elif defined(__unix__)

	int Flags;

	Flags = (O_CREAT | O_EXCL | O_RDWR);

	/* Create a new file in read/write mode */

	if ((m_handle = ::open(a_fileName, Flags, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) != -1)
	{
		RetVal = KErrNone;

		/* Now lock the file so that it cannot be re-opened, if exclusive mode has been requested */

		if (a_fileMode & EFileExclusive)
		{
			if (flock(m_handle, (LOCK_EX | LOCK_NB)) != 0)
			{
				Utils::info("RFile::create() => Unable to lock file for exclusive access");

				RetVal = KErrGeneral;

				close();
			}
		}
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_fileName);
	}

#else /* ! __unix__ */

	DWORD ShareMode = 0;

	if (!(a_fileMode & EFileExclusive))
	{
		ShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE);
	}

	/* Create a new file in read/write mode */

	if ((m_handle = CreateFile(a_fileName, GENERIC_WRITE, ShareMode, NULL, CREATE_NEW, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_fileName);
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Creates a new file for writing, deleting any that previously exists.
 * Creates a new file that can subsequently be used for writing operations.  If a file
 * already exists with the same name then the function will replace it.  This function is
 * a convenience wrapper around RFile::create();  see that function for further details.
 *
 * @date	Monday 19-Apr-2010 6:26 am
 * @param	a_fileName		Ptr to the name of the file to be created
 * @param	a_fileMode		Mode in which to create the file; one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RFile::replace(const char *a_fileName, TUint a_fileMode)
{
	int RetVal;

	/* Try to delete the file specified so that it can be recreated */

	RFileUtils FileUtils;
	RetVal = FileUtils.deleteFile(a_fileName);

	/* If the file was deleted successfully or if it didn't exist, continue on to create a new file */

	if ((RetVal == KErrNone) || (RetVal == KErrNotFound))
	{
		RetVal = create(a_fileName, a_fileMode);
	}

	return(RetVal);
}

/**
 * Opens an existing file for reading and/or writing.
 * Opens an existing file that can subsequently be used for reading or writing operations, or both.
 * The file can be opened using the file mode flags EFileRead, EFileWrite, or a logical combination
 * of them both to elicit the desired behaviour.  The a_fileName parameter can optionally include a
 * path to the file and can also be prefixed with an Amiga OS style "PROGDIR:" prefix.
 *
 * @date	Friday 02-Jan-2009 8:57 pm
 * @param	a_fileName		Ptr to the name of the file to be opened
 * @param	a_fileMode		Mode in which to open the file; one of the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrInUse if the file is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RFile::open(const char *a_fileName, TUint a_fileMode)
{
	char *ResolvedFileName;
	int RetVal;

	/* If the filename is prefixed with an Amiga OS style "PROGDIR:" then resolve it */

	if ((ResolvedFileName = Utils::ResolveProgDirName(a_fileName)) != NULL)
	{

#ifdef __amigaos__

		/* Open the existing file.  We want to open it as having an exclusive lock */
		/* and being read only if EFileWrite is not specified but neither of */
		/* these features are supported by Amiga OS so we will emulate them l8r */

		if ((m_handle = Open(ResolvedFileName, MODE_OLDFILE)) != 0)
		{
			RetVal = KErrNone;

			/* Save the read/write mode for l8r use */

			m_fileMode = a_fileMode;

			/* And change the shared lock to an exclusive lock, if exclusive mode has been requested */

			if (a_fileMode & EFileExclusive)
			{
				if (ChangeMode(CHANGE_FH, m_handle, EXCLUSIVE_LOCK) == 0)
				{
					Utils::info("RFile::open() => Unable to lock file for exclusive access");

					RetVal = KErrInUse;

					close();
				}
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_fileName);
		}

#elif defined(__unix__)

		int Flags;

		/* Open an existing file in read or read/write mode as requested */

		Flags = (a_fileMode & EFileWrite) ? O_RDWR : O_RDONLY;

		if ((m_handle = ::open(ResolvedFileName, Flags, 0)) != -1)
		{
			RetVal = KErrNone;

			/* And change the shared lock to an exclusive lock, if exclusive mode has been requested */

			if (a_fileMode & EFileExclusive)
			{
				if (flock(m_handle, (LOCK_EX | LOCK_NB)) != 0)
				{
					Utils::info("RFile::open() => Unable to lock file for exclusive access");

					/* UNIX behaves slightly differently to Amiga OS.  Amiga OS will fail to open the file when it is */
					/* locked but UNIX will open it but then the call to lock will fail, so we have to return KErrInUse */
					/* in here rather than when open() fails */

					RetVal = KErrInUse;

					close();
				}
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_fileName);
		}

#else /* ! __unix__ */

		DWORD FileMode, ShareMode;

		/* Determine whether to open the file in read or write mode */

		FileMode = GENERIC_READ;
		ShareMode = 0;

		if (a_fileMode & EFileWrite)
		{
			FileMode |= GENERIC_WRITE;
		}

		if (!(a_fileMode & EFileExclusive))
		{
			ShareMode = (FILE_SHARE_READ | FILE_SHARE_WRITE);
		}

		/* And open the file */

		if ((m_handle = CreateFile(ResolvedFileName, FileMode, ShareMode, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
		{
			RetVal = KErrNone;
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_fileName);
		}

#endif /* ! __unix__ */

		/* And free the resolved filename, but only if it contained the prefix */

		if (ResolvedFileName != a_fileName)
		{
			delete [] ResolvedFileName;
		}
	}
	else
	{
		Utils::info("RFile::open() => Unable to resolve program name for %s", a_fileName);

		RetVal = KErrPathNotFound;
	}

	return(RetVal);
}

/**
 * Reads a number of bytes from the file.
 * Reads a number of bytes from the file.  There must be sufficient data in the file to be
 * able to satisfy the read request, or the function will fail.  It is safe to try and read
 * 0 bytes.  In this case 0 will be returned.
 *
 * @pre		The file must be open
 *
 * @date	Friday 02-Jan-2009 10:20 pm
 * @param	a_buffer	Ptr to the buffer to read the data into
 * @param	a_length	Number of bytes in the buffer to be read
 * @return	Number of bytes read, if successful
 * @return	KErrGeneral if the read could not be performed
 */

int RFile::read(unsigned char *a_buffer, int a_length)
{
	int RetVal;

	ASSERTM(m_handle, "RFile::read() => File is not open");

#ifdef __amigaos__

	RetVal = Read(m_handle, a_buffer, a_length);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#elif defined(__unix__)

	RetVal = ::read(m_handle, a_buffer, a_length);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD BytesRead;

	/* Only try to read if at least one byte is being read.  Windows acts strangely if you */
	/* try to read 0 bytes (it returns ERROR_ACCESS_DENIED) even though it can handle trying */
	/* to write 0 bytes, so we will handle the 0 length ourselves */

	if (a_length > 0)
	{
		if (ReadFile(m_handle, a_buffer, a_length, &BytesRead, NULL))
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
 * @param	a_bytes			The number of bytes from the start of the file to which to seek
 * @return	KErrNone if successful
 * @return	KErrGeneral if the seek could not be performed
 */

int RFile::seek(int a_bytes)
{
	TInt RetVal;

	ASSERTM(m_handle, "RFile::seek() => File is not open");

	/* Assume failure */

	RetVal = KErrGeneral;

#ifdef __amigaos__

	if (Seek(m_handle, a_bytes, OFFSET_BEGINNING))
	{
		RetVal = KErrNone;
	}

#elif defined(__unix__)

	if (lseek(m_handle, a_bytes, SEEK_SET) != -1)
	{
		RetVal = KErrNone;
	}

#else /* ! __unix__ */

	DWORD Position;

	if ((Position = SetFilePointer(m_handle, a_bytes, NULL, FILE_BEGIN)) != INVALID_SET_FILE_POINTER)
	{
		RetVal = KErrNone;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Writes a number of bytes to the file.
 * Writes a number of bytes to the file.  The file must have been opened in a writeable mode,
 * either by using RFile::open(EFileWrite), RFile::replace() or RFile::create().  In the
 * latter two cases, files are always opened as writeable, regardless of the file mode passed in.
 * It is safe to try and write 0 bytes.  In this case 0 will be returned
 *
 * @pre		The file must be open
 *
 * @date	Friday 02-Jan-2009 10:29 pm
 * @param	a_buffer	Ptr to the buffer to be written to the file
 * @param	a_length		Number of bytes in the buffer to be written
 * @return	Number of bytes written, if successful
 * @return	KErrGeneral if the write could not be performed
 */

int RFile::write(const unsigned char *a_buffer, int a_length)
{
	TInt RetVal;

	ASSERTM(m_handle, "RFile::write() => File is not open");

#ifdef __amigaos__

	/* Only allow writing if the file was opened using the EFileWrite file mode.  Amiga OS */
	/* doesn't support this functionality so we have to emulate it */

	if (m_fileMode & EFileWrite)
	{
		/* Now perform the write and ensure all of the bytes were written */

		RetVal = Write(m_handle, a_buffer, a_length);

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

	RetVal = ::write(m_handle, a_buffer, a_length);

	if (RetVal != a_length)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD BytesWritten;

	/* Perform the write and ensure all of the bytes were written */

	if ((WriteFile(m_handle, a_buffer, a_length, &BytesWritten, NULL)) &&
		((TInt) BytesWritten == a_length))
	{
		RetVal = a_length;
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
 * @return	KErrNone is always returned, as this method cannot fail
 */

int RFile::close()
{

#ifdef __amigaos__

	if (m_handle != 0)
	{
		DEBUGCHECK((Close(m_handle) != 0), "RFile::close() => Unable to close file");
		m_handle = 0;
	}

#elif defined(__unix__)

	if (m_handle != -1)
	{
		DEBUGCHECK((::close(m_handle) == 0), "RFile::close() => Unable to close file");
		m_handle = -1;
	}

#else /* ! __unix__ */

	if (m_handle != INVALID_HANDLE_VALUE)
	{
		DEBUGCHECK((CloseHandle(m_handle) != FALSE), "RFile::close() => Unable to close file");
		m_handle = INVALID_HANDLE_VALUE;
	}

#endif /* ! __unix__ */

	return KErrNone;
}
