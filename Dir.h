
#ifndef DIR_H
#define DIR_H

/** @file */

#include "StdList.h"
#include "Time.h"

#ifdef __linux__

#include <dirent.h>

#endif /* __linux__ */

/** File sorting possibilities for files returned by RDir::Read() */

enum TDirSortOrder
{
	EDirSortNone,			/**< No sorting */
	EDirSortAscending,		/**< Alphabetical sorting in ascending order */
	EDirSortDescending		/**< Alphabetical sorting in descending order */
};

/* An instance of this class represents a directory or filename and its associated */
/* attributes, such as name etc.  It is filled in by the RDir class */

class TEntry
{
public:

	StdListNode<TEntry>	m_oStdListNode;		/* Standard list node */
	char				iName[MAX_PATH];	/* Name of the file */
	TBool				iIsDir;				/* ETrue if entry is a directory */
	TBool				iIsLink;			/* ETrue if entry is a link */
	TUint				iSize;				/* File size in bytes */
	TUint				iAttributes;		/* Protection attributes, in Amiga/UNIX/Windows format */
	TTime				iModified;			/* Time and date of the file */

#ifdef __amigaos4__

	struct DateStamp	iPlatformDate;		/* Date and time in Amiga specific format */

#elif defined(__linux__)

	time_t				iPlatformDate;		/* Date and time in UNIX specific format */

#else /* ! __linux__ */

	FILETIME			iPlatformDate;		/* Date and time in Windows specific format */

#endif /* ! __linux__ */

public:

	TEntry();

	TBool IsDir() const;

	TBool IsLink() const;

	TBool IsHidden() const;

	TBool IsReadable() const;

	TBool IsWriteable() const;

	TBool IsExecutable() const;

	TBool IsDeleteable() const;

	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_oDateTime);
};

/* An instance of this class represents a number of TEntry classes and is filled */
/* in by the RDir class */

class TEntryArray
{
private:

	StdList<TEntry>		iEntries;	/* List of entries */

public:

	/* Note that having a destructor in T classes is non standard, but this is required */
	/* to free the iEntries array, and this class is really private to RDir anyway */

	~TEntryArray();

	TEntry *Append(const char *a_pccName);

	static TInt CompareEntries(const TEntry *a_poFirst, const TEntry *a_poSecond, void *a_pvUserData);

	TInt Count() const;

	const TEntry &operator[](TInt a_iIndex) const;

	const TEntry *GetHead() const;

	const TEntry *GetSucc(const TEntry *a_poEntry) const;

	void Purge();

	void Remove(const TEntry *a_poEntry);

	void Sort(enum TDirSortOrder a_eSortOrder);
};

/* A class for scanning directories for directory and file entries */

class RDir
{
private:

#ifdef __amigaos4__

	char			*iPath;			/* Path to directory being scanned, minus wildcard */
	char			*iPattern;		/* Pattern to be used for MatchPatternNoCase() */
	APTR			iContext;		/* Context used for scanning directory */

#elif defined(__linux__)

	char			*iPathBuffer;	/* Ptr to memory used by iPath and iPattern buffers */
	char			*iPath;			/* Path to directory being scanned, minus wildcard */
	char			*iPattern;		/* Pattern to be used for pattern matching in RDir::Read() */
	DIR				*iDir;

#else /* ! __linux__ */

	HANDLE			iHandle;		/* Handle used for scanning directory */

#endif /* ! __linux__ */

	TEntry			iSingleEntry;	/* If a single entry is being examined, Open() will populate this */
	TBool			iSingleEntryOk;	/* ETrue if the contents of iSingleEntry are valid, else EFalse */
	TEntryArray		iEntries;		/* Array of TEntry classes containing directory and file information */

private:

#ifdef WIN32

	TInt AppendDirectoryEntry(WIN32_FIND_DATA *a_poFindData);

#endif /* WIN32 */

public:

	RDir();

	TInt Open(const char *a_pccPattern);

	void Close();

	// TODO: CAW - Is it possible to make this more Symbian like?
	TInt Read(TEntryArray *&a_rpoEntries, enum TDirSortOrder a_eSortOrder = EDirSortNone);
};

#endif /* ! DIR_H */
