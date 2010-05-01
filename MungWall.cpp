
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/exec.h>

#endif /* __amigaos4__ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Utils.h"

#undef malloc
#undef free
#undef new
#undef AllocMem
#undef FreeMem

#ifdef _DEBUG

class MungWall oMungWall;

/**********************************************************************/
/* MungWall::~MungWall will check to see if there are any outstanding */
/* memory allocations and will print a list of any that exist.        */
/* Written: Thursday 11-Dec-1997 2:21 pm                              */
/**********************************************************************/

MungWall::~MungWall()
{
	void *pvBlock;
	ULONG ulBytesLeaked, ulNews;
	struct Arena *paArena;

	if (ulNumNews > 100)
	{
		ulNews = ulNumNews;
		paArena = paFirstArena;
		ulBytesLeaked = 0;

		while (paArena)
		{
			ulBytesLeaked += paArena->stOrigSize;
			pvBlock = ((UBYTE *) paArena + sizeof(struct Arena) + MungeSize);
			paArena = paArena->paNext;
			Delete(pvBlock);
		}

		Utils::Info("*** MungWall alert: Severe memory leakage, cleaned up %lu bytes in %lu allocation(s) manually", ulBytesLeaked, ulNews);
	}
	else
	{
		paArena = paFirstArena;

		while (paArena)
		{
			if (paArena->iSourceLine)
			{
				Utils::Info("*** MungWall alert: File %s, line %d: %d bytes still allocated", paArena->pcSourceFile, paArena->iSourceLine, paArena->stOrigSize);
			}
			else
			{
				Utils::Info("*** MungWall alert: File %s: %d bytes still allocated", paArena->pcSourceFile, paArena->stOrigSize);
			}

			paArena = paArena->paNext;
		}
	}
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
			if (pubBuffer[ulIndex] != 0xde) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 1] != 0xad) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 2] != 0xbe) ++ulNumOverWrites;
			if (pubBuffer[ulIndex + 3] != 0xef) ++ulNumOverWrites;
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
		sprintf(acMessage, "*** MungWall alert: File %s, line %d:", paArena->pcSourceFile, paArena->iSourceLine);

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
		Utils::Info(acMessage);
	}
}

/**********************************************************************/
/* MungWall::Delete is called to delete an arena.  It will call       */
/* CheckOverWrites to ensure that the user buffer has not been        */
/* overwritten, and will munge the memory after it is freed.          */
/* Written: Thursday 11-Dec-1997 2:16 pm                              */
/* Passed: pvBlock => Ptr to user buffer to free.  The size of the    */
/*                    munge wall and the Arena header is subtracted   */
/*                    from this to get a pointer to the arena         */
/*         pcSourceFile => Name of the source file which it doing     */
/*                         the delete, if available                   */
/*         iSourceLine => Line # in the source file from which new is */
/*                        being called, if available                  */
/*         bHasSource => TRUE if the pcSourceFile and iSourceLine     */
/*                       members are valid, else FALSE                */
/**********************************************************************/

void MungWall::Delete(void *pvBlock, char *pcSourceFile, int iSourceLine, BOOL bHasSource)
{
	struct Arena *paArena = (struct Arena *) ((UBYTE *) pvBlock - sizeof(struct Arena) - MungeSize);

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
		else
		{
			MungeMem((ULONG *) paArena, paArena->stOrigSize);
			free(paArena);
		}
	}
	else
	{
		if (bHasSource)
		{
			Utils::Info("MungWall alert: File %s, line %d: Invalid block passed in", pcSourceFile, iSourceLine);
		}
		else
		{
			Utils::Info("*** MungWall alert: Invalid block passed in");
		}
	}
}

/********************************************************/
/* MungWall::MungeMem will munge a block of memory.     */
/* Written: Thursday 11-Dec-1997 3:56 pm                */
/* Passed: pulBuffer => Ptr to the buffer to munge      */
/*         ulBufferSize => Size of the buffer, in bytes */
/********************************************************/

