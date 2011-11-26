
#ifndef STDAPPLICATION_H
#define STDAPPLICATION_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* Enumeration defining the types of menu items that can be created. These are mapped */
/* onto the types used by Amiga OS for easy use on that operating system */

enum TStdMenuItemType
{
	EStdMenuEnd,
	EStdMenuTitle,						/* A menu */
	EStdMenuItem,						/* An item on a menu */
	EStdMenuCheck						/* A checked menu item */
};

/* Structure defining a single menu item to be dynamically created */

struct SStdMenuItem
{
	TStdMenuItemType	m_eType;		/* Type of menu or menu item to be created */
	const char			*m_pccLabel;	/* Menu item's label to be displayed */
	const char			*m_pccHotKey;	/* Shortcut key to be displayed, if non NULL */
	TInt				m_iCommand;		/* Command to send when menu is selected */
};

class RApplication
{
private:

	CWindow			*m_poWindows;		/* Ptr to window used by the application */

#ifdef __amigaos4__

	static TBool	m_bCtrlPressed;		/* ETrue if ctrl is currently pressed */

	ULONG			m_ulWindowSignals;	/* Signals representing all currently open windows */
	TBool			m_bDone;			/* Set to ETrue when it's time to exit the main loop */
	struct Menu		*m_poMenus;			/* Ptr to main menus displayed at top of the screen */
	TBool			m_bMenuStripSet;	/* ETrue if menus have been added to the main window */
	TInt			m_iLastX;			/* X and Y positions of the mouse the last time the */
	TInt			m_iLastY;			/* LMB was clicked or released */
	ULONG			m_ulMainSeconds;	/* Tick count in seconds and microseconds of when */
	ULONG			m_ulMainMicros;		/* the LMB was last clicked */

#else /* !__amigaos4__ */

	HACCEL			m_poAccelerators;	/* Ptr to application's accelerator table, if any */
	HWND			m_poCurrentDialog;	/* Ptr to window handle of currently active dialog, if any */

#endif /* ! __amigaos4__ */

	const struct SStdMenuItem *m_pcoMenuItems;	/* Ptr to a list of structures describing the application's menus */

private:

	TBool CreateMenus(const struct SStdMenuItem *a_pcoMenuItems);

public:

	RApplication();

	TInt Open(const struct SStdMenuItem *a_pcoMenuItems);

	int Main();

	void Close();

	void AddWindow(CWindow *a_poWindow);

	void RemoveWindow(CWindow *a_poWindow);

#ifdef WIN32

	void SetCurrentDialog(HWND a_poDialog);

#endif /* WIN32 */

	void Exit();
};

#endif /* ! STDAPPLICATION_H */
