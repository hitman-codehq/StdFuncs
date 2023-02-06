
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
 * A class for reading from or writing to local files.
 * This class enables the local creation, reading and writing of file in a platform independent manner.
 */

class RFile
{
private:

#ifdef __amigaos__

	TUint	m_fileMode;		/* Mode in which file was opened for access */
	BPTR	m_handle;		/* Handle to the open file or 0 if closed */

#elif defined(__unix__)

	int		m_handle;		/* Handle to the open file or 0 if closed */

#else /* ! __unix__ */

	HANDLE	m_handle;		/**< Handle to the open file or 0 if closed */

#endif /* ! __unix__ */

public:

	RFile();

	int create(const char *a_fileName, TUint a_fileMode);

	int replace(const char *a_fileName, TUint a_fileMode);

	int open(const char *a_fileName, TUint a_fileMode);

	int read(unsigned char *a_buffer, int a_length);

	int seek(int a_bytes);

	int write(const unsigned char *a_buffer, int a_length);

	void close();
};

#endif /* ! FILE_H */
