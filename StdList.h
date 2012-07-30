
#ifndef STDLIST_H
#define STDLIST_H

/* Template class for the node that goes onto the list and owns a C++ class instance */

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
		m_oHead.m_poPrev = m_oTail.m_poNext = NULL;
		m_oHead.m_poNext = &m_oTail;
		m_oTail.m_poPrev = &m_oHead;
		m_iCount = 0;
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

	TInt Count()
	{
		return(m_iCount);
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

	// TODO: CAW - This will mess up the Count() function + write a test case
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

		if (m_oHead.m_poNext != &m_oTail)
		{
			RetVal = m_oHead.m_poNext->m_poThis;

			m_oHead.m_poNext = m_oHead.m_poNext->m_poNext;
			m_oHead.m_poNext->m_poPrev = &m_oHead;

			--m_iCount;
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

		return(RetVal);
	}
};

#endif /* ! STDLIST_H */
