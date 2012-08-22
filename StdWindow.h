
#ifndef STDWINDOW_H
#define STDWINDOW_H

#include "StdGadgets.h"

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declaration to reduce the # of includes required */

class RApplication;

/* Mouse events that can be reported to the client */

enum TStdMouseEvent
{
	EStdMouseDown,			/* Mouse button pressed */
	EStdMouseUp,			/* Mouse button released */
	EStdMouseMove,			/* Mouse moved */
	EStdMouseDoubleClick	/* Mouse button double clicked */
};

/* This is the base class for all platform specific windows and dialog boxes */

class CWindow
{
	/* RApplication and CStdGadgetLayout classes need to be able to access this class's internals */
	/* in order to link windows into the window list and manage the gadgets' positions etc. */

	friend class CStdGadgetLayout;
	friend class RApplication;

private:

	TBool				m_bFillBackground;			/* ETrue to fill background when drawing */
	CWindow				*m_poNext;					/* Ptr to next window in list */
	static CWindow		*m_poRootWindow;			/* Ptr to root window on which all other windows open */

#ifdef __amigaos4__

	struct Hook			m_oIDCMPHook;				/* IDCMP hook for watching gadgets such as sliders */
	Object				*m_poRootGadget;			/* layout.gadget containing the window's gadgets */

#endif /* __amigaos4__ */

protected:

	static TBool		m_bCtrlPressed;				/* ETrue if ctrl is currently pressed */
	TBool				m_bOpen;					/* ETrue if window is open */
	TInt				m_iInnerWidth;				/* Width of window, minus left and right borders */
	TInt				m_iInnerHeight;				/* Height of window, minus top and bottom borders */
	StdList<CStdGadgetLayout>	m_oGadgets;			/* List of layout gadgets manually added to the window */
	RApplication		*m_poApplication;			/* Ptr to application that owns this window */

public:

#ifdef __amigaos4__

	Object				*m_poWindowObj;				/* Ptr to underlying Reaction window */
	struct Window		*m_poWindow;				/* Ptr to underlying Intuition window */

#elif defined(__linux__)

	int					*m_poWindow;				// TODO: CAW - Placeholder to make compiling easier

#else /* ! __linux__ */

	HWND				m_poWindow;					/* Ptr to underlying Windows window */
	HDC					m_poDC;						/* Device context and paint structure into which to */
	PAINTSTRUCT			m_oPaintStruct;				/* render;  valid only during calls to CWindow::Draw() */

#endif /* ! __linux__ */

public:

	CWindow(RApplication *a_poApplication)
	{
		m_bFillBackground = ETrue;
		m_poApplication  = a_poApplication;
	}

	~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccScreenName);

	virtual void Close();

	void Activate();

	void Attach(CStdGadgetLayout *a_poLayoutGagdet);

	RApplication *Application()
	{
		return(m_poApplication);
	}

	void ClearBackground(TInt a_iY, TInt a_iHeight, TInt a_iX, TInt a_iWidth);

	void DrawNow();

	void DrawNow(TInt a_iTop, TInt a_iBottom, TInt a_iWidth = -1);

	void EnableFillBackground(TBool a_bFillBackground)
	{
		m_bFillBackground = a_bFillBackground;
	}

	void EnableMenuItem(TInt a_iItemID, TBool a_bEnable);

	void CheckMenuItem(TInt a_iItemID, TBool a_bEnable);

	TBool CtrlPressed()
	{
		return(m_bCtrlPressed);
	}

	ULONG FindMenuMapping(struct SStdMenuMapping *a_poMenuMappings, TInt a_iNumMenuMappings, TInt a_iItemID);

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

#elif defined(WIN32)

	static LRESULT CALLBACK WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam);

#endif /* WIN32 */

	void MakeRootWindow()
	{
		m_poRootWindow = this;
	}

	// TODO: CAW - Naming - Use Get or not?
	static CWindow *GetRootWindow()
	{
		return(m_poRootWindow);
	}

#ifdef __amigaos4__

	static struct Screen *GetRootWindowScreen();

#endif /* __amigaos4__ */

	void Remove(CStdGadgetLayout *a_poLayoutGadget);

	void RethinkLayout();

	/* Functions can be implemented by client software */

	virtual void Activated(TBool /*a_bActivated*/) { }

	virtual void Draw(TInt /*a_iTop*/, TInt /*a_iBottom*/) { }

	virtual void HandleCommand(TInt /*a_iCommand*/) { }

	virtual void HandlePointerEvent(TInt /*a_iX*/, TInt /*a_iY*/, TStdMouseEvent /*a_eMouseEvent*/) { }

	virtual void HandleWheelEvent(TInt /*a_iDelta*/) { }

	virtual void OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_bKeyDown*/) { }

	virtual void Resize(TInt /*a_iOldInnerWidth*/, TInt /*a_iOldInnerHeight*/) { }

protected:

	CWindow() { }

	void CompleteOpen();

private:

	void InternalResize(TInt a_iInnerWidth, TInt a_iInnerHeight);
};

#endif /* ! STDWINDOW_H */
