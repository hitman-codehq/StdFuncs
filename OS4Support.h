
#ifndef OS4SUPPORT_H
#define OS4SUPPORT_H

#if defined(__amigaos__) && !defined(__amigaos4__)

APTR AllocSysObject(ULONG a_type, const struct TagItem *a_tags);
ULONG DateStampToSeconds(const struct DateStamp *a_dateStamp);
void FreeSysObject(ULONG a_type, APTR a_object);

/* New entries in dos/dos.h */

#define EXDB_NO_READ		3
#define EXDB_NO_WRITE		2
#define EXDB_NO_EXECUTE		1
#define EXDB_NO_DELETE		0

#define EXDF_NO_READ		(1 << EXDB_NO_READ)
#define EXDF_NO_WRITE		(1 << EXDB_NO_WRITE)
#define EXDF_NO_EXECUTE		(1 << EXDB_NO_EXECUTE)
#define EXDF_NO_DELETE		(1 << EXDB_NO_DELETE)

#define ZERO				((BPTR) 0)

/* New entries in exec/exectags.h */

#define ASOPORT_Pri			(TAG_USER + 13)
#define ASOPORT_Name		(TAG_USER + 14)

enum enAllocSysObjectTypes
{
	ASOT_PORT		= 6,
	ASOT_SEMAPHORE	= 8
};

inline APTR AllocSysObjectTags(ULONG a_type, Tag a_tag, ...)
{
	return AllocSysObject(a_type, (struct TagItem *) &a_tag);
}

ULONG CopyStringBSTRToC(BSTR a_bstring, STRPTR a_dest, ULONG a_size);

#endif /* defined(__amigaos__) && !defined(__amigaos4__) */

#endif /* ! OS4SUPPORT_H */
