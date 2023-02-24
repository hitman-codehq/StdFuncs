
#ifndef MUNGWALL_H
#define MUNGWALL_H

/** @file */

#include <stdint.h>
#include <stdlib.h>

#ifdef _MSC_VER

/* Disable a bogus MSVC warning */

#pragma warning(disable : 4291) /* no matching operator delete found */

#endif /* _MSC_VER */

/* Normally we could just use the _NOEXCEPT macro for compatibility with compilers that do or */
/* don't support the noexcept keyword, but MungWall is "special" in that it is included before */
/* the system headers, so it is not available and we have to emulate it ourselves.  This is */
/* made more complicated by the fact that we need to support a pretty old compiler on Amiga OS */

#if __cplusplus >= 201103L || _MSC_VER >= 1900

#define NEW_THROW
#define DELETE_THROW noexcept

#elif defined(_MSC_VER)

#define NEW_THROW
#define DELETE_THROW throw()

#else /* ! __cplusplus >= 201103L || _MSC_VER >= 1900 */

#include <new>

#define NEW_THROW throw(std::bad_alloc)
#define DELETE_THROW throw()

#endif /* ! __cplusplus >= 201103L || _MSC_VER >= 1900 */

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

#ifdef __cplusplus

#define MungeSize 100

class MungWall
{
	struct Arena
	{
		const char *pccSourceFile;
		int iSourceLine;
		size_t stOrigSize;
		struct Arena *paPrev;
		struct Arena *paNext;
	 };

	BOOL bEnableOutput;
	BOOL bEnableProfiling;
	ULONG ulNumProfiledNews;
	ULONG ulNumNews;
	struct Arena *paFirstArena;
	BOOL CheckBlockValidity(void *);
	ULONG CheckOverWrite(UBYTE *);
	void CheckOverWrites(struct Arena *);
	void MungeMem(uint32_t *, size_t);

public:

	MungWall() { bEnableOutput = TRUE; bEnableProfiling = FALSE; ulNumNews = 0; paFirstArena = NULL; }
	~MungWall();

	ULONG NumProfiledNews()
	{
		return(ulNumProfiledNews);
	}

	void EnableOutput(BOOL bEnable);
	void EnableProfiling(BOOL bEnable);
	void Free(void *pvBlock, const char *pccSourceFile = NULL, int iSourceLine = 0, BOOL bHasSource = FALSE);
	void *New(size_t stSize, const char *pccSourceFile, int iSourceLine);
	void *ReNew(void *pvBlock, size_t stSize, const char *pccSourceFile, int iSourceLine);
};

extern MungWall oMungWall;

void *operator new(size_t, const char *, int) NEW_THROW;
void *operator new [](size_t, const char *, int) NEW_THROW;

#if defined(_MSC_VER) && (_MSC_VER < 1800)

/* This is heavy wizardry that is required in order to get MungWall working with older MSVC compilers */
/* such as MSVC 6 and MSVC 2008 and also with the latest C++ compilers such as GCC 4.6.x.  One won't */
/* compile without these function declarations and one won't compile with them (but only if the Standard */
/* C++ library is also used! */

void *operator new(size_t);
void *operator new [](size_t);

#endif

extern "C"
{

#endif /* __cplusplus */

void *DebugMalloc(size_t stSize, const char *pccSourceFile, int iSourceLine);
void *DebugReAlloc(void *pvBlock, size_t stSize, const char *pccSourceFile, int iSourceLine);
void DebugFree(void *pvBlock, const char *pccSourceFile, int iSourceLine);

#ifdef __cplusplus

}

#endif /* __cplusplus */

#define malloc(ulSize) DebugMalloc(ulSize, __FILE__, __LINE__)
#define realloc(pvBlock, ulSize) DebugReAlloc(pvBlock, ulSize, __FILE__, __LINE__)
#define free(pvBlock) DebugFree(pvBlock, __FILE__, __LINE__)

#ifndef MUNGWALL_NO_LINE_TRACKING

/* More heavy wizardry required to allow tracking of filenames and line numbers under MSVC 2013. */
/* This feature doesn't work under older MSVC compilers or under GCC if the Standard C++ library */
/* is also used! */

#define new new(__FILE__, __LINE__)

#endif

#endif /* defined(_DEBUG) && !defined(QT_GUI_LIB) */

#endif /* ! MUNGWALL_H */
