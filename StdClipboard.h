
#ifndef STDCLIPBOARD_H
#define STDCLIPBOARD_H

/** @file */

#ifdef QT_GUI_LIB

#include <QtCore/QByteArray>

#endif /* QT_GUI_LIB */

/* Forward declarations to reduce the # of includes required */

class CWindow;
struct IFFHandle;

/**
 * A class for reading and writing clipboard data.
 * This class allows easy access to the underlying OS-specific clipboard implementation, in able to read data from
 * and write data to it.
 */

class RClipboard
{
private:

	const char	*m_pccGetData;			/**< Ptr to data being read */
	const char	*m_pccCurrentGetData;	/**< Ptr to current line of clipboard data being read */
	char		*m_pcSetData;			/**< Ptr to buffer containing data to be written */

#ifdef __amigaos__

	size_t		m_stDataSize;			/**< Size of buffer to be written, in bytes */
	IFFHandle	*m_poHandle;			/**< Handle to data to be transferred */

#elif defined(QT_GUI_LIB)

	size_t		m_stDataSize;			/**< Size of buffer to be written, in bytes */
	QByteArray	m_oGetData;				/**< Data returned by QClipboard::text().
											 The data is read and is pointed to by m_pccGetData.  This must
											 remain persistent and hence must be stored here */

#elif defined(WIN32)

	HANDLE		m_poHandle;				/**< Handle to data to be transferred */

#endif /* defined(WIN32) */

public:

	RClipboard()
	{

#ifdef __amigaos__

		m_poHandle = NULL;

#endif /* __amigaos__ */

		m_pccGetData = m_pccCurrentGetData = NULL;
		m_pcSetData = NULL;

#if defined(WIN32) && !defined(QT_GUI_LIB)

		m_poHandle = NULL;

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	}

	TInt open(CWindow *a_window);

	void close();

	int SetDataStart(size_t a_maxSize);

	void AppendData(const char *a_data, TInt a_offset, size_t a_size);

	void SetDataEnd();

	bool GetDataStart(TEncoding a_encoding);

	const char *GetNextLine(TInt *a_size, TBool *a_hasEOL);

	void GetDataEnd();
};

#endif /* ! STDCLIPBOARD_H */
