
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

	BPTR	m_oHandle;		/* Handle to the open file or 0 if closed */

#else /* ! __amigaos4__ */

	HANDLE	m_oHandle;		// TODO: CAW

#endif /* ! __amigaos4__ */

public:

	RFile();

	int Create(const char *a_pccName, unsigned int a_uiFileMode);

	int Replace(const char *a_pccName, unsigned int a_uiFileMode);

	int Open(const char *a_pccName, unsigned int a_uiFileMode);

	int Read(unsigned char *a_pucBuffer, int a_iLength) const;

	int Write(const unsigned char *a_pcucBuffer, int a_iLength);

	void Close();
};

#endif /* ! FILE_H */