void MungWall::MungeMem(ULONG *pulBuffer, ULONG ulBufferSize)
{
	ULONG ulIndex;

	for (ulIndex = 0; ulIndex < (ulBufferSize / 4); ++ulIndex)
	{
		pulBuffer[ulIndex] = 0xdeadbeef;
	}
}

/***************************************************************************/
/* MungWall::New will allocate a block of memory to be used for the user's */
/* buffer, the Arena header and the munge walls on either side of the      */
/* user's buffer.  It will munge the walls and add the block to the list   */
/* of allocated blocks.                                                    */
/* Written: Thursday 11-Dec-1997 2:11 pm                                   */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*   */
/*         pcSourceFile => Name of the source file which is doing the new  */
/*         iSourceLine => Line # in the source file from which new is      */
/*                        being called                                     */
/* Returns: Ptr to the user's buffer if successful, else FALSE             */
/***************************************************************************/

void *MungWall::New(size_t stSize, const char *pcSourceFile, int iSourceLine)
{
	UBYTE *pubBlock;
	ULONG ulMungedSize = (sizeof(struct Arena) + (MungeSize * 2) + stSize);
	void *pvRetVal = NULL;
	struct Arena *paBlock;

	pubBlock = (UBYTE *) malloc(ulMungedSize);

	if (pubBlock)
	{
		memset(pubBlock, 0, ulMungedSize);
		MungeMem((ULONG *) (pubBlock + sizeof(struct Arena)), MungeSize);
		MungeMem((ULONG *) (pubBlock + sizeof(struct Arena) + MungeSize + stSize), MungeSize);
		paBlock = (struct Arena *) pubBlock;
		paBlock->pcSourceFile = pcSourceFile;
		paBlock->iSourceLine = iSourceLine;
		paBlock->stOrigSize = stSize;
		paBlock->ulMungedSize = ulMungedSize;

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
/*         pcSourceFile => Name of the source file which is doing the new    */
/*         iSourceLine => Line # in the source file from which new is        */
/*                        being called                                       */
/* Returns: Ptr to the user's buffer if successful, else FALSE               */
/*****************************************************************************/

void *MungWall::ReNew(void *pvBlock, size_t stSize, const char *pcSourceFile, int iSourceLine)
{
	void *RetVal;
	struct Arena *paArena = (struct Arena *) ((UBYTE *) pvBlock - sizeof(struct Arena) - MungeSize);

	/* Allocate a new block into which to copy the old block's data */

	RetVal = oMungWall.New(stSize, pcSourceFile, iSourceLine);

	/* If successful, copy the old block's data into it, taking into account that the */
	/* new block may be either larger or smaller than the old one */

	if (RetVal)
	{
		memcpy(RetVal, pvBlock, (paArena->stOrigSize < stSize) ? paArena->stOrigSize : stSize);
	}

	/* And delete the old block */

	Delete(pvBlock);

	return(RetVal);
}

/***************************************************************************/
/* DebugFree will intercept all frees done and will call the               */
/* Delete method of the oMungWall object to handle the free.               */
/* Written: Tuesday 16-Dec-1997 9:33 pm                                    */
/* Passed: pvBlock => Ptr to the user's buffer to delete                   */
/*         pcSourceFile => Name of the source file which is doing the free */
/*         iSourceLine => Line # in the source file from which free is     */
/*                        being called                                     */
/***************************************************************************/

void DebugFree(void *pvBlock, char *pcSourceFile, int iSourceLine)
{
	oMungWall.Delete(pvBlock, pcSourceFile, iSourceLine, TRUE);
}

/**************************************************************************/
/* DebugMalloc will intercept all mallocs done and will call the New      */
/* method of the oMungWall object to handle the malloc.                   */
/* Written: Tuesday 16-Dec-1997 9:32 pm                                   */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/*         pcSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is     */
/*                        being called                                    */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *DebugMalloc(size_t stSize, char *pcSourceFile, int iSourceLine)
{
	return(oMungWall.New(stSize, pcSourceFile, iSourceLine));
}

/**************************************************************************/
/* DebugReAlloc will intercept all reallocs done and will call the ReNew  */
/* method of the oMungWall object to handle the malloc.                   */
/* Written: Tuesday 26-May-2007 11:20 pm, College Close                   */
/* Passed: pvBlock => Ptr to the user's old buffer to reallocated         */
/*         stSize => Number of bytes to allocate for the *user's buffer*  */
/*         pcSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is     */
/*                        being called                                    */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *DebugReAlloc(void *pvBlock, size_t stSize, char *pcSourceFile, int iSourceLine)
{
	return(oMungWall.ReNew(pvBlock, stSize, pcSourceFile, iSourceLine));
}

/**************************************************************************/
/* new will intercept all global news done and will call the New method   */
/* of the oMungWall object to handle the new.                             */
/* Written: Thursday 11-Dec-1997 2:12 pm                                  */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/*         pcSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is     */
/*                        being called                                    */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new(size_t stSize, const char *pcSourceFile, int iSourceLine)
{
	return(oMungWall.New(stSize, pcSourceFile, iSourceLine));
}

/**************************************************************************/
/* new will intercept all global array news done and will call the New    */
/* method of the oMungWall object to handle the new.                      */
/* Written: Thursday 11-Dec-1997 2:13 pm                                  */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/*         pcSourceFile => Name of the source file which is doing the new */
/*         iSourceLine => Line # in the source file from which new is     */
/*                        being called                                    */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new [](size_t stSize, const char *pcSourceFile, int iSourceLine)
{
	return(oMungWall.New(stSize, pcSourceFile, iSourceLine));
}

#endif /* _DEBUG */

/**************************************************************************/
/* new will intercept all global news done and will call the New method   */
/* of the oMungWall object to handle the new.                             */
/* Written: Monday 29-Jun-1998 9:42 pm                                    */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new(size_t stSize)
{

#ifdef _DEBUG

	return(oMungWall.New(stSize, "Anonymous", 0));

#else /* ! _DEBUG */

	void *pvRetVal;

	if ((pvRetVal = malloc(stSize)) != NULL)
	{
		memset(pvRetVal, 0, stSize);
	}

	return(pvRetVal);

#endif /* ! _DEBUG */

}

/**************************************************************************/
/* new will intercept all global array news done and will call the New    */
/* method of the oMungWall object to handle the new.                      */
/* Written: Monday 29-Jun-1998 9:43 pm                                    */
/* Passed: stSize => Number of bytes to allocate for the *user's buffer*  */
/* Returns: Ptr to the user's buffer if successful, else NULL             */
/**************************************************************************/

void *operator new [](size_t stSize)
{

#ifdef _DEBUG

	return(oMungWall.New(stSize, "Anonymous", 0));

#else /* ! _DEBUG */

	void *pvRetVal;

	if ((pvRetVal = malloc(stSize)) != NULL)
	{
		memset(pvRetVal, 0, stSize);
	}

	return(pvRetVal);

#endif /* ! _DEBUG */

}

/**************************************************************************/
/* delete will intercept all global deletes done and will call the Delete */
/* method of the oMungWall object to handle the delete.                   */
/* Written: Thursday 11-Dec-1997 2:10 pm                                  */
/* Passed: pvBlock => Ptr to the user's buffer to delete                  */
/**************************************************************************/

void operator delete(void *pvBlock)
{

#ifdef _DEBUG

	oMungWall.Delete(pvBlock);

#else /* ! _DEBUG */

	free(pvBlock);

#endif /* ! _DEBUG */

}

/*************************************************************************/
/* delete will intercept all global array deletes done and will call the */
/* Delete method of the oMungWall object to handle the delete.           */
/* Written: Thursday 11-Dec-1997 2:11 pm                                 */
/* Passed: pvBlock => Ptr to the user's buffer to delete                 */
/*************************************************************************/

void operator delete [](void *pvBlock)
{

#ifdef _DEBUG

	oMungWall.Delete(pvBlock);

#else /* ! _DEBUG */

	free(pvBlock);

#endif /* ! _DEBUG */

}
