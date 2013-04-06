
#ifndef STDWINDOW_H
#define STDWINDOW_H

#include "StdGadgets.h"

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declarations to reduce the # of includes required */

class CQtAction;
class CQtCentralWidget;
class CQtWindow;
class RApplication;

/* Mouse events that can be reported to the client */

enum TStdMouseEvent
{
	EStdMouseDown,			/* Mouse button pressed */
	EStdMouseUp,			/* Mouse button released */
	EStdMouseMove,			/* Mouse moved */
	EStdMouseDoubleClick	/* Mouse button double clicked */
};

/* The ptr to the root window lives as a global in Utils.cpp rather than as a static */
/* in CWindow, to prevent non GUI programs pulling the framework in */

extern CWindow *g_poRootWindow;

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

#ifdef __amigaos4__

	struct Hook			m_oIDCMPHook;				/* IDCMP hook for watching gadgets such as sliders */
	Object				*m_poRootGadget;			/* layout.gadget containing the window's gadgets */

#elif defined(WIN32)

	ATOM				m_poWindowClass;			/* Window's class, returned from RegisterClass() */

#endif /* WIN32 */

protected:

	static TBool		m_bAltPressed;				/* ETrue if alt is currently pressed */
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

#elif defined(QT_GUI_LIB)

	CQtWindow			*m_poWindow;				/* Ptr to underlying Qt window */
	CQtCentralWidget	*m_poCentralWidget;			/* Ptr to underlying Qt central widget */

#elif defined(WIN32)

	HACCEL				m_poAccelerators;			/* Ptr to application's accelerator table, if any */
	HWND				m_poWindow;					/* Ptr to underlying Windows window */
	HDC					m_poDC;						/* Device context and paint structure into which to */
	PAINTSTRUCT			m_oPaintStruct;				/* render;  valid only during calls to CWindow::Draw() */
	static CWindow		*m_poActiveDialog;			/* Ptr to currently active dialog, if any */

#endif /* WIN32 */

private:

	TBool CreateMenus();

public:

	CWindow(RApplication *a_poApplication)
	{
		m_bFillBackground = ETrue;
		m_poApplication  = a_poApplication;
	}

	virtual ~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccScreenName, TBool a_bResizeable);

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

	TBool AltPressed();

	TBool CtrlPressed();

	/* This function is *internal* and must not be used.  Unfortunately making it private */
	/* would require exposing too much of the internals of the CWindow class so it is */
	/* public with a warning instead.  Sometimes C++ can be a right nuisance! */

	void InternalResize(TInt a_iInnerWidth, TInt a_iInnerHeight);

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

	ULONG FindMenuMapping(struct SStdMenuMapping *a_poMenuMappings, TInt a_iNumMenuMappings, TInt a_iItemID);

	ULONG GetSignal();

#elif defined(QT_GUI_LIB)

	CQtAction *FindMenuItem(TInt a_iItemID);

#elif defined(WIN32)

	static LRESULT CALLBACK WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam);

#endif /* WIN32 */

	void MakeRootWindow()
	{
		g_poRootWindow = this;
	}

	static CWindow *RootWindow()
	{
		return(g_poRootWindow);
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

	virtual TBool OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_bKeyDown*/) { return(EFalse); }

	/* This function is called by The Framework whenever the window is resized.  The */
	/* behaviour can differ slightly between platforms in that some platforms will call */
	/* it before the window is displayed and some platforms will call it only after the */
	/* the window is displayed */

	virtual void Resize(TInt /*a_iOldInnerWidth*/, TInt /*a_iOldInnerHeight*/) { }

protected:

	CWindow() { }

	void CompleteOpen();

	friend class CQtWindow;
};

#endif /* ! STDWINDOW_H */
