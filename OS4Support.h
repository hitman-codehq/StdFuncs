
#ifndef OS4SUPPORT_H
#define OS4SUPPORT_H

#if defined(__amigaos__) && !defined(__amigaos4__)

APTR AllocSysObject(ULONG a_type, const struct TagItem *a_tags);
ULONG CopyStringBSTRToC(BSTR a_bstring, STRPTR a_dest, ULONG a_size);
ULONG DateStampToSeconds(const struct DateStamp *a_dateStamp);
void FreeSysObject(ULONG a_type, APTR a_object);
void RefreshSetGadgetAttrsA(struct Gadget *a_gadget, struct Window *a_window, struct Requester *a_requester,
	struct TagItem *a_taglist);

/* New entries in dos/dos.h */

#define EXDB_NO_READ		3
#define EXDB_NO_WRITE		2
#define EXDB_NO_EXECUTE		1
#define EXDB_NO_DELETE		0

#define EXDF_NO_READ		(1 << EXDB_NO_READ)
#define EXDF_NO_WRITE		(1 << EXDB_NO_WRITE)
#define EXDF_NO_EXECUTE		(1 << EXDB_NO_EXECUTE)
#define EXDF_NO_DELETE		(1 << EXDB_NO_DELETE)

/* This macro works like OS4's EXD_IS_LINK macro, but is passed the type itself so that it can be used with both */
/* ExAllData.ed_Type and FileInfoBlock.fib_DirEntryType */

#define TYPE_IS_LINK(Type)	((Type == ST_SOFTLINK) | (Type == ST_LINKDIR) | (Type == ST_LINKFILE))

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
	va_list args;

	va_start(args, a_tag);
	APTR retVal = AllocSysObject(a_type, (struct TagItem *) &a_tag);
	va_end(args);

	return retVal;
}

inline void RefreshSetGadgetAttrs(struct Gadget *a_gadget, struct Window *a_window, struct Requester *a_requester,
	Tag a_tag, ...)
{
	va_list args;

	va_start(args, a_tag);
	RefreshSetGadgetAttrsA(a_gadget, a_window, a_requester, (struct TagItem *) &a_tag);
	va_end(args);
}

#endif /* defined(__amigaos__) && !defined(__amigaos4__) */

#ifdef __amigaos4__

/* Functions renamed in OS4 but still compatible with their OS3 equivalents */

#define GetCurrentDirName(buffer, length) GetCliCurrentDirName(buffer, length)

#endif /* __amigaos4__ */

#endif /* ! OS4SUPPORT_H */
