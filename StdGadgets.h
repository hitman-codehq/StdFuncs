
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
	TInt					m_iWidth;			/* Width of the gadget in pixels */
	TInt					m_iHeight;			/* Height of the gadget in pixels */
	CWindow					*m_poParentWindow;	/* Ptr to window that owns this gadget */

#ifdef __amigaos4__

	Object					*m_poGadget;		/* Ptr to underlying BOOPSI gadget */

#else /* ! __amigaos4__ */

	HWND					m_poGadget;			/* Ptr to the underlying Windows control */

#endif /* ! __amigaos4__ */

public:

	CStdGadget()
	{
		m_iWidth = m_iHeight = 0;
	}

	StdListNode<CStdGadget>	m_oStdListNode;     /* Standard list node */

	TInt GadgetID()
	{
		return(m_iGadgetID);
	}

	virtual TInt Width()
	{
		return(m_iWidth);
	}

	TInt Height()
	{
		return(m_iHeight);
	}

	virtual void Updated(ULONG /*a_ulData*/ = 0) { }
};

/* A class representing a slider or proportional gadget */

class CStdGadgetSlider : public CStdGadget
{
private:

	MStdGadgetSliderObserver *m_poClient;		/* Ptr to client to notify when gadget changes */

private:

	CStdGadgetSlider() { }

public:

	static CStdGadgetSlider *New(CWindow *a_poParentWindow, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID);

	TInt Create(CWindow *a_poParentWindow, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID);

	void SetPosition(TInt a_iPosition);

	void SetRange(TInt a_iPageSize, TInt a_iMaxRange);

	/* From CStdGadget */

	void Updated(ULONG a_ulData);

	TInt Width();
};

/* A class representing a status bar gadget */

class CStdGadgetStatusBar : public CStdGadget
{
private:

	TInt		m_iNumParts;					/* # of parts within the gadget */

private:

	CStdGadgetStatusBar() { }

public:

	static CStdGadgetStatusBar *New(CWindow *a_poParentWindow, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID);

	TInt Create(CWindow *a_poParentWindow, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID);

	void SetText(TInt a_iPart, const char *a_pccText);
};

/* Mixin class for the slider or proportional gadget to be able to notify its client */
/* when it has been updated */

class MStdGadgetSliderObserver
{
public:

	virtual void SliderUpdated(CStdGadgetSlider *a_poGadget, TInt a_iPosition) = 0;
};

#endif /* ! STDGADGETS_H */

