
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/utility.h>

#elif defined(__linux__)

#include <errno.h>

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

	return(EFalse); // TODO: CAW + use a similar check to below above?

#elif defined(__linux__)

	return(EFalse); // TODO: CAW + use a similar check to below above?

#else /* ! __linux__ */

	return(iAttributes & FILE_ATTRIBUTE_HIDDEN);

#endif /* ! __linux__ */

}

/* Written: Saturday 03-Nov-2007 6:42 pm */

#ifdef __amigaos4__

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime,
	const struct DateStamp &a_roPlatformDate)

#elif defined(__linux__)

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime,
	const time_t &a_roPlatformDate)

#else /* ! __linux__ */

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime,
	const FILETIME &a_roPlatformDate)

#endif /* ! __linux__ */

{
	iIsDir = a_bIsDir;
	iIsLink = a_bIsLink;
	iSize = a_uiSize;
	iAttributes = a_uiAttributes;
	iModified = a_roDateTime;
	iPlatformDate = a_roPlatformDate;
}

/* Written: Saturday 03-Nov-2007 5:57 pm */

TEntryArray::TEntryArray()
{
	iCount = 0;
};

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

		++iCount;
		iEntries.AddTail(Entry);
	}

	return(Entry);
}

/* Written: Saturday 03-Nov-2007 6:18 pm */

TInt TEntryArray::Count() const
{
	return(iCount);
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

	/* And indicate that the array is now empty */

	iCount = 0;
}

/* Written: Saturday 11-Jul-2008 11:42 pm */

void TEntryArray::Remove(const TEntry *a_poEntry)
{
	iEntries.Remove((TEntry *) a_poEntry);
	--iCount;
}

/* Written: Saturday 03-Nov-2007 5:24 pm */

RDir::RDir()
{

#ifdef __amigaos4__

	iPath = NULL;
	iPattern = NULL;
	iContext = NULL;

#elif defined(__linux__)

	iPath = NULL;
	iDir = NULL;

#endif /* __linux__ */

	iSingleEntryOk = EFalse;
}

/* Written: Saturday 03-Nov-2007 4:43 pm */
/* @param	a_pccPattern	OS specific path and wildcard to scan */
/* @return	KErrNone if directory was opened successfully */
/*			KErrNotFound if the directory or file could not be opened for scanning */
/*			KErrNoMemory if not enough memory to open the directory or file */
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

// TODO: CAW - For all versions, this should only open the directory, not scan it
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

				// TODO: CAW - Why isn't this using Set()? Is Set() required?
				Entry->iIsDir = iSingleEntry.iIsDir;
				Entry->iIsLink = iSingleEntry.iIsLink;
				Entry->iSize = iSingleEntry.iSize;
				Entry->iAttributes = iSingleEntry.iAttributes;
				Entry->iModified = iSingleEntry.iModified;
				Entry->iPlatformDate = iSingleEntry.iPlatformDate;
			}
		}
	}

#ifdef __amigaos4__

	const char *Pattern;
	TInt Length;
	LONG Result;
	STRPTR FileNameOffset;

	/* Allocate a buffer for the path passed in and save it for l8r use */

	// TODO: CAW - Make like Windows version, here and in Read().  When done make all checks for iSingleEntryOk
	//             into one for all platforms
	if ((iPath = new char[strlen(a_pccPattern) + 1]) != NULL)
	{
		strcpy(iPath, a_pccPattern);

		if (!(iSingleEntryOk))
		{
			Length = (strlen(a_pccPattern) * 2 + 4 + 2); // TODO: CAW

			if ((iPattern = new char[Length]) != NULL)
			{
				RetVal = KErrNone;

				Pattern = IDOS->FilePart(a_pccPattern);

				if (*Pattern == '\0')
				{
					Pattern = "#?"; // TODO: CAW - Needed?
				}

				if ((Result = IDOS->ParsePatternNoCase(Pattern, iPattern, Length)) == 1)
				{
					FileNameOffset = IDOS->PathPart(iPath);
					iPath[FileNameOffset - iPath] = '\0';
				}
				else if (Result == 0)
				{
					delete [] iPattern;
					iPattern = NULL;
				}
				else
				{
					RetVal = KErrGeneral; // TODO: CAW + all errors need to be checked here and documentation updated
				}
			}

			if (RetVal == KErrNone)
			{
				/* Open a context for the directory to be scanned */

				iContext = IDOS->ObtainDirContextTags(EX_StringNameInput, iPath,
					EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_END);

				if (iContext)
				{
					RetVal = KErrNone; // TODO: CAW
				}
				else
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
	}

#elif defined(__linux__)

	TInt FileNameOffset;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Allocate a buffer to hold the path part of the directory and save the path into */
		/* it by chopping off the filepart of the path passed in (which will be a wildcard). */

		if ((iPath = new char[strlen(a_pccPattern) + 1]) != NULL)
		{
			strcpy(iPath, a_pccPattern);
			FileNameOffset = (Utils::FilePart(iPath) - iPath);

			/* If FileNameOffset == 0 then there is no path component */

			if (FileNameOffset > 0)
			{
				iPath[FileNameOffset] = '\0';
			}

			/* UNIX will not scan a directory represented by an empty string so if this has */
			/* been passed in then convert it to a "." for compatibility with the RDir API */

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
				//RetVal = KErrPathNotFound; // TODO: CAW - This is breaking the Linux version and really needs to be sorted!
				RetVal = KErrNotFound;
			}
		}
		else
		{
			RetVal = KErrNoMemory;
		}
	}

