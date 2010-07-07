
#ifndef STDCLIPBOARD_H
#define STDCLIPBOARD_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* An abstraction class that allows easy access to the native machine's clipboard */
/* in able to read data from and write data to it */

class RClipboard
{
private:

	char	*m_pcData;		/* Ptr to locked clipboard data, if any */

public:

	RClipboard()
	{
		m_pcData = NULL;
	}

	int Open(CWindow *a_poWindow);

	void Close();

	const char *LockData();

	void UnlockData();
};

#endif /* ! STDCLIPBOARD_H */
