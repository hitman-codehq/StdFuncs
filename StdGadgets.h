
#ifndef STDGADGETS_H
#define STDGADGETS_H

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* The abstract base class used for all gadgets */

class CStdGadget
{
	/* CWindow class needs to be able to access this class's internals in order to */
	/* link windows into its gadget list */

	friend class CWindow;

protected:

	CWindow					*m_poParentWindow;	/* Ptr to window that owns this gadget */

#ifdef __amigaos4__

	Object					*m_poGadget;		/* Ptr to underlying BOOPSI gadget */

#endif /* __amigaos4__ */

public:

	StdListNode<CStdGadget>	m_oStdListNode;     /* Standard list node */
};

/* A class representing a scroller or proportional gadget */

class CStdGadgetSlider : public CStdGadget
{
public:

	TInt Create(CWindow *a_poParentWindow);
};

#endif /* ! STDGADGETS_H */

