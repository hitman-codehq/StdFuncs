
#ifndef STDAPPLICATION_H
#define STDAPPLICATION_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

class RApplication
{
private:

	CWindow		*m_poWindow;	/* Ptr to window used by the application */

public:

	int Main();

	void AddWindow(CWindow *a_poWindow);
};

#endif /* ! STDAPPLICATION_H */
