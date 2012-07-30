
#ifndef STDPOOL_H
#define STDPOOL_H

#include "StdList.h"

// TODO: CAW - Properly comment this and the .cpp file
class CPoolNode
{
public:

	StdListNode<CPoolNode>	m_oStdListNode;		/* Standard list node */
};

class RStdPool
{
	char					*m_pcBuffer;		/* Ptr to buffer used by the pool for nodes */
	TBool					m_bExtensible;		/* ETrue if pool can be extended beyond its initial size */
	TInt					m_iSize;			/* Size of the nodes in the pool */
	StdList<CPoolNode>		m_oNodes;			/* List of free nodes in the pool */

public:

	RStdPool()
	{
		m_pcBuffer = NULL;
		m_bExtensible = EFalse;
		m_iSize = 0;
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

	void *GetNode();
};

#endif /* ! STDPOOL_H */