#else /* ! __linux__ */

	char Path[10240]; // TODO: CAW
	WIN32_FIND_DATA FindData;

	if (!(iSingleEntryOk))
	{
		// TODO: CAW - Should only scan the end of a_pccPattern using FilePart()
		if (!(strstr(a_pccPattern, "*")) && (!(strstr(a_pccPattern, "?"))))
		{
			if (strlen(a_pccPattern) > 0)
			{
				strcpy(Path, a_pccPattern);
				strcat(Path, "/*.*");
			}
			else
			{
				strcpy(Path, "*.*"); // TODO: CAW - Needed
			}
		}
		else
		{
			strcpy(Path, a_pccPattern); // TODO: CAW - Needed
		}

		if ((iHandle = FindFirstFile(Path, &FindData)) != INVALID_HANDLE_VALUE)
		{
			RetVal = KErrNone;

			if ((strcmp(FindData.cFileName, ".")) && (strcmp(FindData.cFileName, "..")))
			{
				// TODO: CAW - What frees that + all in one function please + name + finish date time stuff
				TEntry *Lah = iEntries.Append(FindData.cFileName);

				if (Lah)
				{
					SYSTEMTIME SystemTime;

					if (FileTimeToSystemTime(&FindData.ftLastWriteTime, &SystemTime))
					{
						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

						// TODO: CAW - Link stuff, here and in Utils.cpp
						Lah->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0/*EXD_IS_LINK(Entry)*/, FindData.nFileSizeLow, FindData.dwFileAttributes, DateTime,
							FindData.ftLastWriteTime); // TODO: CAW - Extend T_File/T_Dir for this
					}
					else
					{
						RetVal = KErrGeneral; // TODO: CAW + this stuff is done here twice and in Utils::GetFileInfo()
					}
				}
				else
				{
					RetVal = KErrNoMemory;
				}
			}
		}
		else
		{
			// TODO: CAW - Ensure these match Symbian return values. Not finding a file is ok as it means
			//			that the directory was opened successfully but that no files were in it. Write a
			//			test case to handle this situation
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				RetVal = KErrNone;
			}
			else
			{
				RetVal = KErrNotFound;
			}
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

#ifndef WIN32

	delete [] iPath;
	iPath = NULL;

#endif /* ! WIN32 */

#ifdef __amigaos4__

	delete [] iPattern;
	iPattern = NULL;

	if (iContext)
	{
		IDOS->ReleaseDirContext(iContext);
		iContext = NULL;
	}

#elif defined(__linux__)

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

	if (iContext) // TODO: CAW - Test without this + perform full failure testing
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
					AddFile = EFalse; // TODO: CAW - What about error checking using IDOS->IoErr() below?
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

					/* Convert it so a Symbian style TDateTime structure */

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

							if (Utils::GetFileInfo(LinkName, &LinkEntry) == KErrNone) // TODO: CAW
							{
								Size = LinkEntry.iSize;
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
						DateTime, ExamineData->Date);
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
			RetVal = KErrGeneral; // TODO: CAW - What should this be?
		}
	}

#elif defined(__linux__)

	char *QualifiedName;
	TInt BaseLength, Length;
	struct dirent *DirEnt;
	struct TEntry *Entry, FileInfo;

	/* Only try to scan a directory if it wasn't a single file name that was passed in */

	if (!(iSingleEntryOk))
	{
		QualifiedName = NULL;
		BaseLength = (strlen(iPath) + 1 + 1);

		/* We must manually clear errno here, as it is not cleared by readdir() when successful */
		/* and may result in bogus errors appearing if it was != 0 when this routine was entered! */

		errno = 0;

		/* Scan through the directory and fill the iEntries array with filenames */

		while ((RetVal == KErrNone) && ((DirEnt = readdir(iDir)) != NULL))
		{
			/* Append the entry to the entry list, but only if it doesn't represent the current or */
			/* parent directory */

			if ((strcmp(DirEnt->d_name, ".")) && (strcmp(DirEnt->d_name, "..")))
			{
				if ((Entry = iEntries.Append(DirEnt->d_name)) != NULL)
				{
					/* UNIX only returns the filename itself when scanning the directory so get all of */
					/* the other details for the directory entry */

					// TODO: CAW - Use a function for this and change the Amiga version to match it.  Amiga
					//             version uses IDOS->AddPart()?
					Length = (BaseLength + strlen(DirEnt->d_name));

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

#else /* ! __linux__ */

	WIN32_FIND_DATA FindData;

	if (!(iSingleEntryOk))
	{
		// TODO: CAW - How to tell if this has failed or run out of files?
		while (FindNextFile(iHandle, &FindData))
		{
			if ((strcmp(FindData.cFileName, ".")) && (strcmp(FindData.cFileName, "..")))
			{
				// TODO: CAW - What frees that + all in one function please + name + finish date time stuff
				TEntry *Lah = iEntries.Append(FindData.cFileName);

				if (Lah)
				{
					SYSTEMTIME SystemTime;

					// TODO: CAW - This should take DST into account. What about on Amiga OS?
					if (FileTimeToSystemTime(&FindData.ftLastWriteTime, &SystemTime))
					{
						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

						Lah->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0/*EXD_IS_LINK(Entry)*/, FindData.nFileSizeLow, FindData.dwFileAttributes, DateTime,
							FindData.ftLastWriteTime);
					}
					else
					{
						RetVal = KErrGeneral; // TODO: CAW
					}
				}
				else
				{
					RetVal = KErrNoMemory;

					break;
				}
			}
		}
	}

#endif /* ! __linux__ */

	return(RetVal);
}
