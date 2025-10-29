
#ifndef STDAPPLICATION_H
#define STDAPPLICATION_H

/** @file */

#include <vector>

/* Forward declarations to reduce the # of includes required */

class CWindow;
class QApplication;
class RStdExecuter;
class RAmiFileWatcher;

class RApplication
{
private:

	std::vector<RStdExecuter *>		m_oExecuters;	/**< List of executers used by the application */
	std::vector<RAmiFileWatcher *>	m_oWatchers;	/**< List of file watchers used by the application */
	CWindow			*m_poWindows;					/**< Pointer to window used by the application */

#ifdef __amigaos__

	ULONG			m_ulExecuterSignals;		/**< Signals representing all currently active executers */
	ULONG			m_ulWatcherSignals;			/**< Signals representing all currently active watchers */
	ULONG			m_ulWindowSignals;			/**< Signals representing all currently open windows */
	TBool			m_bDone;					/**< Set to ETrue when it's time to exit the main loop */
	TBool			m_bUseAltGr;				/**< ETrue if we are using a keyboard with an alt+gr key */
	TInt			m_iLastX;					/**< X and Y positions of the mouse the last time the */
	TInt			m_iLastY;					/**< LMB was clicked or released */
	ULONG			m_ulMainSeconds;			/**< Tick count in seconds and microseconds of when */
	ULONG			m_ulMainMicros;				/**< the LMB was last clicked */

#elif defined(QT_GUI_LIB)

	QApplication	*m_poApplication;			/**< Ptr to underlying Qt application */

#elif defined(WIN32)

	HWND			m_poCurrentDialog;			/**< Ptr to window handle of currently active dialog, if any */

#endif /* defined(WIN32) */

	const struct SStdMenuItem *m_pcoMenuItems;	/**< Ptr to a list of structures describing the application's menus */

public:

	RApplication();

	TInt open(const struct SStdMenuItem *a_pcoMenuItems);

	TInt Main();

	void close();

	void AddExecuter(RStdExecuter *a_poExecuter);

	void AddWatcher(RAmiFileWatcher *a_poWatcher);

	void AddWindow(CWindow *a_poWindow);

	void RemoveExecuter(RStdExecuter *a_poExecuter);

	void RemoveWatcher(RAmiFileWatcher *a_poWatcher);

	void RemoveWindow(CWindow *a_poWindow);

	const struct SStdMenuItem *MenuItems()
	{
		return(m_pcoMenuItems);
	}

#ifdef QT_GUI_LIB

	QApplication *GetApplication()
	{
		return(m_poApplication);
	}

#elif defined(WIN32)

	void SetCurrentDialog(HWND a_poDialog);

#endif /* WIN32 */

	void exit();
};

#endif /* ! STDAPPLICATION_H */
