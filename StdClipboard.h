
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

	const char	*m_pccGetData;			/**< Ptr to data being read */
	const char	*m_pccCurrentGetData;	/**< Ptr to current line of clipboard data being read */
	char		*m_pcSetData;			/**< Ptr to buffer containing data to be written */

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	size_t		m_stDataSize;			/**< Size of buffer to be written */

#elif defined(WIN32)

	HANDLE		m_poHandle;				/**< Handle to data to be written */

#endif /* defined(WIN32) */

#ifdef QT_GUI_LIB

	QByteArray	m_oGetData;				/**< Data returned by QClipboard::text().
											 The data is read and is pointed to by m_pccGetData.  This must
											 remain persistent and hence must be stored here */

#endif /* QT_GUI_LIB */

public:

	RClipboard()
	{
		m_pccGetData = m_pccCurrentGetData = NULL;
		m_pcSetData = NULL;

#if defined(WIN32) && !defined(QT_GUI_LIB)

		m_poHandle = NULL;

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	}

	TInt open(CWindow *a_poWindow);

	void close();

	int SetDataStart(size_t a_stMaxLength);

	void AppendData(const char *a_pcData, TInt a_iOffset, size_t a_stLength);

	void SetDataEnd();

	const char *GetDataStart();

	const char *GetNextLine(TInt *a_piLength, TBool *a_bHasEOL);

	void GetDataEnd();
};

#endif /* ! STDCLIPBOARD_H */
