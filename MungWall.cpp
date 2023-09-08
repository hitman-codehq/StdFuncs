
#include "StdFuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Utils.h"

#undef malloc
#undef free
#undef new

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

class MungWall oMungWall;

/* Error message strings that can be used in multiple places */

static const char accAllocating[] = "*** MungWall info: Allocating %zu bytes in file %s, line %d";
static const char accLeakage[] = "*** MungWall alert: Severe memory leakage, cleaned up %zu bytes in %lu allocation(s) manually";
static const char accStillAllocatedLine[] = "*** MungWall alert: File %s, line %d: %zd bytes still allocated";
static const char accStillAllocated[] = "*** MungWall alert: File %s: %zd bytes still allocated";
static const char accInvalidBlockLine[] = "MungWall alert: File %s, line %d: Invalid block passed in";
static const char accInvalidBlock[] = "*** MungWall alert: Invalid block passed in";

/**********************************************************************/
/* MungWall::~MungWall will check to see if there are any outstanding */
/* memory allocations and will print a list of any that exist.        */
/* Written: Thursday 11-Dec-1997 2:21 pm                              */
/**********************************************************************/

MungWall::~MungWall()
{
	void *pvBlock;
	size_t stBytesLeaked;
	ULONG ulNews;
	struct Arena *paArena;

	if (bEnableOutput)
	{
		if (ulNumNews > 100)
		{
			ulNews = ulNumNews;
			paArena = paFirstArena;
			stBytesLeaked = 0;

			while (paArena)
			{
				stBytesLeaked += paArena->stOrigSize;
				pvBlock = ((UBYTE*) paArena + sizeof(struct Arena) + MungeSize);
				paArena = paArena->paNext;
				Free(pvBlock);
			}

			Utils::info(accLeakage, stBytesLeaked, ulNews);
			printf(accLeakage, stBytesLeaked, ulNews);
			printf("\n");
		}
		else
		{
			paArena = paFirstArena;

			while (paArena)
			{
				if (paArena->iSourceLine)
				{
					Utils::info(accStillAllocatedLine, paArena->pccSourceFile, paArena->iSourceLine, paArena->stOrigSize);
					printf(accStillAllocatedLine, paArena->pccSourceFile, paArena->iSourceLine, paArena->stOrigSize);
					printf("\n");
				}
				else
				{
					Utils::info(accStillAllocated, paArena->pccSourceFile, paArena->stOrigSize);
					printf(accStillAllocated, paArena->pccSourceFile, paArena->stOrigSize);
					printf("\n");
				}

				paArena = paArena->paNext;
			}
		}
	}

	/* Ensure that the arena list is empty, just in case any further calls are made to MungWall */

	paFirstArena = NULL;
}

/***************************************************************************/
/* MungWall::CheckBlockValidity will check that the block is on the list   */
/* of allocated blocks.                                                    */
/* Written: Friday 12-Dec-1997 10:34 am                                    */
/* Passed: pvBlock => Ptr to the user block to check.  Not that to get the */
/*                    ptr to the actual block that was allocated, you must */
/*                    subtract the size of the Mungwall and the size of    */
/*                    the Arena header from this                           */
/* Returns: TRUE if the block is valid, else FALSE                         */
/***************************************************************************/

BOOL MungWall::CheckBlockValidity(void *pvBlock)
{
	BOOL bRetVal = FALSE;
	struct Arena *paUserArena, *paTempArena = paFirstArena;

	paUserArena = (struct Arena *) ((UBYTE *) pvBlock - sizeof(struct Arena) - MungeSize);

	while ((paTempArena) && (!(bRetVal)))
	{
		if (paUserArena == paTempArena)
		{
			bRetVal = TRUE;
		}
		else
		{
			paTempArena = paTempArena->paNext;
		}
	}

	return(bRetVal);
}

/*******************************************************************/
/* MungWall::CheckOverWrite will check a buffer of MungeSize bytes */
/* to see if any bytes have been overwritten.                      */
/* Written: Thursday 11-Dec-1997 4:38 pm                           */
/* Passed: pubBuffer => Ptr to buffer to check for overwrites      */
/* Returns: # of bytes overwritten, or 0 if memory is ok           */
/*******************************************************************/

