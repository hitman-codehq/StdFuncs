
#ifndef STDWINDOW_H
#define STDWINDOW_H

#include "StdGadgets.h"

#ifdef __amigaos4__

#include <intuition/classes.h>

#endif /* __amigaos4__ */

/* Forward declarations to reduce the # of includes required */

class CAmiMenus;
class CQtAction;
class CQtCentralWidget;
class CQtWindow;
class RApplication;

/* Enumeration defining the types of menu items that can be created. These are mapped */
/* onto the types used by Amiga OS for easy use on that operating system */

enum TStdMenuItemType
{
	EStdMenuEnd,							/**< Marker to indicate the end of the menu list */
	EStdMenuTitle,							/**< A menu - must be 1 to match NM_TITLE */
	EStdMenuItem,							/**< An item on a menu  - must be 2 to match NM_ITEM */
	EStdMenuCheck,							/**< A checked menu item */
	EStdMenuSeparator,						/**< A separator bar */
	EStdMenuSubMenu,						/**< A popout submenu */
	EStdMenuSubMenuEnd						/**< Special value that ends a submenu definition */
};

/* Each instance of this structure represents an Amiga OS menu mapping */

struct SStdMenuMapping
{
	TInt	m_iID;							/**< Integer ID of the menu */
	ULONG	m_ulFullMenuNum;				/**< FULLMENUNUM of the menu, useable for OffMenu() etc */
};

/* Structure defining a single menu item to be dynamically created */

struct SStdMenuItem
{
	TStdMenuItemType	m_eType;			/**< Type of menu or menu item to be created */
	const char			*m_pccLabel;		/**< Menu item's label to be displayed */
	const char			*m_pccHotKey;		/**< Shortcut key to be displayed, if non NULL */
	TInt				m_iHotKeyModifier;	/**< Modifier for hotkey, such as STD_KEY_CONTROL */
	TInt				m_iCommand;			/**< Command to send when menu is selected */
};

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
private:

	TInt				m_iCursorX;					/**< Current X position of the cursor */
	TInt				m_iCursorY;					/**< Current Y position of the cursor */
	TInt				m_iCursorHeight;			/**< Height of the cursor to be displayed */
	TBool				m_bFillBackground;			/**< ETrue to fill background when drawing */
	CWindow				*m_poNext;					/**< Ptr to next window in list */

#ifdef __amigaos4__

	struct Hook			m_oIDCMPHook;				/**< IDCMP hook for watching gadgets such as sliders */
	Object				*m_poRootGadget;			/**< layout.gadget containing the window's gadgets */

#elif defined(WIN32)

	ATOM				m_poWindowClass;			/**< Window's class, returned from RegisterClass() */
	HMENU				m_poMenu;					/**< Handle to window's menu bar */

#endif /* WIN32 */

protected:

	static TBool		m_bAltPressed;				/**< ETrue if alt is currently pressed */
	static TBool		m_bCtrlPressed;				/**< ETrue if ctrl is currently pressed */
	static TBool		m_bShiftPressed;			/**< ETrue if shift is currently pressed */

	TBool				m_bOpen;					/**< ETrue if window is open */
	TInt				m_iInnerWidth;				/**< Width of window, minus left and right borders */
	TInt				m_iInnerHeight;				/**< Height of window, minus top and bottom borders */
	StdList<CStdGadgetLayout>	m_oGadgets;			/**< List of layout gadgets manually added to the window */
	RApplication		*m_poApplication;			/**< Ptr to application that owns this window */

public:

#ifdef __amigaos4__

	CAmiMenus			*m_poAmiMenus;				/**< Ptr to instance of Amiga menu helper class */
	Object				*m_poWindowObj;				/**< Ptr to underlying Reaction window */
	struct Window		*m_poWindow;				/**< Ptr to underlying Intuition window */

#elif defined(QT_GUI_LIB)

	CQtWindow			*m_poWindow;				/**< Ptr to underlying Qt window */
	CQtCentralWidget	*m_poCentralWidget;			/**< Ptr to underlying Qt central widget */

#elif defined(WIN32)

	HACCEL				m_poAccelerators;			/**< Ptr to application's accelerator table, if any */
	HWND				m_poWindow;					/**< Ptr to underlying Windows window */
	HDC					m_poDC;						/**< Device context and paint structure into which to */
	PAINTSTRUCT			m_oPaintStruct;				/**< render;  valid only during calls to CWindow::Draw() */
	static CWindow		*m_poActiveDialog;			/**< Ptr to currently active dialog, if any */

