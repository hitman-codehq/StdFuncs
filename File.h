
#ifndef FILE_H
#define FILE_H

/** @file */

/**
 * Enumerations for use with RFile functions requiring file modes.
 * Enum to specify a range of bit values that can be masked together to specify the
 * mode in which to open a file, using the RFile class.
 */

enum TFileMode
{
	EFileRead = 0x00,	/**< Opens the file in read only mode */
	EFileWrite = 0x200,	/**< Opens the file in read/write mode */
	EFileExclusive = 0x400	/**< Opens the file in exclusive mode, preventing access by other */
							/**< processes or instances of the RFile class */
};

/**
 * A class for reading from or writing to files.
 * This class enables the creation, reading and writing of file in a platform independent manner.
 */

class RFile
{
private:

#ifdef __amigaos__

	TUint	m_uiFileMode;	/* Mode in which file was opened for access */
	BPTR	m_oHandle;		/* Handle to the open file or 0 if closed */

#elif defined(__unix__)

	TInt		m_oHandle;	/* Handle to the open file or 0 if closed */

#else /* ! __unix__ */

	HANDLE	m_oHandle;		/**< Handle to the open file or 0 if closed */

#endif /* ! __unix__ */

public:

	RFile();

	TInt Create(const char *a_pccFileName, TUint a_uiFileMode);

	TInt Replace(const char *a_pccFileName, TUint a_uiFileMode);

	TInt open(const char *a_pccFileName, TUint a_uiFileMode);

	TInt read(unsigned char *a_pucBuffer, TInt a_iLength) const;

	TInt seek(TInt a_iBytes);

	TInt write(const unsigned char *a_pcucBuffer, TInt a_iLength);

	void close();
};

#endif /* ! FILE_H */