ULONG MungWall::CheckOverWrite(UBYTE *pubBuffer)
{
	ULONG ulIndex, ulNumOverWrites = 0;

	for (ulIndex = 0; ulIndex < MungeSize; ulIndex += 4)
	{
		if (*(ULONG *) (pubBuffer + ulIndex) != 0xdeadbeef)
		{

#ifdef __amigaos__

			if (pubBuffer[ulIndex + 0] != 0xde) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 1] != 0xad) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 2] != 0xbe) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 3] != 0xef) ++ulNumOverWrites;

#else /* ! __amigaos__ */

			if (pubBuffer[ulIndex + 3] != 0xde) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 2] != 0xad) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 1] != 0xbe) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 0] != 0xef) ++ulNumOverWrites;

#endif /* ! __amigaos__ */

		}
	}

	return(ulNumOverWrites);
}

/***************************************************************************/
/* MungWall::CheckOverWrites will check the buffers on either side of the  */
/* user buffer for overwrites and will print an error if any overwrites    */
/* are detected.                                                           */
/* Written: Thursday 11-Dec-1997 4:05 pm                                   */
/* Passed: paArena => Ptr to arena to check for overwrites                 */
/***************************************************************************/

void MungWall::CheckOverWrites(struct Arena *paArena)
{
	char acMessage[256];
	ULONG ulNumEndOverWrites, ulNumStartOverWrites, *pulBuffer;

	ulNumStartOverWrites = CheckOverWrite((UBYTE *) (paArena + 1));
	pulBuffer = (ULONG *) ((UBYTE *) paArena + sizeof(struct Arena) + MungeSize + paArena->stOrigSize);
	ulNumEndOverWrites = CheckOverWrite((UBYTE *) pulBuffer);

	if ((ulNumStartOverWrites) || (ulNumEndOverWrites))
	{
		sprintf(acMessage, "*** MungWall alert: File %s, line %d:", paArena->pccSourceFile, paArena->iSourceLine);

		if (ulNumStartOverWrites)
		{
			sprintf(&acMessage[strlen(acMessage)], ", %ld byte%s before allocation", ulNumStartOverWrites, ((ulNumStartOverWrites > 1) ? "s" : ""));
		}

		if (ulNumEndOverWrites)
		{
			if (ulNumStartOverWrites)
			{
				sprintf(&acMessage[strlen(acMessage)], " and");
			}

			sprintf(&acMessage[strlen(acMessage)], " %ld byte%s after allocation", ulNumEndOverWrites, ((ulNumEndOverWrites > 1) ? "s" : ""));
		}

		sprintf(&acMessage[strlen(acMessage)], " overwritten");
		Utils::info(acMessage);
		printf("%s\n", acMessage);
	}
}

/**
 * Enables or disables reporting.
 * This method can be called to disable reporting of memory leaks in
 * situations where such reporting is not desired.  For instance, when
 * performing a call to exit() after a failed unit test when there will
 * be leaks due to cleanup methods and destructors not being called,
 * you may wish to observe the output of the test without lots of false
 * memory leaks being reported.  Reporting memory leaks is enabled by
 * default.
 *
 * @date	Monday 08-Aug-2022 7:38 am, Code HQ Tokyo Tsukuda
 * @param	bEnableOutput	TRUE to enable reporting, else FALSE
 */

void MungWall::EnableOutput(BOOL bEnable)
{
	bEnableOutput = bEnable;
}

/**
 * Enables or disables profiling.
 * This method can be called to enable or disable reporting of allocations.
 * Reporting allocatings is disabled by default as it can result in a lot of
 * debug logs being printed, but it can be useful to temporarily enable it to
 * check what is being allocated in performance critical parts of the code.
 *
 * @date	Tuesday 27-Sep-2022 7:15 am, Code HQ Tsukuda
 * @param	bEnableOutput	TRUE to enable reporting, else FALSE
 */

void MungWall::EnableProfiling(BOOL bEnable)
{
	bEnableProfiling = bEnable;

	if (bEnable)
	{
		ulNumProfiledNews = 0;
	}
}

