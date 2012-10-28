
#include "StdFuncs.h"
#include "StdPool.h"

/* Written: Sunday 13-May-2012 2:18 pm, Lindau (Bodensee) */
/* @param	a_iSize			Size of items in the pool to be created */
/*			a_iNumItems		# of items that can be held in the pool */
/*			a_bExtensible	ETrue if the pool can be extended beyond its */
/*							initial size, else EFalse */
/* @return	KErrNone if successful */
/*			KErrNoMemory if not enough memory was available */
/*			KErrInUse if the Create() has already been called */
/* Initialises a pool that is large enough to contain a_iNumItems of */
/* size a_iSize.  When requests are made from the pool for items, they */
/* will be returned from the pool and the pool will be automatically */
/* enlarged if required */

TInt RStdPool::Create(TInt a_iSize, TInt a_iNumItems, TBool a_bExtensible)
{
	TInt RetVal;

	ASSERTM((a_iSize >= (TInt) sizeof(CPoolNode)), "RStdPool::Create() => Node size is too small");

	/* Initialise the pool if it has not already been initialised */

	if (m_iSize == 0)
	{
		m_iSize = a_iSize;
		m_iNumItems = a_iNumItems;
		m_bExtensible = a_bExtensible;

		/* Now allocate the initial set of a_iNumItems nodes */

		RetVal = ExtendPool();
	}
	else
	{
		RetVal = KErrInUse;

		Utils::Info("RStdPool::Create() => Pool is already in use");
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
/* have been freed and is now invalid.  Once this function has returned, you can call */
/* RStdPool::Create() to reuse the pool if desired */

void RStdPool::Close(TBool a_bFreeNodes)
{
	CBufferNode *Buffer;
	CPoolNode *Node;

	/* If required, manually iterate through the list and free each and every */
	/* node */

	if (a_bFreeNodes)
	{
		while ((Node = m_oNodes.RemHead()) != NULL) { }
	}

	/* Otherwise just hard reset the list back to its original state */

	else
	{
		m_oNodes.Reset();
	}

	/* Free the buffers used by the pool for its nodes */

	while ((Buffer = m_oBuffers.RemHead()) != NULL)
	{
		delete [] (char *) Buffer;
	}

	/* And reset the other variables back to their defaults */

	m_bExtensible = EFalse;
	m_iNumItems = m_iSize = 0;
}

/* Written: Sunday 03-Jun-2012 11:20 pm, On train to Munich Deutsches Museum */
/* @return	Ptr to the new node if successful, else NULL */
/* Allocates a node from the list of available nodes.  This is a very fast operation */
/* as it simply returns the first node on the list of available nodes.  If there are */
/* no nodes on the list and the pool is extensible, an attempt will be made to */
/* allocate further memory for nodes via the heap.  If no memory could be allocated */
/* or if the pool was not extensible, NULL will be returned.  To ensure fast allocation, */
/* the contents of the node are *not* cleared to zero */

void *RStdPool::GetNode()
{
	CPoolNode *RetVal;

	/* Get a ptr to the first available node */

	RetVal = m_oNodes.GetHead();

	/* If there are none left and the pool is extendable, then try to allocate */
	/* another block of nodes and then get a ptr to the first available new node */

	if ((!(RetVal)) && (m_bExtensible))
	{
		if (ExtendPool() == KErrNone)
		{
			RetVal = m_oNodes.GetHead();
		}
	}

	/* If we obtained a node successfully then remove it from the list of available */
	/* nodes as it is no longer available */

	if (RetVal)
	{
		m_oNodes.Remove(RetVal);
	}

	return(RetVal);
}

/* Written: Wednesday 24-Oct-2012 6:37 am, Vis à Vis Hotel, Lindau */
/* @param	a_poNode	Ptr to the node to be released back to the pool */
/* Releases a node previously allocated using RPool::GetNode() back into */
/* the pool, thus making it available for allocation again */

void RStdPool::ReleaseNode(void *a_poNode)
{
	ASSERTM((a_poNode != NULL), "RStdPool::ReleaseNode() => Valid node must be passed in");

	m_oNodes.AddTail((CPoolNode *) a_poNode);
}

/* Written: Tuesday 31-Jul-2012 9:42 am, Starbucks Nürnberg */
/* @return	KErrNone if successful, else KErrNoMemory */
/* Extends the pool by allocating a further m_iNumItems of nodes in a single */
/* allocation.  The block is added to the list of blocks and the nodes are */
/* added to the list of nodes */

TInt RStdPool::ExtendPool()
{
	char *Buffer;
	TInt Index, RetVal;
	CBufferNode *BufferNode;
	CPoolNode *PoolNode;

	/* Allocate a buffer large enough to hold the requested number of nodes */
	/* of the requested size, plus one more which can be used to keep track */
	/* of the block in the list of blocks */

	if ((Buffer = new char[m_iSize * (m_iNumItems + 1)]) != NULL)
	{
		RetVal = KErrNone;

		/* Make the first node into a CBufferNode and add it to the list of buffers */

		BufferNode = (CBufferNode *) Buffer;
		m_oBuffers.AddTail(BufferNode);
		Buffer += m_iSize;

		/* For the remaining nodes in the buffer transform each one into a CPoolNode, */
		/* and add it to the list of nodes */

		for (Index = 0; Index < m_iNumItems; ++Index)
		{
			PoolNode = (CPoolNode *) Buffer;
			m_oNodes.AddTail(PoolNode);
			Buffer += m_iSize;
		}
	}
	else
	{
		RetVal = KErrNoMemory;

		Utils::Info("RStdPool::ExtendPool() => Out of memory");
	}

	return(RetVal);
}
