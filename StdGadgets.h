
#ifndef STDGADGETS_H
#define STDGADGETS_H

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declaration to reduce the # of includes required */

class CWindow;
class MStdGadgetSliderObserver;

/* The abstract base class used for all gadgets */

class CStdGadget
{
	/* CWindow class needs to be able to access this class's internals in order to */
	/* link windows into its gadget list */

	friend class CWindow;

protected:

	TInt					m_iGadgetID;		/* Unique ID of the gadget */
	CWindow					*m_poParentWindow;	/* Ptr to window that owns this gadget */

#ifdef __amigaos4__

	Object					*m_poGadget;		/* Ptr to underlying BOOPSI gadget */

#else /* ! __amigaos4__ */

	HWND					m_poGadget;			/* Ptr to the underlying Windows control */

#endif /* ! __amigaos4__ */

public:

	StdListNode<CStdGadget>	m_oStdListNode;     /* Standard list node */

	TInt GadgetID()
	{
		return(m_iGadgetID);
	}

	virtual void Updated(ULONG /*a_ulData*/ = 0) { }
};

/* A class representing a slider or proportional gadget */

class CStdGadgetSlider : public CStdGadget
{
private:

	MStdGadgetSliderObserver *m_poClient;		/* Ptr to client to notify when gadget changes */

public:

	TInt Create(CWindow *a_poParentWindow, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID);

	void SetRange(TInt a_iPageSize, TInt a_iMaxRange);

	/* From CStdGadget */

	void Updated(ULONG a_ulData);
};

/* Mixin class for the slider or proportional gadget to be able to notify its client */
/* when it has been updated */

class MStdGadgetSliderObserver
{
public:

	virtual void SliderUpdated(CStdGadgetSlider *a_poGadget, TInt a_iValue) = 0;
};

#endif /* ! STDGADGETS_H */

