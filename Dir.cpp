
#include "StdFuncs.h"

#ifdef __amigaos__

#include <proto/utility.h>
#include "OS4Support.h"

#elif defined(__unix__)

#include <errno.h>
#include <fnmatch.h>
#include <time.h>
#include <sys/stat.h>

#endif /* __unix__ */

#include <string.h>
#include "Dir.h"

#ifndef __amigaos4__

/* Size of the memory blocked used when scanning directories.  Each structure instance is approximately 44 bytes */
/* in size, but we won't get exactly 100 entries of 44 bytes back, as the space is also used for the filename and */
/* comment strings, and it is possible to request less information than what can be stored in the structure */

#define NUM_ENTRIES 100
#define DATA_BUFFER_SIZE (sizeof(struct ExAllData) * NUM_ENTRIES)

#endif /* ! __amigaos4__ */

/** Structure containing information regarding the desired sort order */

struct SSortInfo
{
	enum TDirSortOrder m_eSortOrder;	/**< Sort order originally passed to TEntryArray::Sort() */
};

/**
 * TEntry constructor.
 * Initialises the members of the class to their default values, including setting
 * the iAttributes member to something reasonable for each platform.
 *
 * @date	Saturday 03-Nov-2007 7:27 pm
 */

TEntry::TEntry()
{
	Reset();
}

/**
 * TEntry constructor.
 * Initialises most members of the class to their default values, including setting the
 * iAttributes member to something reasonable for each platform.  This override accepts a
 * TDateTime structure that is assigned to the entry, and the entry's platform specific
 * timestamp related fields are initialised as appropriate.
 *
 * @date	Sunday 24-Jan-2021 8:03 am, Code HQ Bergmannstrasse
 * @param	a_roDateTime	The timestamp to be assigned to the entry
 */

TEntry::TEntry(const TDateTime &a_roDateTime)
{
	Reset();

	iModified = a_roDateTime;

#ifdef __amigaos__

	ULONG AmigaDate;
	struct ClockData ClockData;

	/* Extract the time information from the TDateTime structure into an Amiga specific structure */

	ClockData.year = a_roDateTime.Year();
	ClockData.month = (a_roDateTime.Month() + 1);
	ClockData.mday = a_roDateTime.Day();
	ClockData.hour = a_roDateTime.Hour();
	ClockData.min = a_roDateTime.Minute();
	ClockData.sec = a_roDateTime.Second();
	AmigaDate = Date2Amiga(&ClockData);

	/* And now convert that into the number of seconds since the 1st of January 1978 */

	iPlatformDate.ds_Days = (AmigaDate / SECONDS_PER_DAY);
	AmigaDate = (AmigaDate % SECONDS_PER_DAY);
	iPlatformDate.ds_Minute = (AmigaDate / 60);
	iPlatformDate.ds_Tick = ((AmigaDate % 60) * 50);

#elif defined(__unix__)

	struct tm Tm;

	/* Extract the time information from the TDateTime structure into a UNIX specific structure */

	memset(&Tm, 0, sizeof(Tm));
	Tm.tm_year = (a_roDateTime.Year() - 1900);
	Tm.tm_mon = a_roDateTime.Month();
	Tm.tm_mday = a_roDateTime.Day();
	Tm.tm_hour = a_roDateTime.Hour();
	Tm.tm_min = a_roDateTime.Minute();
	Tm.tm_sec = a_roDateTime.Second();
	Tm.tm_wday = 0;
	Tm.tm_yday = 0;
	Tm.tm_isdst = -1;

	/* And now convert that into the number of seconds since the UNIX Epoch, which is the */
	/* 1st of January 1970 */

	iPlatformDate = mktime(&Tm);

#else /* ! __unix__ */

	SYSTEMTIME SystemTime;

	/* Extract the time information from the TDateTime structure into a Windows specific structure */

	SystemTime.wYear = (WORD) a_roDateTime.Year();
	SystemTime.wMonth = (WORD) (a_roDateTime.Month() + 1);
	/* This member is ignored by SystemTimeToFileTime() but initialise it anyway, for consistency */
	SystemTime.wDayOfWeek = 0;
	SystemTime.wDay = (WORD) a_roDateTime.Day();
	SystemTime.wHour = (WORD) a_roDateTime.Hour();
	SystemTime.wMinute = (WORD) a_roDateTime.Minute();
	SystemTime.wSecond = (WORD) a_roDateTime.Second();
	SystemTime.wMilliseconds = (WORD) a_roDateTime.MilliSecond();

	/* And now convert that into the number of 100 nanosecond intervals since the 1st of January 1601 */

	DEBUGCHECK(SystemTimeToFileTime(&SystemTime, &iPlatformDate), "Unable to convert date to filetime");

#endif /* ! __unix__ */

}