/**********************************************************************/
/* MungWall::Free is called to delete an arena.  It will call         */
/* CheckOverWrites to ensure that the user buffer has not been        */
/* overwritten, and will munge the memory after it is freed.          */
/* Written: Thursday 11-Dec-1997 2:16 pm                              */
/* Passed: pvBlock => Ptr to user buffer to free.  The size of the    */
/*                    munge wall and the Arena header is subtracted   */
/*                    from this to get a pointer to the arena         */
/*         pccSourceFile => Name of the source file which it doing    */
/*                          the delete, if available                  */
/*         iSourceLine => Line # in the source file from which new is */
/*                        being called, if available                  */
/*         bHasSource => TRUE if the pccSourceFile and iSourceLine    */
/*                       members are valid, else FALSE                */
/**********************************************************************/

void MungWall::Free(void *pvBlock, const char *pccSourceFile, int iSourceLine, BOOL bHasSource)
{
	struct Arena *paArena = (struct Arena *) ((UBYTE *) pvBlock - sizeof(struct Arena) - MungeSize);

	/* If the arena list is empty then return without doing anything.  This probably means that we are */
	/* being called after MungWall has been destroyed, so nothing can be done anyway */

	if (!paFirstArena)
	{
		return;
	}

	if (pvBlock)
	{
		if (CheckBlockValidity(pvBlock))
		{
			CheckOverWrites(paArena);
			--ulNumNews;

			if (paArena == paFirstArena)
			{
				paFirstArena = paArena->paNext;
			}

			if (paArena->paPrev)
			{
				paArena->paPrev->paNext = paArena->paNext;
			}

			if (paArena->paNext)
			{
				paArena->paNext->paPrev = paArena->paPrev;
			}

			MungeMem((uint32_t *) paArena, paArena->stOrigSize);
			free(paArena);
		}
		else
		{
			if (bHasSource)
			{
				Utils::info(accInvalidBlockLine, pccSourceFile, iSourceLine);
				printf(accInvalidBlockLine, pccSourceFile, iSourceLine);
				printf("\n");
			}
			else
			{
				Utils::info(accInvalidBlock);
				printf("%s\n", accInvalidBlock);
			}
		}
	}
}

/********************************************************/
/* MungWall::MungeMem will munge a block of memory.     */
/* Written: Thursday 11-Dec-1997 3:56 pm                */
/* Passed: puiBuffer => Ptr to the buffer to munge      */
/*         stBufferSize => Size of the buffer, in bytes */
/********************************************************/

void MungWall::MungeMem(uint32_t *puiBuffer, size_t stBufferSize)
{
	ULONG ulIndex;

	for (ulIndex = 0; ulIndex < (stBufferSize / 4); ++ulIndex)
	{
		puiBuffer[ulIndex] = 0xdeadbeef;
	}
}

/***************************************************************************/
/* MungWall::New will allocate a block of memory to be used for the user's */
/* buffer, the Arena header and the munge walls on either side of the      */
/* user's buffer.  It will munge the walls and add the block to the list   */
/* of allocated blocks.                                                    */
/* Written: Thursday 11-Dec-1997 2:11 pm                                   */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pccSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else FALSE             */
/***************************************************************************/

void *MungWall::New(size_t stSize, const char *pccSourceFile, int iSourceLine)
{
	UBYTE *pubBlock;
	size_t stMungedSize = (sizeof(struct Arena) + (MungeSize * 2) + stSize);
	void *pvRetVal = NULL;
	struct Arena *paBlock;

	/* Account for the wraparound that will happen on 32 bit systems, if someone tries to allocate a */
	/* very large amount of memory, such as 0xffffffff bytes */

	if (stMungedSize < stSize)
	{
		return(NULL);
	}

	if (bEnableProfiling)
	{
		++ulNumProfiledNews;
		Utils::info(accAllocating, stSize, pccSourceFile, iSourceLine);
		printf(accAllocating, stSize, pccSourceFile, iSourceLine);
		printf("\n");
	}

	pubBlock = (UBYTE *) malloc(stMungedSize);

	if (pubBlock)
	{
		memset(pubBlock, 0, stMungedSize);
		MungeMem((uint32_t *) (pubBlock + sizeof(struct Arena)), MungeSize);
		MungeMem((uint32_t *) (pubBlock + sizeof(struct Arena) + MungeSize + stSize), MungeSize);
		paBlock = (struct Arena *) pubBlock;
		paBlock->pccSourceFile = pccSourceFile;
		paBlock->iSourceLine = iSourceLine;
		paBlock->stOrigSize = stSize;

		if (paFirstArena)
		{
			paBlock->paPrev = NULL;
			paBlock->paNext = paFirstArena;
			paFirstArena->paPrev = paBlock;
			paFirstArena = paBlock;
		}
		else
		{
			paBlock->paPrev = NULL;
			paBlock->paNext = NULL;
			paFirstArena = paBlock;
		}

		++ulNumNews;
		pvRetVal = (pubBlock + MungeSize + sizeof(struct Arena));
	}

	return(pvRetVal);
}

