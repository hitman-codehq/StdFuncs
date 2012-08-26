
#ifndef STDPOOL_H
#define STDPOOL_H

#include "StdList.h"

/* Node used to keep a track of the backing buffers used by the pool. */
/* Memory for this is "borrowed" from the first list node, so this node */
/* must be kept the same size as the CPoolNode class */

class CBufferNode
{
public:

	StdListNode<CBufferNode>	m_oStdListNode;		/* Standard list node */
};

// TODO: CAW - Properly comment this and the .cpp file
class CPoolNode
{
public:

	StdListNode<CPoolNode>	m_oStdListNode;		/* Standard list node */
};

class RStdPool
{
	TBool					m_bExtensible;		/* ETrue if pool can be extended beyond its initial size */
	TInt					m_iNumItems;		/* # of items in each block of items */
	TInt					m_iSize;			/* Size of the nodes in the pool */
	StdList<CBufferNode>	m_oBuffers;			/* List of buffers used by the pool */
	StdList<CPoolNode>		m_oNodes;			/* List of free nodes in the pool */

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
};

#endif /* ! STDPOOL_H */
