
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/utility.h>

#elif defined(__linux__)

#include <errno.h>
#include <fnmatch.h>
#include <sys/stat.h>

#endif /* __linux__ */

#include <string.h>
#include "Dir.h"

// TODO: CAW - Add Utils::Info() calls all though here and RFile (check others) but only in debug mode?  Consistency!

/* Written: Saturday 03-Nov-2007 7:27 pm */

TEntry::TEntry()
{
	iName[0] = '\0';
	iIsDir = iIsLink = EFalse;
	iSize = 0;
	iAttributes = 0;
}

/* Written: Saturday 03-Nov-2007 8:07 pm */

TBool TEntry::IsDir() const
{
	return(iIsDir);
}

/* Written: Saturday 03-Nov-2007 8:15 pm */

TBool TEntry::IsLink() const
{
	return(iIsLink);
}

/* Written: Monday 28-Dec-2009 12:15 pm */

TBool TEntry::IsHidden() const
{

#ifdef __amigaos4__

	/* Amiga OS does not have the concept of hidden files */

	return(EFalse);

#elif defined(__linux__)

	/* UNIX does not have the concept of hidden files (we won't count files starting */
	/* with . as hidden as this is not a function of the filesystem */

	return(EFalse);

#else /* ! __linux__ */

	return(iAttributes & FILE_ATTRIBUTE_HIDDEN);

#endif /* ! __linux__ */

}

/* Written: Friday 17-Aug-2012 6:49 am */

TBool TEntry::IsReadable() const
{

#ifdef __amigaos4__

	return((iAttributes & FIBF_READ) == 0);

#elif defined(__linux__)

	return(iAttributes & S_IRUSR);

#else /* ! __linux__ */

	return(ETrue);

#endif /* ! __linux__ */

}

/* Written: Friday 17-Aug-2012 6:51 am */

TBool TEntry::IsWriteable() const
{

#ifdef __amigaos4__

	return((iAttributes & FIBF_WRITE) == 0);

#elif defined(__linux__)

	return(iAttributes & S_IWUSR);

#else /* ! __linux__ */

	return((iAttributes & FILE_ATTRIBUTE_READONLY) == 0);

#endif /* ! __linux__ */

}

/* Written: Friday 17-Aug-2012 6:52 am */

TBool TEntry::IsExecutable() const
{

#ifdef __amigaos4__

	return((iAttributes & FIBF_EXECUTE) == 0);

#elif defined(__linux__)

	return(iAttributes & S_IXUSR);

#else /* ! __linux__ */

	return(ETrue);

#endif /* ! __linux__ */

}

/* Written: Friday 17-Aug-2012 6:53 am */

TBool TEntry::IsDeleteable() const
{

#ifdef __amigaos4__

	return((iAttributes & FIBF_DELETE) == 0);

#elif defined(__linux__)

	return(iAttributes & S_IWUSR);

#else /* ! __linux__ */

	return((iAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)) == 0);

#endif /* ! __linux__ */

}

/* Written: Saturday 03-Nov-2007 6:42 pm */

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime)
{
	iIsDir = a_bIsDir;
	iIsLink = a_bIsLink;
	iSize = a_uiSize;
	iAttributes = a_uiAttributes;
	iModified = a_roDateTime;
}

/* Written: Saturday 03-Nov-2007 5:58 pm */

TEntryArray::~TEntryArray()
{
	Purge();
}

/* Written: Friday 19-Jun-2009 7:11 am */

TEntry *TEntryArray::Append(const char *a_pccName)
{
	TEntry *Entry;

	/* Allocate a new TEntry node */

	if ((Entry = new TEntry) != NULL)
	{
		/* Copy the name of the file */

		strcpy(Entry->iName, a_pccName);

		/* And append the node to the list */

		iEntries.AddTail(Entry);
	}

	return(Entry);
}

/* Written: Saturday 03-Nov-2007 6:18 pm */

TInt TEntryArray::Count() const
{
	return(iEntries.Count());
}