/*****************************************************************************/
/* MungWall::ReNew will allocate a block of memory to be used for the user's */
/* buffer, the Arena header and the munge walls on either side of the user's */
/* buffer.  It will munge the walls and add the block to the list of         */
/* allocated blocks.  It will also copy an already existing (smaller) buffer */
/* into the newly allocated one                                              */
/* Written: Tuesday 26-May-2007 11:22 pm, College Close                      */
/* Passed: pvBlock => Ptr to the user's old buffer to reallocated            */
/*         stSize => Number of bytes to allocate for the *user's buffer*     */
/*         pccSourceFile => Name of the source file which is doing the new   */
/*         iSourceLine => Line # in the source file from which new is        */
/*                        being called                                       */
/* Returns: Ptr to the user's buffer if successful, else FALSE               */
/*****************************************************************************/

void *MungWall::ReNew(void *pvBlock, size_t stSize, const char *pccSourceFile, int iSourceLine)
{
	void *RetVal;
	struct Arena *paArena = (struct Arena *) ((UBYTE *) pvBlock - sizeof(struct Arena) - MungeSize);

	/* Allocate a new block into which to copy the old block's data */

	RetVal = oMungWall.New(stSize, pccSourceFile, iSourceLine);

	/* If successful, copy the old block's data into it, taking into account that the */
	/* new block may be either larger or smaller than the old one */

	if (RetVal)
	{
		memcpy(RetVal, pvBlock, (paArena->stOrigSize < stSize) ? paArena->stOrigSize : stSize);
	}

	/* And delete the old block */

	Free(pvBlock);

	return(RetVal);
}

/****************************************************************************/
/* DebugFree will intercept all frees done and will call the                */
/* Delete method of the oMungWall object to handle the free.                */
/* Written: Tuesday 16-Dec-1997 9:33 pm                                     */
/* Passed: pvBlock => Ptr to the user's buffer to delete                    */
/*         pccSourceFile => Name of the source file which is doing the free */
/*         iSourceLine => Line # in the source file from which free is      */
/*                        being called                                      */
/****************************************************************************/

void DebugFree(void *pvBlock, const char *pccSourceFile, int iSourceLine)
{
	oMungWall.Free(pvBlock, pccSourceFile, iSourceLine, TRUE);
}

/***************************************************************************/
/* DebugMalloc will intercept all mallocs done and will call the New       */
/* method of the oMungWall object to handle the malloc.                    */
/* Written: Tuesday 16-Dec-1997 9:32 pm                                    */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pccSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else NULL              */
/***************************************************************************/

void *DebugMalloc(size_t stSize, const char *pccSourceFile, int iSourceLine)
{
	return(oMungWall.New(stSize, pccSourceFile, iSourceLine));
}

/***************************************************************************/
/* DebugReAlloc will intercept all reallocs done and will call the ReNew   */
/* method of the oMungWall object to handle the malloc.                    */
/* Written: Tuesday 26-May-2007 11:20 pm, College Close                    */
/* Passed: pvBlock => Ptr to the user's old buffer to reallocated          */
/*         stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pccSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else NULL              */
/***************************************************************************/

void *DebugReAlloc(void *pvBlock, size_t stSize, const char *pccSourceFile, int iSourceLine)
{
	return(oMungWall.ReNew(pvBlock, stSize, pccSourceFile, iSourceLine));
}

