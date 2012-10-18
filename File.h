
#ifndef FILE_H
#define FILE_H

/* Enum to specify a range of bit values that can be masked together to specify the */
/* mode in which to open a file, using the RFile class */

enum TFileMode
{
	EFileRead = 0x00,
	EFileWrite = 0x200
};

/* A class for reading from or writing to a file in a platform independent manner */

class RFile
{

#ifdef __amigaos4__

	TUint	m_uiFileMode;	/* Mode in which file was opened for access */
	BPTR	m_oHandle;		/* Handle to the open file or 0 if closed */

#elif defined(__linux__)

	int		m_oHandle;		/* Handle to the open file or 0 if closed */

#else /* ! __linux__ */

	HANDLE	m_oHandle;		/* Handle to the open file or 0 if closed */

#endif /* ! __linux__ */

public:

	RFile();

	TInt Create(const char *a_pccFileName, TUint a_uiFileMode);

	TInt Replace(const char *a_pccFileName, TUint a_uiFileMode);

	TInt Open(const char *a_pccFileName, TUint a_uiFileMode);

	TInt Read(unsigned char *a_pucBuffer, TInt a_iLength) const;

	TInt Write(const unsigned char *a_pcucBuffer, TInt a_iLength);

	void Close();
};

#endif /* ! FILE_H */