/* Written: Saturday 03-Nov-2007 6:20 pm */

const TEntry &TEntryArray::operator[](TInt a_iIndex) const
{
	// TODO: CAW - Will be slow + what if this is NULL? Check for count and assert
	TEntry *Entry;

	Entry = iEntries.GetHead();

	while (a_iIndex > 0)
	{
		Entry = iEntries.GetSucc(Entry);
		--a_iIndex;
	}

	return(*Entry);
}

/* Written: Saturday 11-Jul-2008 10:44 pm */

const TEntry *TEntryArray::GetHead() const
{
	return(iEntries.GetHead());
}

const TEntry *TEntryArray::GetSucc(const TEntry *a_poEntry) const
{
	return(iEntries.GetSucc(a_poEntry));
}

/* Written: Saturday 11-Jul-2010 3:36 pm */

void TEntryArray::Purge()
{
	TEntry *Entry;

	/* Iterate through the list of nodes and delete each one */

	while ((Entry = iEntries.RemHead()) != NULL)
	{
		delete Entry;
	}
}

/* Written: Saturday 11-Jul-2008 11:42 pm */

void TEntryArray::Remove(const TEntry *a_poEntry)
{
	iEntries.Remove((TEntry *) a_poEntry);
}

/* Written: Saturday 03-Nov-2007 5:24 pm */

RDir::RDir()
{

#ifdef __amigaos4__

	iPath = NULL;
	iPattern = NULL;
	iContext = NULL;

#elif defined(__linux__)

	iPathBuffer = iPath = iPattern = NULL;
	iDir = NULL;

#endif /* __linux__ */

	iSingleEntryOk = EFalse;
}

#ifdef WIN32

/* Written: Friday 12-Oct-2012 5:39 am, Maxhotel Lindau */
/* @param	a_poFindData	Windows specific information about a scanned file */
/* @return	KErrNone if the entry was appended successfully */
/*			KErrNoMemory if not enough memory was available */
/*			KErrGeneral if some other unspecified error occurred */
/* This Windows specific function will take a structure containing information about */
/* a scanned file and will convert it to the internal TEntry format used by the */
/* framework.  It will then append it to the list of entries representing the directory */
/* that has been scanned */

