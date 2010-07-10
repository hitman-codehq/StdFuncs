
#ifndef STDCLIPBOARD_H
#define STDCLIPBOARD_H

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* An abstraction class that allows easy access to the native machine's clipboard */
/* in able to read data from and write data to it */

class RClipboard
{
private:

	const char	*m_pccGetData;			/* Ptr to data being read */
	const char	*m_pccCurrentGetData;	/* Ptr to current line of clipboard data being read */
	char		*m_pcSetData;			/* Ptr to buffer containing data to be written */
	HANDLE		m_poHandle;				/* Handle to data to be written */

public:

	RClipboard()
	{
		m_pccGetData = m_pccCurrentGetData = NULL;
		m_pcSetData = NULL;
		m_poHandle = NULL;
	}

	int Open(CWindow *a_poWindow);

	void Close();

	int SetDataStart(int a_iMaxLength);

	void AppendData(const char *a_pcData, int a_iOffset, int a_iLength);

	void SetDataEnd();

	const char *GetDataStart();

	const char *GetNextLine(TInt *a_piLength, TBool *a_bHasEOL);

	void GetDataEnd();
};

#endif /* ! STDCLIPBOARD_H */