/**
 * Clears a file's archive attribute.
 * Clears a file's archive attribute, thus indicating that the file has been archived and is no
 * longer changed on disc.  The next time the file is edited by a program, the archive attribute
 * will again be set, indicating to backup software that the file needs to be backed up.
 * This function currently only performs an action on Windows.
 *
 * @date	Thursday 09-Jun-2016 06:44 am, Code HQ Ehinger Tor
 */

void TEntry::ClearArchive()
{

#ifdef WIN32

	/* Windows is a little odd with its "normal" file attribute.  A sane implementation would */
	/* simply consider a file with no attributes to be a normal file, but with Windows a file */
	/* can be normal while still having special attribute bits set.  So to clear the archive */
	/* attribute we have to both clear the archive attribute and set the normal attribute */

	iAttributes = (iAttributes & ~FILE_ATTRIBUTE_ARCHIVE);
	iAttributes |= FILE_ATTRIBUTE_NORMAL;

#endif /* WIN32 */

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

#ifdef __amigaos__

	/* Amiga OS does not have the concept of hidden files */

	return(EFalse);

#elif defined(__unix__)

	/* UNIX does not have the concept of hidden files (we won't count files starting */
	/* with . as hidden as this is not a function of the file system */

	return(EFalse);

#else /* ! __unix__ */

	return(iAttributes & FILE_ATTRIBUTE_HIDDEN);

#endif /* ! __unix__ */

}

/* Written: Friday 17-Aug-2012 6:49 am */

TBool TEntry::IsReadable() const
{

#ifdef __amigaos__

	return((iAttributes & EXDF_NO_READ) == 0);

#elif defined(__unix__)

	return(iAttributes & S_IRUSR);

#else /* ! __unix__ */

	return(ETrue);

#endif /* ! __unix__ */

}

/* Written: Friday 17-Aug-2012 6:51 am */

TBool TEntry::IsWriteable() const
{

#ifdef __amigaos__

	return((iAttributes & EXDF_NO_WRITE) == 0);

#elif defined(__unix__)

	return(iAttributes & S_IWUSR);

#else /* ! __unix__ */

	return((iAttributes & FILE_ATTRIBUTE_READONLY) == 0);

#endif /* ! __unix__ */

}

/* Written: Friday 17-Aug-2012 6:52 am */

TBool TEntry::IsExecutable() const
{

#ifdef __amigaos__

	return((iAttributes & EXDF_NO_EXECUTE) == 0);

#elif defined(__unix__)

	return(iAttributes & S_IXUSR);

#else /* ! __unix__ */

	return(ETrue);

#endif /* ! __unix__ */

}

/* Written: Friday 17-Aug-2012 6:53 am */

TBool TEntry::IsDeleteable() const
{

#ifdef __amigaos__

	return((iAttributes & EXDF_NO_DELETE) == 0);

#elif defined(__unix__)

	return(iAttributes & S_IWUSR);

#else /* ! __unix__ */

	return((iAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)) == 0);

#endif /* ! __unix__ */

}

/**
 * Reset the class instance's variables to defaults.
 * Initialises the members of the class to their default values, including setting
 * the iAttributes member to something reasonable for each platform.
 *
 * @date	Monday 25-Jan-2021 6:18 am, Code HQ Bergmannstrasse
 */

void TEntry::Reset()
{
	iName[0] = iLink[0] = '\0';
	iIsDir = iIsLink = EFalse;
	iSize = 0;

#ifdef __amigaos__

	iAttributes = 0;

#elif defined(__unix__)

	iAttributes = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

#else /* WIN32 */

	iAttributes = FILE_ATTRIBUTE_NORMAL;

#endif /* WIN32 */

}