TInt RDir::AppendDirectoryEntry(WIN32_FIND_DATA *a_poFindData)
{
	TInt RetVal;
	TEntry *Entry;
	SYSTEMTIME SystemTime;

	/* Assume success */

	RetVal = KErrNone;

	/* Only add the entry if it is not one of the pseudo directory entries */

	if ((strcmp(a_poFindData->cFileName, ".")) && (strcmp(a_poFindData->cFileName, "..")))
	{
		/* Allocate a TEntry structure and simultaneously append it to the list */

		if ((Entry = iEntries.Append(a_poFindData->cFileName)) != NULL)
		{
			/* Determine the time that the object was last written to */

			if (FileTimeToSystemTime(&a_poFindData->ftLastWriteTime, &SystemTime))
			{
				/* Convert the Windows time to a generic framework time structure */

				TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay,
					SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

				/* And populate the TEntry structure with the rest of the information */

				// TODO: CAW - Link stuff, here and in Utils.cpp.  Can this be a reusable function?
				Entry->Set((a_poFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0, a_poFindData->nFileSizeLow, a_poFindData->dwFileAttributes, DateTime);
				Entry->iPlatformDate = a_poFindData->ftLastWriteTime;
			}
			else
			{
				Utils::Info("RDir::AppendDirectoryEntry() => Unable to determine time of file or directory");

				RetVal = KErrGeneral;
			}
		}
		else
		{
			Utils::Info("RDir::AppendDirectoryEntry() => Unable to convert Windows time to generic framework time");

			RetVal = KErrNoMemory;
		}
	}

	return(RetVal);
}

#endif /* WIN32 */

/* Written: Saturday 03-Nov-2007 4:43 pm */
/* @param	a_pccPattern	OS specific path and wildcard to scan */
/* @return	KErrNone if directory was opened successfully */
/*			KErrNotFound if the directory or file could not be opened for scanning */
/*			KErrNoMemory if not enough memory was available */
/*			KErrGeneral if some other unspecified error occurred */
/* This function prepares to scan a file or directory.  The a_pccPattern parameter can */
/* refer to either a directory name, a single filename, a wildcard pattern or a combination */
/* thereof.  Examples are: */
/* */
/* "" */
/* "." */
/* "SomeDir" */
/* "SomeDir/" */
/* "*" */
/* "*.cpp" */
/* "SomeFile" */
/* "SomeDir/SomeFile.txt" */

TInt RDir::Open(const char *a_pccPattern)
{
	TInt RetVal;
	TEntry *Entry;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Get information about the file or directory being passed in. If it is a file or a link */
	/* then save the information for l8r and don't continue */

	if (Utils::GetFileInfo(a_pccPattern, &iSingleEntry) == KErrNone)
	{
		if (!(iSingleEntry.IsDir()))
		{
			iSingleEntryOk = ETrue;

			/* Append the entry to the array of files and directories being listed */

			if ((Entry = iEntries.Append(iSingleEntry.iName)) != NULL)
			{
				RetVal = KErrNone;

				Entry->Set(iSingleEntry.iIsDir, iSingleEntry.iIsLink, iSingleEntry.iSize, iSingleEntry.iAttributes,
					iSingleEntry.iModified.DateTime());
				Entry->iPlatformDate = iSingleEntry.iPlatformDate;
			}
		}
	}

#ifdef __amigaos4__

	const char *Pattern;
	TInt Length;
	LONG Result;
	STRPTR FileNameOffset;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Allocate a buffer for the path passed in and save it for l8r use in RDir::Read(). */
		/* It is required in order to examine links */

		if ((iPath = new char[strlen(a_pccPattern) + 1]) != NULL)
		{
			strcpy(iPath, a_pccPattern);

			/* We may or may not need to use a pattern, depending on whether there is one */
			/* passed in, so determine this and build a pattern to scan for as appropriate */

			Pattern	= Utils::FilePart(a_pccPattern);

			/* According to dos.doc, the buffer used must be at least twice the size of */
			/* the pattern it is scanning + 2 */

			Length = (strlen(Pattern) * 2 + 2);

			if ((iPattern = new char[Length]) != NULL)
			{
				RetVal = KErrNone;

				/* See if a pattern was passed in */

				if ((Result = IDOS->ParsePatternNoCase(Pattern, iPattern, Length)) == 1)
				{
					/* We are using a pattern so remove it from the base path, which we */
					/* want to point just to the directory */

					FileNameOffset = IDOS->PathPart(iPath);
					iPath[FileNameOffset - iPath] = '\0';
				}

				/* No pattern is in use so indicate this */

				else if (Result == 0)
				{
					delete [] iPattern;
					iPattern = NULL;
				}
				else
				{
					RetVal = KErrGeneral;
				}
			}

			if (RetVal == KErrNone)
			{
				/* Open a context for the directory to be scanned */

				iContext = IDOS->ObtainDirContextTags(EX_StringNameInput, iPath,
					EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_DONE);

				if (!(iContext))
				{
					Result = IDOS->IoErr();

					if (Result == ERROR_OBJECT_NOT_FOUND)
					{
						RetVal = KErrNotFound;
					}
					else
					{
						RetVal = KErrGeneral;
					}
				}
			}
		}
		else
		{
			Utils::Info("RDir::Open() => Out of memory");

			RetVal = KErrNoMemory;
		}
	}

#elif defined(__linux__)

	TInt Length, FileNameOffset;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Allocate a buffer to hold the path part of the directory and save the path and */
		/* wildcard (if any) into it */

		Length = strlen(a_pccPattern);

		if ((iPathBuffer = new char[Length + 1]) != NULL)
		{
			strcpy(iPathBuffer, a_pccPattern);
			FileNameOffset = (Utils::FilePart(iPathBuffer) - iPathBuffer);

			/* If there is a wildcard present then extract it */

			if ((strstr(iPathBuffer, "*")) || (strstr(iPathBuffer, "?")))
			{
				/* If FileNameOffset is > 0 then there is a path component so extract both it */
				/* and the pattern */

				if (FileNameOffset > 0)
				{
					iPathBuffer[FileNameOffset - 1] = '\0';
					iPath = iPathBuffer;
					iPattern = &iPathBuffer[FileNameOffset];
				}

				/* Otherwise there is only a pattern */

				else
				{
					iPath = &iPathBuffer[Length];
					iPattern = iPathBuffer;
				}
			}

			/* There is no wildcard so extract only the path and set the pattern to empty */

			else
			{
				iPath = iPathBuffer;
				iPattern = &iPathBuffer[Length];
			}

			/* UNIX will not scan a directory represented by an empty string so if this has */
			/* been passed in then convert it to a "." for compatibility with the RDir API */

			a_pccPattern = iPath;

			if (*a_pccPattern == '\0')
			{
				a_pccPattern = ".";
			}

			/* Open the directory for scanning.  We don't do any actual scanning here - that will */
			/* be done in Read() */

			if ((iDir = opendir(a_pccPattern)) != NULL)
			{
				RetVal = KErrNone;
			}
			else
			{
				RetVal = KErrNotFound;
			}
		}
		else
		{
			RetVal = KErrNoMemory;
		}
	}

