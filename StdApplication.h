
#ifndef STDAPPLICATION_H
#define STDAPPLICATION_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RApplication
{
private:

	CWindow		*m_poWindow;		/* Ptr to window used by the application */

#ifdef WIN32

	HACCEL		m_poAccelerators;	/* Ptr to application's accelerator table, if any */

#endif /* WIN32 */

public:

	RApplication();

	int Main();

	void AddWindow(CWindow *a_poWindow);

	void Exit();
};

#endif /* ! STDAPPLICATION_H */
