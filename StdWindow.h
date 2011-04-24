
#ifndef STDWINDOW_H
#define STDWINDOW_H

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declaration to reduce the # of includes required */

class RApplication;
class CStdGadget;

/* Mouse events that can be reported to the client */

enum TStdMouseEvent
{
	EStdMouseDown,	/* Mouse button pressed */
	EStdMouseMove,	/* Mouse moved */
	EStdMouseUp,	/* Mouse button released */
};

/* This is the base class for all platform specific windows and dialog boxes */

class CWindow
{
	/* RApplication class needs to be able to access this class's internals in order to */
	/* link windows into its window list */

	friend class RApplication;

private:

	TBool				m_bFillBackground;	/* ETrue to fill background when drawing */
	CWindow				*m_poNext;			/* Ptr to next window in list */
	StdList<CStdGadget>	m_oGadgets;			/* List of gadgets manually added to the window */
	static CWindow		*m_poRootWindow;	/* Ptr to root window on which all other windows open */

#ifdef __amigaos4__

	struct Hook			m_oIDCMPHook;		/* IDCMP hook for watching gadgets such as sliders */
	Object				*m_poRootGadget;	/* layout.gadget containing the window's gadgets */

#else /* ! __amigaos4__ */

	static TBool		m_bCtrlPressed;		/* ETrue if ctrl is currently pressed */

#endif /* ! __amigaos4__ */

protected:

	TBool				m_bOpen;			/* ETrue if window is open */
	TInt				m_iInnerWidth;		/* Width of window, minus left and right borders */
	TInt				m_iInnerHeight;		/* Height of window, minus top and bottom borders */
	TInt				m_iFontHeight;		/* Height of a character in the current font, in pixels */
	TInt				m_iFontWidth;		/* Width of a character in the current font, in pixels */
	RApplication		*m_poApplication;	/* Ptr to application that owns this window */

public:

#ifdef __amigaos4__

	Object				*m_poWindowObj;		/* Ptr to underlying Reaction window */
	struct Window		*m_poWindow;		/* Ptr to underlying Intuition window */

#else /* ! __amigaos4__ */

	HWND				m_poWindow;			/* Ptr to underlying Windows window */
	HDC					m_poDC;				/* Device context and paint structure into which to */
	PAINTSTRUCT			m_oPaintStruct;		/* render;  valid only during calls to CWindow::Draw() */

#endif /* ! __amigaos4__ */

public:

	CWindow(RApplication *a_poApplication)
	{
		m_bFillBackground = ETrue;
		m_poApplication  = a_poApplication;
	}

	~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccPubScreenName);

	virtual void Close();

	void Activate();

	void Attach(CStdGadget *a_poGagdet);

	RApplication *Application()
	{
		return(m_poApplication);
	}

	void DrawNow();

	void DrawNow(TInt a_iTop, TInt a_iBottom);

	void EnableFillBackground(TBool a_bFillBackground)
	{
		m_bFillBackground = a_bFillBackground;
	}

	TInt InnerWidth()
	{
		return(m_iInnerWidth);
	}

	TInt InnerHeight()
	{
		return(m_iInnerHeight);
	}

#ifdef __amigaos4__

	static void IDCMPFunction(struct Hook *a_poHook, Object *a_poObject, struct IntuiMessage *a_poIntuiMessage);

	ULONG GetSignal();

#else /* ! __amigaos4__ */

	static LRESULT CALLBACK WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam);

#endif /* ! __amigaos4__ */

	void MakeRootWindow()
	{
		m_poRootWindow = this;
	}

	static CWindow *GetRootWindow()
	{
		return(m_poRootWindow);
	}

#ifdef __amigaos4__

	static struct Screen *GetRootWindowScreen();

#endif /* __amigaos4__ */

	/* Functions can be implemented by client software */

	virtual void Draw(TInt /*a_iTop*/, TInt /*a_iBottom*/) { }

	virtual void HandleCommand(TInt /*a_iCommand*/) { }

	virtual void HandlePointerEvent(TInt /*a_iX*/, TInt /*a_iY*/, TStdMouseEvent /*a_eMouseEvent*/) { }

	virtual void HandleWheelEvent(TInt /*a_iDelta*/) { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_iKeyDown*/) { }

protected:

	CWindow() { }

	void CompleteOpen();
};

#endif /* ! STDWINDOW_H */