/***************************************************************************/
/* new will intercept all global news done and will call the New method    */
/* of the oMungWall object to handle the new.                              */
/* Written: Thursday 11-Dec-1997 2:12 pm                                   */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pccSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else NULL              */
/***************************************************************************/

void *operator new(size_t stSize, const char *pccSourceFile, int iSourceLine) NEW_THROW
{
	return(oMungWall.New(stSize, pccSourceFile, iSourceLine));
}

/***************************************************************************/
/* new will intercept all global array news done and will call the New     */
/* method of the oMungWall object to handle the new.                       */
/* Written: Thursday 11-Dec-1997 2:13 pm                                   */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pccSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else NULL              */
/***************************************************************************/

void *operator new [](size_t stSize, const char *pccSourceFile, int iSourceLine) NEW_THROW
{
	return(oMungWall.New(stSize, pccSourceFile, iSourceLine));
}

#endif /* defined(_DEBUG) && !defined(QT_GUI_LIB) */

/**************************************************************************/
/* new will intercept all global news done and will call the New method   */
/* of the oMungWall object to handle the new.                             */
/* Written: Monday 29-Jun-1998 9:42 pm                                    */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new(size_t stSize) NEW_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	return(oMungWall.New(stSize, "Anonymous", 0));

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	void *pvRetVal;

	if ((pvRetVal = malloc(stSize)) != NULL)
	{
		memset(pvRetVal, 0, stSize);
	}

	return(pvRetVal);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

/**************************************************************************/
/* new will intercept all global array news done and will call the New    */
/* method of the oMungWall object to handle the new.                      */
/* Written: Monday 29-Jun-1998 9:43 pm                                    */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new [](size_t stSize) NEW_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	return(oMungWall.New(stSize, "Anonymous", 0));

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	void *pvRetVal;

	if ((pvRetVal = malloc(stSize)) != NULL)
	{
		memset(pvRetVal, 0, stSize);
	}

	return(pvRetVal);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

/**************************************************************************/
/* delete will intercept all global deletes done and will call the Delete */
/* method of the oMungWall object to handle the delete.                   */
/* Written: Thursday 11-Dec-1997 2:10 pm                                  */
/* Passed: pvBlock => Ptr to the user's buffer to delete                  */
/**************************************************************************/

void operator delete(void *pvBlock) DELETE_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	oMungWall.Free(pvBlock);

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	free(pvBlock);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

/*************************************************************************/
/* delete will intercept all global array deletes done and will call the */
/* Delete method of the oMungWall object to handle the delete.           */
/* Written: Thursday 11-Dec-1997 2:11 pm                                 */
/* Passed: pvBlock => Ptr to the user's buffer to delete                 */
/*************************************************************************/

void operator delete [](void *pvBlock) DELETE_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	oMungWall.Free(pvBlock);

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	free(pvBlock);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

#if __cplusplus >= 201103

/**
 * Deletes a block of memory.
 * Intercepts all global deletes and will calls Mungwall::Free() to handle the deletion.
 *
 * @date	Thursday 30-May-2019 4:20 pm, Code HQ Bergmannstrasse
 * @param	pvBlock			Pointer to the user's buffer to delete
 */

void operator delete(void *pvBlock, std::size_t /*stSize*/) DELETE_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	oMungWall.Free(pvBlock);

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	free(pvBlock);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

/**
 * Deletes a block of memory containing an array of objects.
 * Intercepts all global array deletes and will calls Mungwall::Delete() to handle the deletion.
 *
 * @date	Thursday 30-May-2019 4:25 pm, Code HQ Bergmannstrasse
 * @param	pvBlock			Pointer to the user's buffer to delete
 * @param	stSize			The number of elements in the array to be deleted
 */

void operator delete [](void *pvBlock, std::size_t /*stSize*/) DELETE_THROW
{

#if defined(_DEBUG) && !defined(QT_GUI_LIB)

	oMungWall.Free(pvBlock);

#else /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

	free(pvBlock);

#endif /* ! defined(_DEBUG) && !defined(QT_GUI_LIB) */

}

#endif /* __cplusplus >= 201103 */
