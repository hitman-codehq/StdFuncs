
#ifndef STDCLIPBOARD_H
#define STDCLIPBOARD_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* An abstraction class that allows easy access to the native machine's clipboard */
/* in able to read data from and write data to it */

class RClipboard
{
private:

	const char	*m_pccData;			/* Ptr to locked clipboard data, if any */
	const char	*m_pccCurrentData;	/* Ptr to current line of clipboard data, if any */

public:

	RClipboard()
	{
		m_pccData = m_pccCurrentData = NULL;
	}

	int Open(CWindow *a_poWindow);

	void Close();

	int InsertData(const char *a_pcData, int a_iLength);

	const char *LockData();

	const char *GetNextLine(TInt *a_piLength, TBool *a_bHasEOL);

	void UnlockData();
};

#endif /* ! STDCLIPBOARD_H */
