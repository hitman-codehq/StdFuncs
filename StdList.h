
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
	/* Typedef for a function pointer that is used for comparing two nodes in a linked list. */
	/* The comparison function should return negative if the first node is less than the second */

	typedef int (*CompareFunction)(const T *a_poFirst, const T *a_poSecond);

private:

	StdListNode<T>			m_oHead;		/* Dummy node representing head of the list */
	StdListNode<T>			m_oTail;		/* Dummy node representing tail of the list */
	TInt					m_iCount;		/* # of items in the list */

public:

	StdList()
	{
		Reset();
	};

	void AddHead(T *a_poNode)
	{
		a_poNode->m_oStdListNode.m_poThis = a_poNode;
		a_poNode->m_oStdListNode.m_poPrev = &m_oHead;
		a_poNode->m_oStdListNode.m_poNext = m_oHead.m_poNext;

		m_oHead.m_poNext->m_poPrev = &a_poNode->m_oStdListNode;
		m_oHead.m_poNext = &a_poNode->m_oStdListNode;

		++m_iCount;
	}

	void AddTail(T *a_poNode)
	{
		a_poNode->m_oStdListNode.m_poThis = a_poNode;
		a_poNode->m_oStdListNode.m_poPrev = m_oTail.m_poPrev;
		a_poNode->m_oStdListNode.m_poNext = &m_oTail;

		m_oTail.m_poPrev->m_poNext = &a_poNode->m_oStdListNode;
		m_oTail.m_poPrev = &a_poNode->m_oStdListNode;

		++m_iCount;
	}

	/**
	 * Removes a single node from one list and appends it to another.
	 * This function will remove a node from one list and append it to the end of another list.
	 * It will take care of updating all links in both the source and destination lists, as well as
	 * the metadata regarding the number of nodes in the two lists.  The source list is passed in
	 * as a parameter and the destination is implicitly the current list.
	 *
	 * @date	Friday 27-Jun-2014 7:20 am, Code HQ Ehinger Tor
	 * @param	a_poSourceList	Pointer to the list from which to append the node
	 * @param	a_poSourceItem	Pointer to the list node to be appended to the destination list
	 */

	void AppendItem(StdList<T> *a_poSourceList, StdListNode<T> *a_poSourceItem)
	{
		StdListNode<T> *OldPrev, *OldNext;

		/* Remove the node from the source list */

		OldPrev = a_poSourceItem->m_poPrev;
		OldNext = a_poSourceItem->m_poNext;
		OldPrev->m_poNext = OldNext;
		OldNext->m_poPrev = OldPrev;
		--a_poSourceList->m_iCount;

		/* And insert it into the destination list */

		a_poSourceItem->m_poPrev = m_oTail.m_poPrev;
		a_poSourceItem->m_poNext = &m_oTail;
		m_oTail.m_poPrev->m_poNext = a_poSourceItem;
		m_oTail.m_poPrev = a_poSourceItem;
		++m_iCount;
	}

	/**
	 * Appends nodes from one list to the end of another list.
	 * This function will append all of the nodes in a given list to the end of this list.  After the append has
	 * been performed, the source list will be completely empty of nodes and can be reused without any further
	 * reinitialisation.  The count of nodes in this list will be the count before the append operation, plus
	 * however many nodes were in the source list.
	 *
	 * @param	a_poSourceList	Pointer to the list from which to append the nodes
	 */

	void AppendList(StdList<T> *a_poSourceList)
	{
		/* Only do anything if the source list actually contains any nodes, to avoid appending the non moveable */
		/* head and tail nodes */

		if (a_poSourceList->Count() > 0)
		{
			m_oTail.m_poPrev->m_poNext = a_poSourceList->m_oHead.m_poNext;
			a_poSourceList->m_oHead.m_poNext->m_poPrev = m_oTail.m_poPrev;

			a_poSourceList->m_oTail.m_poPrev->m_poNext = &m_oTail;
			m_oTail.m_poPrev = a_poSourceList->m_oTail.m_poPrev;

			m_iCount += a_poSourceList->m_iCount;

			/* The source list no longer owns the nodes but still points to them, so reset it back to */
			/* a completely empty state so that it can be reused */

			a_poSourceList->Reset();
		}
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

	/**
	 * Moves nodes from one list to another list.
	 * This function will move all of the nodes in a given list into this list.  After the move has been
	 * been performed, the source list will be completely empty of nodes and can be reused without any further
	 * reinitialisation.
	 *
	 * Any nodes present in this list when the operation is performed will be discarded and thrown away.  It
	 * is the callers responsibility to free these nodes if required and no check is performed to see if there
	 * are any nodes already present on this list when this function is called!
	 *
	 * @date	Thursday 12-Jun-2014 6:40 am, Code HQ Ehinger Tor
	 * @param	a_poSourceList	Pointer to the list from which to move the nodes
	 */

	void MoveList(StdList<T> *a_poSourceList)
	{
		/* Only do anything if the source list actually contains any nodes, to avoid moving the non moveable */
		/* head and tail nodes */

		if (a_poSourceList->Count() > 0)
		{
			m_oHead.m_poNext = a_poSourceList->m_oHead.m_poNext;
			m_oHead.m_poNext->m_poPrev = &m_oHead;

			m_oTail.m_poPrev = a_poSourceList->m_oTail.m_poPrev;
			m_oTail.m_poPrev->m_poNext = &m_oTail;

			m_iCount = a_poSourceList->m_iCount;

			/* The source list no longer owns the nodes but still points to them, so reset it back to */
			/* a completely empty state so that it can be reused */

			a_poSourceList->Reset();
		}
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

	/**
	 * Sorts a list into order using a user defined sort key.
	 * This function will use the so-called merge sort as developed by Simon Tatham of Putty fame.
	 * in order to sort a list in-place.  After it has been called, the list will contain the same
	 * number of nodes as before, but they will be sorted into order.  It uses a user supplied function
	 * to compare the nodes in order to determine which node is "less" and which node is "more".  That
	 * is, the node that is "less" will be sorted such that it is before the node that is "more".
	 *
	 * @date	Friday 27-Jun-2014 7:12 am, Code HQ Ehinger Tor
	 * @param	a_poCompare		Pointer to a user defined function that will compare two nodes
	 */

	void Sort(CompareFunction a_poCompare)
	{
		TBool Merged;
		TInt Index, InSize, NumMerges, PSize, QSize;
		StdList<T> List;
		StdListNode<T> *P, *Q, *E, *Tail;

		Merged = ETrue;
		InSize = 1;

		do
		{
			P = m_oHead.m_poNext;
			Tail = NULL;

			/* Count number of merges we do in this pass */

			NumMerges = 0;

			while (P != &m_oTail)
			{
				/* There exists a merge to be done */

				Merged = EFalse;
				NumMerges++;

				/* Step `InSize' places along from P */

				Q = P;
				PSize = 0;

				for (Index = 0; Index < InSize; ++Index)
				{
					++PSize;
					Q = Q->m_poNext;

					if (Q == &m_oTail)
					{
						break;
					}
				}

				/* If Q hasn't fallen off end, we have two lists to merge */

				QSize = InSize;

				/* Now we have two lists; merge them */

				while (PSize > 0 || (QSize > 0 && Q != &m_oTail))
				{
					/* Decide whether next element of merge comes from P or Q */

					if (PSize == 0)
					{
						/* P is empty; E must come from Q */

						E = Q; Q = Q->m_poNext; --QSize;
					}
					else if (QSize == 0 || Q == &m_oTail)
					{
						/* Q is empty; E must come from P */

						E = P; P = P->m_poNext; --PSize;
					}
					else if (a_poCompare(P->m_poThis, Q->m_poThis) <= 0)
					{
						/* First element of P is lower (or same); e must come from P */

						E = P; P = P->m_poNext; --PSize;
					}
					else
					{
						/* First element of Q is lower; E must come from Q */

						E = Q; Q = Q->m_poNext; --QSize;
					}

					/* Add the next element to the merged list */

					Merged = ETrue;
					List.AppendItem(this, E);
					Tail = E;
				}

				/* Now P has stepped `InSize' places along, and Q has too */

				P = Q;
			}

			/* The list has been partially sorted and moved into "List" to move it back into this list in preparation */
			/* for the next iteration */

			MoveList(&List);

			/* Repeat the sort, merging lists twice the size.  If we only did one merge this iteration then the */
			/* while statement below will trigger and we will exit the loop */

			InSize *= 2;
		}
		while (NumMerges > 1);
	}
};

#endif /* ! STDLIST_H */
