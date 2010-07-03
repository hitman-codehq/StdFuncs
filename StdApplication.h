
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
	EStdMenuItem						/* An item on a menu */
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

	CWindow			*m_poWindow;		/* Ptr to window used by the application */

#ifdef __amigaos4__

	TBool			m_bDone;			/* Set to ETrue when it's time to exit the main loop */
	struct Menu		*m_poMenus;			/* Ptr to main menus displayed at top of the screen */
	TBool			m_bMenuStripSet;	/* ETrue if menus have been added to the main window */

#else /* !__amigaos4__ */

	HACCEL			m_poAccelerators;	/* Ptr to application's accelerator table, if any */

#endif /* ! __amigaos4__ */

	const struct SStdMenuItem *m_pcoMenuItems;	/* Ptr to a list of structures describing the application's menus */

private:

	TBool CreateMenus(const struct SStdMenuItem *a_pcoMenuItems);

public:

	RApplication();

	TInt Open(const struct SStdMenuItem *a_pcoMenuItems);

	int Main();

	void AddWindow(CWindow *a_poWindow);

	void Exit();
};

#endif /* ! STDAPPLICATION_H */
