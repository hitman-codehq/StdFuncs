
#ifndef STDLIST_H
#define STDLIST_H

/* Template class for the node that goes onto the list and owns a C++ class instance. */
/* The use of macros for Mungwall means that we cannot implement a placement new */
/* operator.  To enable client code to create instances of the StdListNode simply */
/* by casting, it must not ever be given a constructor */

template <class T>
struct StdListNode
{
	StdListNode<T>	*m_poPrev;				/* Ptr to the previous node on the list */
	StdListNode<T>	*m_poNext;				/* Ptr to the next node on the list */
	T				*m_poThis;				/* Ptr to C++ class on the list */
};

/* Template class for a generic list */

template <class T>
class StdList
{
	StdListNode<T>			m_oHead;		/* Dummy node representing head of the list */
	StdListNode<T>			m_oTail;		/* Dummy node representing tail of the list */
	TInt					m_iCount;		/* # of items in the list */

public:

	StdList()
	{
		Reset();
	};

	void AddTail(T *a_poNode)
	{
		a_poNode->m_oStdListNode.m_poThis = a_poNode;
		a_poNode->m_oStdListNode.m_poPrev = m_oTail.m_poPrev;
		a_poNode->m_oStdListNode.m_poNext = &m_oTail;

		m_oTail.m_poPrev->m_poNext = &a_poNode->m_oStdListNode;
		m_oTail.m_poPrev = &a_poNode->m_oStdListNode;

		++m_iCount;
	}

	TInt Count() const
	{
		return(m_iCount);
	}

	/* Written: Friday 10-Aug-2012 6:53 am, Code HQ Ehinger Tor */

	void Insert(T *a_poNode, T *a_poAfter)
	{
		a_poNode->m_oStdListNode.m_poThis = a_poNode;
		a_poNode->m_oStdListNode.m_poPrev = &a_poAfter->m_oStdListNode;
		a_poNode->m_oStdListNode.m_poNext = a_poAfter->m_oStdListNode.m_poNext;

		a_poAfter->m_oStdListNode.m_poNext->m_poPrev = &a_poNode->m_oStdListNode;
		a_poAfter->m_oStdListNode.m_poNext = &a_poNode->m_oStdListNode;

		++m_iCount;
	}

	/* Written: Friday 10-Aug-2012 7:42 am, Code HQ Ehinger Tor */

	bool IsOnList(T *a_poNode) const
	{
		return(a_poNode->m_oStdListNode.m_poPrev != NULL);
	}

	T *GetHead() const
	{
		return((m_oHead.m_poNext != &m_oTail) ? m_oHead.m_poNext->m_poThis : NULL);
	}

	T *GetTail() const
	{
		return((m_oTail.m_poPrev != &m_oHead) ? m_oTail.m_poPrev->m_poThis : NULL);
	}

	T *GetPred(const T* a_poCurrent) const
	{
		return((a_poCurrent->m_oStdListNode.m_poPrev != &m_oHead) ? a_poCurrent->m_oStdListNode.m_poPrev->m_poThis : NULL);
	}

	T *GetSucc(const T* a_poCurrent) const
	{
		return((a_poCurrent->m_oStdListNode.m_poNext != &m_oTail) ? a_poCurrent->m_oStdListNode.m_poNext->m_poThis : NULL);
	}

	// TODO: CAW - This will mess up the Count() function + write a test case for this whole class
	void MoveList(StdList<T> *a_poList)
	{
		m_oTail.m_poPrev->m_poNext = a_poList->m_oHead.m_poNext;
		a_poList->m_oHead.m_poNext->m_poPrev = m_oTail.m_poPrev;

		a_poList->m_oTail.m_poPrev->m_poNext = &m_oTail;
		m_oTail.m_poPrev = a_poList->m_oTail.m_poPrev;
	}

	T *RemHead()
	{
		T *RetVal;

		// TODO: CAW - Assert the node is on a list, here and for others

		if (m_oHead.m_poNext != &m_oTail)
		{
			RetVal = m_oHead.m_poNext->m_poThis;

			m_oHead.m_poNext = m_oHead.m_poNext->m_poNext;
			m_oHead.m_poNext->m_poPrev = &m_oHead;

			--m_iCount;

			RetVal->m_oStdListNode.m_poPrev = RetVal->m_oStdListNode.m_poNext = NULL;
		}
		else
		{
			RetVal = NULL;
		}

		return(RetVal);
	}

	T *Remove(T *a_poNode)
	{
		T *RetVal;

		if (a_poNode->m_oStdListNode.m_poNext != &m_oTail)
		{
			RetVal = a_poNode->m_oStdListNode.m_poNext->m_poThis;
		}
		else
		{
			RetVal = NULL;
		}

		a_poNode->m_oStdListNode.m_poPrev->m_poNext = a_poNode->m_oStdListNode.m_poNext;
		a_poNode->m_oStdListNode.m_poNext->m_poPrev = a_poNode->m_oStdListNode.m_poPrev;

		--m_iCount;

		a_poNode->m_oStdListNode.m_poPrev = a_poNode->m_oStdListNode.m_poNext = NULL;

		return(RetVal);
	}

	void Reset()
	{
		m_oHead.m_poPrev = m_oTail.m_poNext = NULL;
		m_oHead.m_poNext = &m_oTail;
		m_oTail.m_poPrev = &m_oHead;
		m_iCount = 0;
	}
};

#endif /* ! STDLIST_H */