/* Written: Saturday 03-Nov-2007 6:42 pm */

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TInt64 a_iSize, TUint a_uiAttributes, const TDateTime &a_roDateTime)
{
	iIsDir = a_bIsDir;
	iIsLink = a_bIsLink;
	iSize = a_iSize;
	iAttributes = a_uiAttributes;
	iModified = a_roDateTime;

#ifdef WIN32

	/* From a functional perspective, knowing that a file on Windows is compressed by the file system is of no use, */
	/* but it can confuse software that is trying to compare files for equality.  So throw this attribute away, if */
	/* it is present */

	iAttributes &= ~FILE_ATTRIBUTE_COMPRESSED;

	if (iAttributes == 0)
	{
		iAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

#endif /* WIN32 */

}

/**
 * Copy attributes of a TEntry instance into this one.
 * Copies all metadata concerning the TEntry instance, except the name string, into this instance.
 *
 * @date	Monday 11-Aug-2025 6:31 pm, Code HQ Tokyo Tsukuda
 * @param	a_entry			Reference to new TEntry to assign to this TEntry
 */

void TEntry::Set(const TEntry &a_oEntry)
{
	Set(a_oEntry.iIsDir, a_oEntry.iIsLink, a_oEntry.iSize, a_oEntry.iAttributes, a_oEntry.iModified.DateTime());
	iPlatformDate = a_oEntry.iPlatformDate;
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

		iEntries.addTail(Entry);
	}

	return(Entry);
}

/**
 * Compares two directory entries and returns whether one is "less" than the other.
 * This function is used by the TEntryArray::Sort() function as a callback when the list of
 * TEntry structures is being sorted.  The sort function calls it in order to determine in which
 * order to sort the nodes in the list.  The return value will depend on the sort mode being used.
 *
 * @date	Saturday 12-Jul-2014 6:59 am, Code HQ Ehinger Tor
 * @param	a_poFirst		Pointer to the first node to be compared
 * @param	a_poSecond		Pointer to the second node to be compared
 * @param	a_pvUserData	Pointer to a SSortInfo structure that was passed to StdList::Sort()
 * @return	A negative value to indicate that the first node should go first on the list,
 *			otherwise a positive value to indicate that the second node should go first
 */

TInt TEntryArray::CompareEntries(const TEntry *a_poFirst, const TEntry *a_poSecond, void *a_pvUserData)
{
	TInt RetVal;
	struct SSortInfo *SortInfo;

	SortInfo = (struct SSortInfo *) a_pvUserData;

	ASSERTM((SortInfo != NULL), "TEntryArray::CompareEntries() => User data passed in is invalid");

	if (SortInfo->m_eSortOrder == EDirSortNameAscending)
	{
		RetVal = strcmp(a_poFirst->iName, a_poSecond->iName);
	}
	else if (SortInfo->m_eSortOrder == EDirSortSizeAscending)
	{
		RetVal = static_cast<TInt>(a_poFirst->iSize - a_poSecond->iSize);
	}
	else if (SortInfo->m_eSortOrder == EDirSortSizeDescending)
	{
		RetVal = static_cast<TInt>(a_poSecond->iSize - a_poFirst->iSize);
	}
	else if (SortInfo->m_eSortOrder == EDirSortDateAscending)
	{
		RetVal = (a_poFirst->iModified > a_poSecond->iModified);
	}
	else if (SortInfo->m_eSortOrder == EDirSortDateDescending)
	{
		RetVal = (a_poSecond->iModified > a_poFirst->iModified);
	}
	else
	{
		RetVal = strcmp(a_poSecond->iName, a_poFirst->iName);
	}

	return(RetVal);
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

	Entry = iEntries.getHead();

	while (a_iIndex > 0)
	{
		Entry = iEntries.getSucc(Entry);
		--a_iIndex;
	}

	return(*Entry);
}

/* Written: Saturday 11-Jul-2008 10:44 pm */

const TEntry *TEntryArray::getHead() const
{
	return(iEntries.getHead());
}

const TEntry *TEntryArray::getSucc(const TEntry *a_poEntry) const
{
	return(iEntries.getSucc(a_poEntry));
}

/* Written: Saturday 11-Jul-2010 3:36 pm */

void TEntryArray::Purge()
{
	TEntry *Entry;

	/* Iterate through the list of nodes and delete each one */

	while ((Entry = iEntries.remHead()) != NULL)
	{
		delete Entry;
	}
}

/* Written: Saturday 11-Jul-2008 11:42 pm */

