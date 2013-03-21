
#ifndef MUNGWALL_H
#define MUNGWALL_H

#include <stdlib.h>

#ifdef _MSC_VER

/* Disable a bogus MSVC warning */

#pragma warning(disable : 4291) /* no matching operator delete found */

#endif /* _MSC_VER */

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

#ifdef __cplusplus

#define MungeSize 100

class MungWall
{
	struct Arena
	{
		const char *pcSourceFile;
		int iSourceLine;
		size_t stOrigSize;
		ULONG ulMungedSize;
		struct Arena *paPrev;
		struct Arena *paNext;
	 };

	ULONG ulNumNews;
	struct Arena *paFirstArena;
	BOOL CheckBlockValidity(void *);
	ULONG CheckOverWrite(UBYTE *);
	void CheckOverWrites(struct Arena *);
	void MungeMem(ULONG *, ULONG);

public:

	MungWall() { ulNumNews = 0; paFirstArena = NULL; }
	~MungWall();
	void Delete(void *, char * = NULL, int = 0, BOOL = FALSE);
	void *New(size_t, const char *, int);
	void *ReNew(void *, size_t, const char *, int);
};

extern MungWall oMungWall;

void *operator new(size_t, const char *, int);
void *operator new [](size_t, const char *, int);

extern "C"
{

#endif /* __cplusplus */

void *DebugMalloc(size_t stSize, char *pcSourceFile, int iSourceLine);
void *DebugReAlloc(void *pvBlock, size_t stSize, char *pcSourceFile, int iSourceLine);
void DebugFree(void *pvBlock, char *pcSourceFile, int iSourceLine);
APTR DebugAllocMem(ULONG ulSize, char *pcSourceFile, int iSourceLine);
void DebugFreeMem(APTR apBlock, char *pcSourceFile, int iSourceLine, ULONG ulSize);

#ifdef __cplusplus

}

#endif /* __cplusplus */

#define malloc(ulSize) DebugMalloc(ulSize, __FILE__, __LINE__)
#define realloc(pvBlock, ulSize) DebugReAlloc(pvBlock, ulSize, __FILE__, __LINE__)
#define free(pvBlock) DebugFree(pvBlock, __FILE__, __LINE__)
#define new new(__FILE__, __LINE__)

#endif /* defined(_DEBUG) && !defined(QT_GUI_LIB) */

#endif /* ! MUNGWALL_H */
