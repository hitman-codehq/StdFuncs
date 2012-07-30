
#include <new.h>
#include "StdFuncs.h"
#include "StdPool.h"

/* Written: Sunday 13-May-2012 2:18 pm, Lindau (Bodensee) */
/* @param	a_iSize			Size of items in the pool to be created */
/*			a_iNumItems		# of items that can be held in the pool */
/*			a_bExtensible	ETrue if the pool can be extended beyond its */
/*							initial size, else EFalse */
/* @return	KErrNone if successful, else KErrNoMemory */
/* Initialises a pool that is large enough to contain a_iNumItems of */
/* size a_iSize.  When requests are made from the pool for items, they */
/* will be returned from the pool and the pool will be automatically */
/* enlarged if required */

TInt RStdPool::Create(TInt a_iSize, TInt a_iNumItems, TBool a_bExtensible)
{
	char *Buffer;
	TInt Index, RetVal;
	CPoolNode *PoolNode;

	ASSERTM((a_iSize >= sizeof(CPoolNode)), "RStdPool::Create() => Node size is too small");

	m_bExtensible = a_bExtensible;
	m_iSize = a_iSize;

	m_pcBuffer = Buffer = new char[a_iSize * a_iNumItems];

	RetVal = (m_pcBuffer != NULL) ? KErrNone : KErrNoMemory;

	if (RetVal == KErrNone)
	{
// TODO: CAW - Hmmm.  What to do about this?
#undef new

		for (Index = 0; Index < a_iNumItems; ++Index)
		{
			PoolNode = new(Buffer) CPoolNode;
			m_oNodes.AddTail(PoolNode);
			Buffer += a_iSize;
		}
	}

	return(RetVal);
}

/* Written: Sunday 03-Jun-2012 11:27 am, On train to Munich Deutsches Museum */
/* @param	a_bFreeNodes	If ETrue then all nodes referring to the pool are explicitly */
/*							freed, otherwise they are implicitly freed */
/* Frees the resources allocated by the class and makes it ready for use again.  After */
/* this is called, the class is back to the state it was in when it had just been */
/* initialised - that is, the constructor had been called but *not* RStdPool::Create(). */
/* You should be careful with the use of the a_bFreeNodes parameter.  This is provided */
/* mainly as an optimisation for clients that have a large number (ie. In the millions) */
/* of allocated nodes.  If ETrue then each node will be explicitly freed from the internal */
/* list, thus affecting the result of the RStdPool::Count() function.  If EFalse then the */
/* internal list will be forcibly reset, which is much faster.  ETrue should only be used */
/* when testing and in debug builds, if you need to ensure that all nodes are freed for */
/* resource tracking purposes.  Either way, you should not ever access the contents of a */
/* node allocated from a pool after RStdPool::Close() has been called, as its memory will */
/* have been freed and is now invalid */

void RStdPool::Close(TBool a_bFreeNodes)
{
	CPoolNode *Node;

	/* If required, manually iterate through the list and free each and every */
	/* node */

	if (a_bFreeNodes)
	{
		while ((Node = m_oNodes.RemHead()) != NULL)
		{
		}
	}

	/* Otherwise just hard reset the list back to its original state */

	else
	{
		m_oNodes.Reset();
	}

	/* Free the buffer used by the pool for its nodes */

	delete [] m_pcBuffer;
	m_pcBuffer = NULL;

	/* And reset the other variables back to their defaults */

	m_bExtensible = EFalse;
	m_iSize = 0;
}

/* Written: Sunday 03-Jun-2012 11:20 pm, On train to Munich Deutsches Museum */

void *RStdPool::GetNode()
{
	CPoolNode *RetVal;

	RetVal = m_oNodes.GetHead();

	if (RetVal)
	{
		m_oNodes.Remove(RetVal);
		memset(RetVal, 0, m_iSize);
	}

	return(RetVal);
}
