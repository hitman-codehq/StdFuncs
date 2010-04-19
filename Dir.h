
#ifndef DIR_H
#define DIR_H

#include "StdList.h"
#include "Time.h"

/* An instance of this class represents a directory or file name and its associated */
/* attributes, such as name etc.  It is filled in by the RDir class */

class TEntry
{
public:

	StdListNode<TEntry>	m_oStdListNode;		/* Standard list node */
	const char			*iName;				/* Ptr to the name of the file */
	TBool				iIsDir;				/* ETrue if entry is a directory */
	TBool				iIsLink;			/* ETrue if entry is a link */
	TUint				iSize;				/* File size in bytes */
	TUint				iAttributes;		/* Protection attributes */
	TTime				iModified;			/* Time and date of the file */

#ifdef __amigaos4__

	// TODO: CAW - I don't like this, nor the Set() function below
	struct DateStamp	iAmigaDate;			/* Date and time in Amiga specific format */

#else /* ! __amigaos4__ */

	FILETIME			iWindowsDate;		/* Date and time in Windows specific format */

#endif /* ! __amigaos4__ */

	TEntry();

	~TEntry();

	TBool IsDir() const;

	TBool IsLink() const;

	TBool IsHidden() const;

#ifdef __amigaos4__

	// TODO: CAW - Types?
	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, TDateTime &a_oDateTime,
		struct DateStamp &a_roAmigaDate);

#else /* ! __amigaos4__ */

	void Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, TDateTime &a_oDateTime,
		FILETIME &a_roWindowsDate);

#endif /* ! __amigaos4__ */

};

/* An instance of this class represents a number of TEntry classes and is filled */
/* in by the RDir class */

class TEntryArray
{
	int					iCount;		// TODO: CAW - Should this be in the StdList? Why does list have a ptr to this class?
	StdList<TEntry>		iEntries;	/* Standard list node */

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

#else /* ! __amigaos4__ */

	HANDLE			iHandle;		/* Handle used for scanning directory */

#endif /* __amigaos4__ */

	TEntry			iSingleEntry;	/* If a single entry is being examined, Open() will populate this */
	bool			iSingleEntryOk;	/* true if the contents of iSingleEntry are valid, else false */
	TEntryArray		iEntries;		/* Array of TEntry classes containing directory and file information */
};

#endif /* ! DIR_H */
