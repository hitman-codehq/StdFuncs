
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

void RStdPool::Close()
{
	// TODO: CAW - Can we just reset the list?  Do we need to test that nodes
	//             have all been released or is this ok?
	CPoolNode *Node;

	while ((Node = m_oNodes.RemHead()) != NULL)
	{
	}

	delete [] m_pcBuffer;
	m_pcBuffer = NULL;

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
