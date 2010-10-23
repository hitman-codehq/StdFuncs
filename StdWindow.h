
#ifndef STDWINDOW_H
#define STDWINDOW_H

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declaration to reduce the # of includes required */

class RApplication;

/* This is the base class for all platform specific windows and dialog boxes */

class CWindow
{
	/* RApplication class needs to be able to access this class's internals in order to */
	/* link windows into its window list */

	friend class RApplication;

private:

	CWindow			*m_poNext;			/* Ptr to next window in list */

protected:

	TBool			m_bOpen;			/* ETrue if window is open */
	TInt			m_iInnerWidth;		/* Width of window, minus left and right borders */
	TInt			m_iInnerHeight;		/* Height of window, minus top and bottom borders */
	RApplication	*m_poApplication;	/* Ptr to application that owns this window */

public:

#ifdef __amigaos4__

	Object			*m_poWindowObj;	/* Ptr to underlying Reaction window */
	struct Window	*m_poWindow;	/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

	HWND			m_poWindow;		/* Ptr to underlying Windows window */
	HDC				m_poDC;			/* Device context and paint structure into which to */
	PAINTSTRUCT		m_oPaintStruct;	/* render;  valid only during calls to CWindow::Draw() */

#endif /* ! __amigaos4__ */

public:

	CWindow(RApplication *a_poApplication)
	{
		m_poApplication  = a_poApplication;
	}

	~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccPubScreenName);

	void Close();

	void Activate();

	void DrawNow();

	TInt InnerWidth()
	{
		return(m_iInnerWidth);
	}

	TInt InnerHeight()
	{
		return(m_iInnerHeight);
	}

#ifdef __amigaos4__

	ULONG GetSignal();

	struct Window *GetWindow();

#endif /* __amigaos4__ */

	/* Functions can be implemented by client software */

	virtual void Draw() { }

	virtual void HandleCommand(TInt /*a_iCommand*/) { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_iKeyDown*/) { }

protected:

	CWindow() { }

	void CompleteOpen();
};

#endif /* ! STDWINDOW_H */
