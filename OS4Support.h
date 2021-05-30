
#ifndef OS4SUPPORT_H
#define OS4SUPPORT_H

#if defined(__amigaos__) && !defined(__amigaos4__)

APTR AllocSysObject(ULONG a_type, const struct TagItem *a_tags);
ULONG DateStampToSeconds(const struct DateStamp *a_dateStamp);
void FreeSysObject(ULONG a_type, APTR a_object);

/* New entries in exec/exectags.h */

#define ASOPORT_Pri         (TAG_USER + 13)
#define ASOPORT_Name        (TAG_USER + 14)

enum enAllocSysObjectTypes
{
	ASOT_PORT		= 6,
	ASOT_SEMAPHORE	= 8
};

inline APTR AllocSysObjectTags(ULONG a_type, Tag a_tag, ...)
{
	return AllocSysObject(a_type, (struct TagItem *) &a_tag);
}

#endif /* defined(__amigaos__) && !defined(__amigaos4__) */

#endif /* ! OS4SUPPORT_H */
