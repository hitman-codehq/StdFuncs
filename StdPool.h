
#ifndef STDPOOL_H
#define STDPOOL_H

#include "StdList.h"

/* Node used to keep a track of the backing buffers used by the pool. */
/* Memory for this is "borrowed" from the first list node, so this node */
/* must be kept the same size as the CPoolNode class */

class CBufferNode
{
public:

	StdListNode<CBufferNode>	m_oStdListNode;		/**< Standard list node */
};

/* Class used for allocating small fixed size objects from an area of preallocated */
/* memory.  This class allows you to very quickly allocate and free structures */
/* without the overhead of calling into a heap allocator.  It is also more memory */
/* efficient than a heap allocator as the memory used for the nodes is reused for */
/* their metadata when they are not allocated (ie. they are in the pool) */

class CPoolNode
{
public:

	StdListNode<CPoolNode>	m_oStdListNode;			/**< Standard list node */
};

class RStdPool
{
private:

	TBool					m_bExtensible;			/**< ETrue if pool can be extended beyond its initial size */
	TInt					m_iNumItems;			/**< Number of items in each block of items */
	TInt					m_iSize;				/**< Size of the nodes in the pool */
	StdList<CBufferNode>	m_oBuffers;				/**< List of buffers used by the pool */
	StdList<CPoolNode>		m_oNodes;				/**< List of free nodes in the pool */

public:

	RStdPool()
	{
		m_bExtensible = EFalse;
		m_iNumItems = m_iSize = 0;
	}

	~RStdPool()
	{
		Close(EFalse);
	}

	TInt Create(TInt a_iSize, TInt a_iNumItems, TBool a_bExtensible);

	void Close(TBool a_bFreeNodes);

	TInt Count()
	{
		return(m_oNodes.Count());
	}

	TInt ExtendPool();

	void *GetNode();

	void ReleaseNode(void *a_poNode);
};

#endif /* ! STDPOOL_H */
