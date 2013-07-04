
#ifndef STDCLIPBOARD_H
#define STDCLIPBOARD_H

#ifdef QT_GUI_LIB

#include <QtCore/QByteArray>

#endif /* QT_GUI_LIB */

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

#if defined(__amigaos4__) || defined(QT_GUI_LIB)

	TInt		m_iDataSize;			/* Size of buffer to be written */

#else /* ! defined(__amigaos4__) || defined(QT_GUI_LIB) */

	HANDLE		m_poHandle;				/* Handle to data to be written */

#endif /* ! defined(__amigaos4__) || defined(QT_GUI_LIB) */

#ifdef QT_GUI_LIB

	QByteArray	m_oGetData;				/* Data returned by QClipboard::text() and pointed to */
										/* by m_pccGetData.  This must remain persistent and hence */
										/* must be stored here */

#endif /* QT_GUI_LIB */

public:

	RClipboard()
	{
		m_pccGetData = m_pccCurrentGetData = NULL;
		m_pcSetData = NULL;

#if !defined(__amigaos4__) && !defined(QT_GUI_LIB)

		m_poHandle = NULL;

#endif /* !defined(__amigaos4__) && !defined(QT_GUI_LIB) */

	}

	TInt Open(CWindow *a_poWindow);

	void Close();

	int SetDataStart(int a_iMaxLength);

	void AppendData(const char *a_pcData, int a_iOffset, int a_iLength);

	void SetDataEnd();

	const char *GetDataStart();

	const char *GetNextLine(TInt *a_piLength, TBool *a_bHasEOL);

	void GetDataEnd();
};

#endif /* ! STDCLIPBOARD_H */