void TEntryArray::remove(const TEntry *a_poEntry, TBool a_bDelete)
{
	iEntries.remove((TEntry *) a_poEntry);

	if (a_bDelete)
	{
		delete a_poEntry;
	}
}

/**
 * Sorts the array of file entries.
 * This function will sort the array of file entries in one of a number of ways, as
 * specified by the a_eSortOrder parameter.  The entries are sorted in situ.
 *
 * @date	Saturday 12-Jul-2014 7:23 am, Code HQ Ehinger Tor
 * @param	a_eSortOrder	Order in which to sort, as specified by the TDirSortOrder enum
 */

void TEntryArray::Sort(enum TDirSortOrder a_eSortOrder)
{
	struct SSortInfo SortInfo;

	/* Setup a structure containing information regarding the sort order and call the link list's */
	/* sorting function */

	SortInfo.m_eSortOrder = a_eSortOrder;

	iEntries.Sort(&SortInfo, CompareEntries);
}

/* Written: Saturday 03-Nov-2007 5:24 pm */

RDir::RDir()
{

#ifdef __amigaos__

	iPath = NULL;
	iPattern = NULL;
	iContext = NULL;

#ifndef __amigaos4__

	iLock = 0;
	iCurrent = iExAllData = NULL;

#endif /* ! __amigaos4__ */

#elif defined(__unix__)

	iPathBuffer = iPath = iPattern = NULL;
	iDir = NULL;

#else /* ! __unix__ */

	iHandle = NULL;

#endif /* __unix__ */

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

		if ((Entry = m_entries.Append(a_poFindData->cFileName)) != NULL)
		{
			/* Determine the time that the object was last written to */

			if (FileTimeToSystemTime(&a_poFindData->ftLastWriteTime, &SystemTime))
			{
				/* Convert the Windows time to a generic framework time structure */

				TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay,
					SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);

				/* And populate the TEntry structure with the rest of the information */

				Entry->Set((a_poFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), (a_poFindData->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT),
					(TInt64) a_poFindData->nFileSizeHigh << 32 | a_poFindData->nFileSizeLow, a_poFindData->dwFileAttributes, DateTime);
				Entry->iPlatformDate = a_poFindData->ftLastWriteTime;
			}
			else
			{
				Utils::info("RDir::AppendDirectoryEntry() => Unable to determine time of file or directory");

				RetVal = KErrGeneral;
			}
		}
		else
		{
			Utils::info("RDir::AppendDirectoryEntry() => Unable to convert Windows time to generic framework time");

			RetVal = KErrNoMemory;
		}
	}

	return(RetVal);
}

#endif /* WIN32 */