#else /* ! __linux__ */

	char *Path;
	const char *FileName;
	TInt Length;
	WIN32_FIND_DATA FindData;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Allocate a buffer large enough to hold the path to be scanned and the */
		/* wildcard pattern used to scan it (wildcard + \0" == 5 bytes) */

		Length = (strlen(a_pccPattern) + 5);

		if ((Path = new char[Length]) != NULL)
		{
			/* We may or may not need to append a wildcard, depending on whether there */
			/* is already one in the pattern passed in, so determine this and build a */
			/* wildcard pattern to scan for as appropriate */

			FileName = Utils::FilePart(a_pccPattern);

			if (!(strstr(FileName, "*")) && !(strstr(FileName, "?")))
			{
				strcpy(Path, a_pccPattern);
				DEBUGCHECK((Utils::AddPart(Path, "*.*", Length) != EFalse), "RDir::Open() => Unable to build wildcard to scan");
			}
			else
			{
				strcpy(Path, a_pccPattern);
			}

			/* Scan the directory using the wildcard and find the first entry */

			if ((iHandle = FindFirstFile(Path, &FindData)) != INVALID_HANDLE_VALUE)
			{
				RetVal = AppendDirectoryEntry(&FindData);
			}
			else
			{
				if (GetLastError() == ERROR_FILE_NOT_FOUND)
				{
					RetVal = KErrNone;
				}
				else
				{
					RetVal = KErrNotFound;
				}
			}

			delete [] Path;
		}
		else
		{
			Utils::Info("RDir::Open() => Out of memory");

			RetVal = KErrNoMemory;
		}
	}

#endif /* ! __linux__ */

	/* If anything went wrong, clean up whatever was allocated */

	if (RetVal != KErrNone)
	{
		Close();
	}

	return(RetVal);
}

/* Written: Saturday 03-Nov-2007 4:49 pm */

void RDir::Close()
{
	/* Free the contents of the TEntry array in case it the RDir class is reused */

	iEntries.Purge();

#ifdef __amigaos4__

	delete [] iPath;
	iPath = NULL;

	delete [] iPattern;
	iPattern = NULL;

	if (iContext)
	{
		IDOS->ReleaseDirContext(iContext);
		iContext = NULL;
	}

#elif defined(__linux__)

	delete [] iPathBuffer;
	iPathBuffer = iPath = iPattern = NULL;

	if (iDir)
	{
		closedir(iDir);
		iDir = NULL;
	}

#else /* ! __linux__ */

	if (iHandle)
	{
		FindClose(iHandle);
		iHandle = NULL;
	}

#endif /* ! __linux__ */

	iSingleEntryOk = EFalse;
}

