
#ifndef STDAPPLICATION_H
#define STDAPPLICATION_H

/* Forward declarations to reduce the # of includes required */

class CWindow;
class QApplication;

class RApplication
{
private:

	CWindow			*m_poWindows;		/* Ptr to window used by the application */

#ifdef __amigaos4__

	ULONG			m_ulWindowSignals;	/* Signals representing all currently open windows */
	TBool			m_bDone;			/* Set to ETrue when it's time to exit the main loop */
	TInt			m_iLastX;			/* X and Y positions of the mouse the last time the */
	TInt			m_iLastY;			/* LMB was clicked or released */
	ULONG			m_ulMainSeconds;	/* Tick count in seconds and microseconds of when */
	ULONG			m_ulMainMicros;		/* the LMB was last clicked */

#elif defined(QT_GUI_LIB)

	QApplication	*m_poApplication;	/* Ptr to underlying Qt application */

#else /* ! QT_GUI_LIB */

	HWND			m_poCurrentDialog;	/* Ptr to window handle of currently active dialog, if any */

#endif /* ! QT_GUI_LIB */

	const struct SStdMenuItem *m_pcoMenuItems;	/**< Ptr to a list of structures describing the application's menus */

public:

	RApplication();

	TInt Open(const struct SStdMenuItem *a_pcoMenuItems);

	TInt Main();

	void Close();

	void AddWindow(CWindow *a_poWindow);

	void RemoveWindow(CWindow *a_poWindow);

	const struct SStdMenuItem *MenuItems()
	{
		return(m_pcoMenuItems);
	}

#ifdef QT_GUI_LIB

	QApplication *Application()
	{
		return(m_poApplication);
	}

#elif defined(WIN32)

	void SetCurrentDialog(HWND a_poDialog);

#endif /* WIN32 */

	void Exit();
};

#endif /* ! STDAPPLICATION_H */