/**
 * Opens an object for scanning.
 * This function prepares to scan a file or directory.  The a_pccPattern parameter can
 * refer to either a directory name, a single filename, a wildcard pattern or a combination
 * thereof.  Examples are:
 *
 * ""\n
 * "."\n
 * "SomeDir"\n
 * "SomeDir/"\n
 * "*"\n
 * "*.cpp"\n
 * "SomeFile"\n
 * "SomeDir/SomeFile.txt"\n
 * "PROGDIR:"\n
 *
 * @date	Saturday 03-Nov-2007 4:43 pm
 * @param	a_pccPattern	OS specific path and wildcard to scan
 * @return	KErrNone if directory was opened successfully
 * @return	KErrNotFound if the directory or file could not be opened for scanning
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

TInt RDir::open(const char *a_pccPattern)
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

			if ((Entry = m_entries.Append(iSingleEntry.iName)) != NULL)
			{
				RetVal = KErrNone;

				Entry->Set(iSingleEntry.iIsDir, iSingleEntry.iIsLink, iSingleEntry.iSize, iSingleEntry.iAttributes,
					iSingleEntry.iModified.DateTime());
				Entry->iPlatformDate = iSingleEntry.iPlatformDate;
			}
		}
	}

#ifdef __amigaos__

	const char *Pattern;
	TInt Length;
	LONG Result;
	STRPTR FileNameOffset;

	/* Only try to scan a directory if it wasn't a single filename that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Allocate a buffer for the path passed in and save it for l8r use in RDir::read(). */
		/* It is required in order to examine links */

		if ((iPath = new char[strlen(a_pccPattern) + 1]) != NULL)
		{
			strcpy(iPath, a_pccPattern);

			/* We may or may not need to use a pattern, depending on whether there is one */
			/* passed in, so determine this and build a pattern to scan for as appropriate */

			Pattern	= Utils::filePart(a_pccPattern);

			/* According to dos.doc, the buffer used must be at least twice the size of */
			/* the pattern it is scanning + 2 */

			Length = (strlen(Pattern) * 2 + 2);

			if ((iPattern = new char[Length]) != NULL)
			{
				RetVal = KErrNone;

				/* See if a pattern was passed in */

				if ((Result = ParsePatternNoCase(Pattern, iPattern, Length)) == 1)
				{
					/* We are using a pattern so remove it from the base path, which we */
					/* want to point just to the directory */

					FileNameOffset = PathPart(iPath);
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

#ifdef __amigaos4__

				/* Open a context for the directory to be scanned */

				iContext = ObtainDirContextTags(EX_StringNameInput, iPath,
					EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_DONE);

#else /* ! __amigaos4__ */

				if ((iLock = Lock(iPath, ACCESS_READ)) != 0)
				{
					/* Allocate a buffer for the file information to be returned, and a control block to keep track */
					/* of the progress of the scan */

					if ((iExAllData = new struct ExAllData[DATA_BUFFER_SIZE]) != NULL)
					{
						if ((iContext = (struct ExAllControl *) AllocDosObject(DOS_EXALLCONTROL, NULL)) != NULL)
						{
							iContext->eac_LastKey = 0;
						}
					}
				}

				if (!(iContext))
				{
					if (iExAllData)
					{
						delete [] iExAllData;
						iExAllData = NULL;
					}

					if (iLock != 0)
					{
						UnLock(iLock);
						iLock = 0;
					}
				}

#endif /* ! __amigaos4__ */

				if (!(iContext))
				{
					Result = IoErr();

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
			Utils::info("RDir::open() => Out of memory");

			RetVal = KErrNoMemory;
		}
	}

#elif defined(__unix__)

	char *ProgDirName;
	const char *ToOpen;
	TInt Length, FileNameOffset;

	/* Only try to scan a directory if it wasn't a single filename that was passed in */

	if (!(iSingleEntryOk))
	{
		/* If the filename is prefixed with an Amiga OS style "PROGDIR:" then resolve it */

		if ((ProgDirName = Utils::ResolveProgDirName(a_pccPattern)) != NULL)
		{
			/* Allocate a buffer to hold the path part of the directory and save the path and */
			/* wildcard (if any) into it */

			Length = strlen(ProgDirName);

			if ((iPathBuffer = new char[Length + 1]) != NULL)
			{
				strcpy(iPathBuffer, ProgDirName);
				FileNameOffset = (Utils::filePart(iPathBuffer) - iPathBuffer);

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

				ToOpen = iPath;

				if (*ToOpen == '\0')
				{
					ToOpen = ".";
				}

				/* Open the directory for scanning.  We don't do any actual scanning here - that will */
				/* be done in read() */

				if ((iDir = opendir(ToOpen)) != NULL)
				{
					RetVal = KErrNone;
				}
				else
				{
					RetVal = KErrNotFound;
				}
			}

			/* And free the resolved filename, but only if it contained the PROGDIR: prefix */

			if (ProgDirName != a_pccPattern)
			{
				delete [] ProgDirName;
			}
		}
		else
		{
			RetVal = KErrNoMemory;
		}
	}

#else /* ! __unix__ */

	char *Path, *ProgDirName;
	const char *FileName;
	size_t Length;
	WIN32_FIND_DATA FindData;

	/* Only try to scan a directory if it wasn't a single filename that was passed in */

	if (!(iSingleEntryOk))
	{
		/* If the filename is prefixed with an Amiga OS style "PROGDIR:" then resolve it */

		if ((ProgDirName = Utils::ResolveProgDirName(a_pccPattern)) != NULL)
		{
			/* Allocate a buffer large enough to hold the path to be scanned and the */
			/* wildcard pattern used to scan it (wildcard + \0" == 5 bytes) */

			Length = (strlen(ProgDirName) + 5);

			if ((Path = new char[Length]) != NULL)
			{
				/* We may or may not need to append a wildcard, depending on whether there */
				/* is already one in the pattern passed in, so determine this and build a */
				/* wildcard pattern to scan for as appropriate */

				FileName = Utils::filePart(ProgDirName);

				if (!(strstr(FileName, "*")) && (!(strstr(FileName, "?"))))
				{
					strcpy(Path, ProgDirName);
					DEBUGCHECK((Utils::addPart(Path, "*.*", Length) != EFalse), "RDir::open() => Unable to build wildcard to scan");
				}
				else
				{
					strcpy(Path, ProgDirName);
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
				Utils::info("RDir::open() => Out of memory");
			}

			/* And free the resolved filename, but only if it contained the PROGDIR: prefix */

			if (ProgDirName != a_pccPattern)
			{
				delete [] ProgDirName;
			}
		}
	}

#endif /* ! __unix__ */

	/* If anything went wrong, clean up whatever was allocated */

	if (RetVal != KErrNone)
	{
		close();
	}

	return(RetVal);
}

/* Written: Saturday 03-Nov-2007 4:49 pm */

void RDir::close()
{
	RDirObject::close();

#ifdef __amigaos__

	delete [] iPath;
	iPath = NULL;

	delete [] iPattern;
	iPattern = NULL;

#ifdef __amigaos4__

	if (iContext)
	{
		ReleaseDirContext(iContext);
		iContext = NULL;
	}

#else /* ! __amigaos4__ */

	if (iExAllData)
	{
		delete [] iExAllData;
		iExAllData = NULL;
	}

	if (iContext)
	{
		FreeDosObject(DOS_EXALLCONTROL, iContext);
		iContext = NULL;
	}

	if (iLock)
	{
		UnLock(iLock);
		iLock = 0;
	}

#endif /* ! __amigaos4__ */

#elif defined(__unix__)

	delete [] iPathBuffer;
	iPathBuffer = iPath = iPattern = NULL;

	if (iDir)
	{
		closedir(iDir);
		iDir = NULL;
	}

#else /* ! __unix__ */

	if (iHandle)
	{
		FindClose(iHandle);
		iHandle = NULL;
	}

#endif /* ! __unix__ */

	iSingleEntryOk = EFalse;
}

/**
 * Scans a directory for file and directory entries.
 * Scans a directory that has been prepared with RRemoteDir::open() and populates an internal list with all of
 * the entries found.  This list can be then be accessed using getEntries().
 *
 * @date	Saturday 03-Nov-2007 5:38 pm
 * @param	a_eSortOrder	Enumeration specifying the order in which to sort the files.
 *							EDirSortNone is used by default
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

TInt RDir::read(enum TDirSortOrder a_eSortOrder)
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

#ifdef __amigaos4__

	char *LinkName;
	TBool AddFile;
	TInt Length, Size;
	struct ClockData ClockData;
	struct ExamineData *ExamineData;
	struct TEntry *Entry, LinkEntry;

	/* Iterate through the scanned entries and prepare a TEntry instance to contain */
	/* the appropriate information about each entry */

	if (iContext)
	{
		while ((ExamineData = ExamineDir(iContext)) != NULL)
		{
			/* Add the file to the list by default */

			AddFile = ETrue;

			/* If we are using pattern matching then check to see if the file matches the patttern */
			/* and if not, don't add it to the list */

			if (iPattern)
			{
				if (!(MatchPatternNoCase(iPattern, ExamineData->Name)))
				{
					AddFile = EFalse;
				}
			}

			/* Add the file to the list if it matches or no wildcards were used */

			if (AddFile)
			{
				if ((Entry = m_entries.Append(ExamineData->Name)) != NULL)
				{
					/* Convert the new style date structure into something more usable that also contains */
					/* year, month and day information */

					Amiga2Date(DateStampToSeconds(&ExamineData->Date), &ClockData);

					/* Convert it to a Symbian style TDateTime structure */

					TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), (ClockData.mday - 1), ClockData.hour,
						ClockData.min, ClockData.sec, 0);

					/* If the file found is a link then special processing will be required to determine */
					/* its real size, as ExamineDir() will return an ExamineData::FileSize of -1 for links. */
					/* If this fails then we will just print a warning and continue, as there is not much */
					/* that can be done about it */

					if (EXD_IS_LINK(ExamineData))
					{
						/* Set the size to 0 by default */

						Size = 0;

						/* Allocate a temporary buffer large enough to hold the fully qualified link name */

						Length = (strlen(iPath) + 1 + strlen(ExamineData->Name) + 1);

						if ((LinkName = new char[Length]) != NULL)
						{
							/* And copy the link name into the buffer */

							strcpy(LinkName, iPath);
							AddPart(LinkName, ExamineData->Name, Length);

							/* Obtain the size of the file that the link points to */

							RetVal = Utils::GetFileInfo(LinkName, &LinkEntry);
							delete [] LinkName;

							/* If we obtained the information successfully then save the size of the file */
							/* to which the link points.  If the target of the link does not exist then do */
							/* not treat this as an error and leave the size of the file as 0 */

							if (RetVal == KErrNone)
							{
								Size = LinkEntry.iSize;
							}
							else if (RetVal == KErrNotFound)
							{
								RetVal = KErrNone;
							}
							else
							{
								break;
							}
						}
						else
						{
							Utils::info("RDir::read() => Unable to allocate buffer to resolve link size");
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

		/* There are not many errors that can be returned from ExamineDir(), except ones to */
		/* indicate that a disc error occurred. Check for anything besides ERROR_NO_MORE_ENTRIES and */
		/* return an error if approrpriate so that client software can abort its attempt to read the */
		/* directory */

		if (IoErr() != ERROR_NO_MORE_ENTRIES)
		{
			RetVal = KErrGeneral;
		}
	}

	/* If iContext == NULL then either we are being called before open() has been called, or open() */
	/* has been called for a single file. Return KErrNone or an error as appropriate */

	else
	{
		if (!(iSingleEntryOk))
		{
			RetVal = KErrGeneral;
		}
	}

#elif defined(__amigaos__)

	char *LinkName;
	BOOL More;
	LONG IoErr;
	TBool AddFile;
	TInt Length, Size;
	struct ClockData ClockData;
	struct TEntry *Entry, LinkEntry;

	/* Iterate through the scanned entries and prepare a TEntry instance to contain the appropriate information */
	/* about each entry */

	if (iContext)
	{
		do
		{
			/* Scan the directory and fill the passed in buffer with as many ExAllData structures as possible.  The */
			/* only extra field we are interested in is the date */

			More = ExAll(iLock, iExAllData, DATA_BUFFER_SIZE, ED_DATE, iContext);

			/* If ExAll() returns FALSE then there are no more entries to be read.  However, an error may have */
			/* occurred so we have to check IoErr() to determine this.  If IoErr() returns ERROR_NO_MORE_ENTRIES */
			/* then no error has occurred but there might be more entries in the buffer that have been returned */
			/* from this call to ExAll() */

			if (!More)
			{
				/* IoErr() can only be called once, so cache the result */

				IoErr = IoErr();

				if (IoErr != ERROR_NO_MORE_ENTRIES)
				{
					RetVal = KErrGeneral;

					Utils::info("RDir::read() => ExAll() failed, IoErr() = %ld", IoErr);

					break;
				}
			}

			/* No error occurred, but there are also no more entries to be read, so bail out */

			if (iContext->eac_Entries == 0)
			{
				continue;
			}

			/* Scan through the list of returned entries and call the user defined callback for each entry */

			iCurrent = iExAllData;

			while (iCurrent)
			{
				/* Add the file to the list by default */

				AddFile = ETrue;

				/* If we are using pattern matching then check to see if the file matches the patttern and if not, */
				/* don't add it to the list */

				if (iPattern)
				{
					if (!(MatchPatternNoCase(iPattern, (char *) iCurrent->ed_Name)))
					{
						AddFile = EFalse;
					}
				}

				if (AddFile)
				{
					if ((Entry = m_entries.Append((char *) iCurrent->ed_Name)) != NULL)
					{
						/* Convert the date information into something more usable that also contains year, month */
						/* and day information */

						struct DateStamp DateStamp;

						DateStamp.ds_Days = iCurrent->ed_Days;
						DateStamp.ds_Minute = iCurrent->ed_Mins;
						DateStamp.ds_Tick = iCurrent->ed_Ticks;
						Amiga2Date(DateStampToSeconds(&DateStamp), &ClockData);

						/* Convert it to a Symbian style TDateTime structure */

						TDateTime DateTime(ClockData.year, (TMonth) ClockData.month, (ClockData.mday - 1),
							ClockData.hour, ClockData.min, ClockData.sec, 0);

						/* If the file found is a link then special processing will be required to determine its */
						/* real size, as ExAll() will return an ExAllData::ed_Size of -1 for links.  If this fails */
						/* then we will just print a warning and continue, as there is not much that can be done */
						/* about it */

						if (TYPE_IS_LINK(iExAllData->ed_Type))
						{
							/* Set the size to 0 by default */

							Size = 0;

							/* Allocate a temporary buffer large enough to hold the fully qualified link name */

							Length = (strlen(iPath) + 1 + strlen((char *) iCurrent->ed_Name) + 1);

							if ((LinkName = new char[Length]) != NULL)
							{
								/* And copy the link name into the buffer */

								strcpy(LinkName, iPath);
								AddPart(LinkName, (char *) iCurrent->ed_Name, Length);

								/* Obtain the size of the file that the link points to */

								RetVal = Utils::GetFileInfo(LinkName, &LinkEntry);
								delete [] LinkName;

								/* If we obtained the information successfully then save the size of the file */
								/* to which the link points.  If the target of the link does not exist then do */
								/* not treat this as an error and leave the size of the file as 0 */

								if (RetVal == KErrNone)
								{
									Size = LinkEntry.iSize;
								}
								else if (RetVal == KErrNotFound)
								{
									RetVal = KErrNone;
								}
								else
								{
									break;
								}
							}
							else
							{
								Utils::info("RDir::read() => Unable to allocate buffer to resolve link size");
							}
						}
						else
						{
							Size = iCurrent->ed_Size;
						}

						/* And populate the new TEntry instance with information about the file or directory */

						Entry->Set((iCurrent->ed_Type >= 0), TYPE_IS_LINK(iCurrent->ed_Type), Size, iCurrent->ed_Prot, DateTime);
						Entry->iPlatformDate = DateStamp;
					}
					else
					{
						RetVal = KErrNoMemory;

						break;
					}
				}

				iCurrent = iCurrent->ed_Next;
			}
		}
		while (More);
	}

	/* If iContext == NULL then either we are being called before open() has been called, or open() */
	/* has been called for a single file. Return KErrNone or an error as appropriate */

	else
	{
		if (!(iSingleEntryOk))
		{
			RetVal = KErrGeneral;
		}
	}

#elif defined(__unix__)

	char *QualifiedName;
	TBool Append;
	TInt Length;
	struct dirent *DirEnt;
	TEntry *Entry, FileInfo;

	/* Only try to scan a directory if it wasn't a single filename that was passed in */

	if (!(iSingleEntryOk))
	{
		/* Ensure that the directory has been opened for reading */

		if (iDir)
		{
			QualifiedName = NULL;

			/* We must manually clear errno here, as it is not cleared by readdir() when successful */
			/* and may result in bogus errors appearing if it was != 0 when this routine was entered! */

			errno = 0;

			/* Scan through the directory and fill the m_entries array with filenames */

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
					if ((Entry = m_entries.Append(DirEnt->d_name)) != NULL)
					{
						/* UNIX only returns the filename itself when scanning the directory so get all of */
						/* the other details for the directory entry */

						// TODO: CAW - Change the Amiga version to use Utils::GetTempBuffer()
						Length = (strlen(iPath) + 1 + strlen(DirEnt->d_name) + 1);

						if ((QualifiedName = (char *) Utils::GetTempBuffer(QualifiedName, Length)) != NULL)
						{
							strcpy(QualifiedName, iPath);
							Utils::addPart(QualifiedName, DirEnt->d_name, Length);

							/* Pass in EFalse as the a_bResolveLink parameter as we want to get information about the */
							/* link itself, not the file it points to */

							RetVal = Utils::GetFileInfo(QualifiedName, Entry, EFalse);
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

				/* Reset errno before calling readdir() again, in case it was set by another function call while */
				/* scanning the directory */

				errno = 0;
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
			Utils::info("RDir::read() => Unable to scan an unopened directory");

			RetVal = KErrGeneral;
		}
	}

#else /* ! __unix__ */

	WIN32_FIND_DATA FindData;

	/* Only try to scan a directory if it wasn't a single filename that was passed in */

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
			Utils::info("RDir::read() => Unable to scan an unopened directory");

			RetVal = KErrGeneral;
		}
	}

#endif /* ! __unix__ */

	/* Sort the list before returning it, if requested */

	if (a_eSortOrder != EDirSortNone)
	{
		m_entries.Sort(a_eSortOrder);
	}

	return(RetVal);
}
