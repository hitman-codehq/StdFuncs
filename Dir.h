
#ifndef DIR_H
#define DIR_H

#include "StdList.h"
#include "Time.h"

#ifdef __linux__

#include <dirent.h>

#endif /* __linux__ */

/* An instance of this class represents a directory or file name and its associated */
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

	// TODO: CAW - I don't like this, nor the Set() function below
	struct DateStamp	iPlatformDate;		/* Date and time in Amiga specific format */

#elif defined(__linux__)

	time_t				iPlatformDate;		/* Date and time in UNIX specific format */

#else /* ! __linux__ */

	FILETIME			iPlatformDate;		/* Date and time in Windows specific format */

#endif /* ! __linux__ */

	TEntry();

	TBool IsDir() const;

	TBool IsLink() const;

	TBool IsHidden() const;

#ifdef __amigaos4__

	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_oDateTime,
		const struct DateStamp &a_roPlatformDate);

#elif defined(__linux__)

	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_oDateTime,
		const time_t &a_roPlatformDate);

#else /* ! __linux__ */

	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_oDateTime,
		const FILETIME &a_roPlatformDate);

#endif /* ! __linux__ */

};

/* An instance of this class represents a number of TEntry classes and is filled */
/* in by the RDir class */

class TEntryArray
{
	TInt				iCount;		// TODO: CAW - Should this be in the StdList? Why does list have a ptr to this class?
	StdList<TEntry>		iEntries;	/* List of entries */

public:

	TEntryArray();

	/* Note that having a destructor in T classes is non standard, but this is required */
	/* to free the iEntries array, and this class is really private to RDir anyway */

	~TEntryArray();

	TEntry *Append(const char *a_pccName);

	TInt Count() const;

	const TEntry &operator[](TInt a_iIndex) const;

	const TEntry *GetHead() const;

	const TEntry *GetSucc(const TEntry *a_poEntry) const;

	void Purge();

	void Remove(const TEntry *a_poEntry);
};

/* A class for scanning directories for directory and file entries */

class RDir
{
public:

	RDir();

	TInt Open(const char *a_pccPattern);

	void Close();

	// TODO: CAW - Is it possible to make this more Symbian like?
	TInt Read(TEntryArray *&a_rpoEntries);

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
};

#endif /* ! DIR_H */