#endif /* WIN32 */

private:

	TInt AddMenuItem(const struct SStdMenuItem *a_pcoMenuItem, void *a_pvDropdownMenu);

	TBool CreateMenus();

#if defined(WIN32) && !defined(QT_GUI_LIB)

	TInt AddAccelerator(const struct SStdMenuItem *a_pcoMenuItem);

	void InitialiseAccelerator(ACCEL *a_poAccelerator, const struct SStdMenuItem *a_pcoMenuItem);

	const char *InitialiseMenuLabel(const struct SStdMenuItem *a_pcoMenuItem);

	TInt RemoveAccelerator(TInt a_iCommand);

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

#ifdef __amigaos4__

	static void IDCMPFunction(struct Hook *a_poHook, Object *a_poObject, struct IntuiMessage *a_poIntuiMessage);

	static struct Screen *GetRootWindowScreen();

	ULONG GetSignal();

	struct Menu *Menus();

#elif defined(QT_GUI_LIB)

	CQtAction *FindMenuItem(TInt a_iItemID);

#elif defined(WIN32)

	static LRESULT CALLBACK WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam);

#endif /* WIN32 */

protected:

	CWindow() { }

	void CompleteOpen();

public:

	CWindow(RApplication *a_poApplication)
	{
		m_iCursorHeight = 16;
		m_bFillBackground = ETrue;
		m_poApplication = a_poApplication;
	}

	virtual ~CWindow();

	TInt Open(const char *a_pccTitle, const char *a_pccScreenName, TBool a_bResizeable);

	virtual void Close();

	void Activate();

	TInt AddMenuItem(TStdMenuItemType a_eMenuItemType, const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iSubOrdinal, TInt a_iCommand);

	void Attach(CStdGadgetLayout *a_poLayoutGagdet);

	RApplication *Application()
	{
		return(m_poApplication);
	}

	void BringToFront();

	void ClearBackground(TInt a_iY, TInt a_iHeight, TInt a_iX, TInt a_iWidth);

	void DrawNow();

	void DrawNow(TInt a_iTop, TInt a_iBottom, TInt a_iWidth = -1);

	void EnableFillBackground(TBool a_bFillBackground)
	{
		m_bFillBackground = a_bFillBackground;
	}

	void EnableMenuItem(TInt a_iItemID, TBool a_bEnable);

	void CheckMenuItem(TInt a_iItemID, TBool a_bEnable);

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

	void MakeRootWindow()
	{
		g_poRootWindow = this;
	}

	TBool MenuItemChecked(TInt a_iItemID);

	static CWindow *RootWindow()
	{
		return(g_poRootWindow);
	}

	void Remove(CStdGadgetLayout *a_poLayoutGadget);

	void RemoveMenuItem(TInt a_iOrdinal, TInt a_iCommand);

	void RethinkLayout();

	void SetCursorInfo(TInt a_iX, TInt a_iY, TInt a_iHeight);

	void UpdateMenuItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iCommand);

	/* Accessor functions */

	static TBool AltPressed();

	static TBool CtrlPressed();

	static TBool ShiftPressed();

	/* Functions that can be implemented by client software */

	virtual void Activated(TBool /*a_bActivated*/) { }

	virtual void Draw(TInt /*a_iTop*/, TInt /*a_iBottom*/) { }

	virtual void HandleCommand(TInt /*a_iCommand*/) { }

	virtual void HandlePointerEvent(TInt /*a_iX*/, TInt /*a_iY*/, TStdMouseEvent /*a_eMouseEvent*/) { }

	virtual void HandleWheelEvent(TInt /*a_iDelta*/) { }

	virtual TBool OfferKeyEvent(TInt /*a_iKey*/, TBool /*a_bKeyDown*/) { return(EFalse); }

	virtual void OfferRawKeyEvent(TInt /*a_iKey*/, TBool /*a_bKeyDown*/) { }

	/* This function is called by The Framework whenever the window is resized.  The */
	/* behaviour can differ slightly between platforms in that some platforms will call */
	/* it before the window is displayed and some platforms will call it only after the */
	/* the window is displayed */

	virtual void Resize(TInt /*a_iOldInnerWidth*/, TInt /*a_iOldInnerHeight*/) { }

	/* RApplication and CStdGadgetLayout classes need to be able to access this class's internals */
	/* in order to link windows into the window list and manage the gadgets' positions etc. */

	friend class CQtWindow;
	friend class CStdGadgetLayout;
	friend class RApplication;
	friend class RStdImage;
};

#endif /* ! STDWINDOW_H */