/* Written: Saturday 03-Nov-2007 5:38 pm */
/* @param	a_roEntries	Reference to a ptr into which to place a ptr to the */
/*						array of entries read by this function */
/* @return	KErrNone if successful */
/*			KErrNoMemory if not enough memory was available */
/*			KErrGeneral if some other unspecified error occurred */
/* Scans a directory that has been prepared with RDir::Open() and populates a list */
/* with all of the entries found.  This list is then returned to the calling client */
/* code */

TInt RDir::Read(TEntryArray *&a_roEntries)
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;
	a_roEntries = &iEntries;

#ifdef __amigaos4__

	char *LinkName;
	TBool AddFile, Error;
	TInt Length, Size;
	struct ClockData ClockData;
	struct ExamineData *ExamineData;
	struct TEntry *Entry, LinkEntry;

	/* Iterate through the scanned entries and prepare a TEntry instance to contain */
	/* the appropriate information about each entry */

	Error = EFalse;

	if (iContext)
	{
		while ((ExamineData = IDOS->ExamineDir(iContext)) != NULL)
		{
			/* Add the file to the list by default */

			AddFile = ETrue;

			/* If we are using pattern matching then check to see if the file matches the patttern */
			/* and if not, don't add it to the list */

			if (iPattern)
			{
				if (!(IDOS->MatchPatternNoCase(iPattern, ExamineData->Name)))
				{
					AddFile = EFalse;
				}
			}

			/* Add the file to the list if it matches or no wildcards were used */

			if (AddFile)
			{
				if ((Entry = iEntries.Append(ExamineData->Name)) != NULL)
				{
					/* Convert the new style date structure into something more usable that also contains */
					/* year, month and day information */

					IUtility->Amiga2Date(IDOS->DateStampToSeconds(&ExamineData->Date), &ClockData);

					/* Convert it to a Symbian style TDateTime structure */

					TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), (ClockData.mday - 1), ClockData.hour,
						ClockData.min, ClockData.sec, 0);

					/* If the file found is a link then special processing will be required to determine */
					/* its real size, as ExamineDir() will return an ExamineData::FileSize of -1 for links. */
					/* If this fails then we will just print a warning and continue, as there is not much */
					/* that can be done about it */

					if (EXD_IS_LINK(ExamineData))
					{
						/* Set the size to 0 by default so as not to mess up any size calculations with -1 */

						Size = 0;

						/* Allocate a temporary buffer large enough to hold the fully qualified link name */

						Length = (strlen(iPath) + 1 + strlen(ExamineData->Name) + 1);

						if ((LinkName = new char[Length]) != NULL)
						{
							/* And copy the link name into the buffer */

							strcpy(LinkName, iPath);
							IDOS->AddPart(LinkName, ExamineData->Name, Length);

							/* Obtain the size of the file that the link points to */

							if ((RetVal = Utils::GetFileInfo(LinkName, &LinkEntry)) == KErrNone)
							{
								Size = LinkEntry.iSize;
							}
							else
							{
								break;
							}

							delete [] LinkName;
						}
						else
						{
							Utils::Info("RDir::Read() => Unable to allocate buffer to resolve link size\n");
						}
					}
					else
					{
						Size = ExamineData->FileSize;
					}

					/* And populate the new TEntry instance with information about the file or directory */

					Entry->Set(EXD_IS_DIRECTORY(ExamineData), EXD_IS_LINK(ExamineData), Size, ExamineData->Protection,
						DateTime);
					Entry->iPlatformDate = ExamineData->Date;
				}
				else
				{
					RetVal = KErrNoMemory;

					break;
				}
			}
		}

		/* There are not many errors that can be returned from IDOS->ExamineDir(), except ones to */
		/* indicate that a disk error occurred. Check for anything besides ERROR_NO_MORE_ENTRIES and */
		/* return an error if approrpriate so that client software can abort its attempt to read the */
		/* directory */

		if (IDOS->IoErr() != ERROR_NO_MORE_ENTRIES)
		{
			RetVal = KErrGeneral;
		}
	}

	/* If iContext == NULL then either we are being called before Open() has been called, or Open() */
	/* has been called for a single file. Return KErrNone or an error as appropriate */

	else
	{
		if (!(iSingleEntryOk))
		{
			RetVal = KErrGeneral;
		}
	}

#elif defined(__linux__)

	char *QualifiedName;
	TBool Append;
	TInt Length;
	struct dirent *DirEnt;
	struct TEntry *Entry, FileInfo;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Ensure that the directory has been opened for reading */

		if (iDir)
		{
			QualifiedName = NULL;

			/* We must manually clear errno here, as it is not cleared by readdir() when successful */
			/* and may result in bogus errors appearing if it was != 0 when this routine was entered! */

			errno = 0;

			/* Scan through the directory and fill the iEntries array with filenames */

			while ((RetVal == KErrNone) && ((DirEnt = readdir(iDir)) != NULL))
			{
				/* Append the entry to the entry list, but only if it doesn't represent the current or */
				/* parent directory */

				Append = EFalse;

				if ((strcmp(DirEnt->d_name, ".")) && (strcmp(DirEnt->d_name, "..")))
				{
					Append = ETrue;

					/* If there is a wildcard then see if it matches and if not, don't append the entry */

					if (iPattern[0] != '\0')
					{
						if (fnmatch(iPattern, DirEnt->d_name, 0) != 0)
						{
							Append = EFalse;
						}
					}
				}

				/* Append the entry if it is was judged ok to do so */

				if (Append)
				{
					if ((Entry = iEntries.Append(DirEnt->d_name)) != NULL)
					{
						/* UNIX only returns the filename itself when scanning the directory so get all of */
						/* the other details for the directory entry */

						// TODO: CAW - Change the Amiga version to use Utils::GetTempBuffer()
						Length = (strlen(iPath) + 1 + strlen(DirEnt->d_name) + 1);

						if ((QualifiedName = (char *) Utils::GetTempBuffer(QualifiedName, Length)) != NULL)
						{
							strcpy(QualifiedName, iPath);
							Utils::AddPart(QualifiedName, DirEnt->d_name, Length);

							RetVal = Utils::GetFileInfo(QualifiedName, Entry);
						}
						else
						{
							RetVal = KErrNoMemory;
						}
					}
					else
					{
						RetVal = KErrNoMemory;

						break;
					}
				}
			}

			/* Free the temporary buffer.  This is done outside the loop so that the originally allocated */
			/* buffer can be reused if it is large enough */

			Utils::FreeTempBuffer(QualifiedName);

			/* If DirEnt is NULL then we reached the end of the readdir() scan.  Check to see if this */
			/* happened because we had scanned all files or if it is because there was an error.  We */
			/* also have to take into account that RetVal may have been set during scanning due to */
			/* another error occurring */

			if ((RetVal == KErrNone) && (!(DirEnt)) && (errno != 0))
			{
				RetVal = KErrNotFound;
			}
		}
		else
		{
			Utils::Info("RDir::Read() => Unable to scan an unopened directory");

			RetVal = KErrGeneral;
		}
	}

#else /* ! __linux__ */

	WIN32_FIND_DATA FindData;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Ensure that the directory has been opened for reading */

		if (iHandle)
		{
			while (FindNextFile(iHandle, &FindData))
			{
				if ((RetVal = AppendDirectoryEntry(&FindData)) != KErrNone)
				{
					break;
				}
			}

			/* Determine whether we ran out of files to scan or an error occurred when scanning */

			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				RetVal = KErrGeneral;
			}
		}
		else
		{
			Utils::Info("RDir::Read() => Unable to scan an unopened directory");

			RetVal = KErrGeneral;
		}
	}

#endif /* ! __linux__ */

	return(RetVal);
}
