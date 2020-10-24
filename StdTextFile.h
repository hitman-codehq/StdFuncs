
#ifndef STDTEXTFILE_H
#define STDTEXTFILE_H

/* This class will allow clients to read in a text file in its entirity and easily extract */
/* lines from that file, one at a time */

class RTextFile
{
private:

	char	*m_pcBuffer;	/**< Pointer to buffer containing the text */
	char	*m_pcBufferEnd;	/**< Pointer to NULL terminator at end of text buffer */
	char	*m_pcBufferPtr;	/**< Pointer to current position in buffer */
	TInt	m_iSize;		/**< Size of the text file in bytes, excluding NULL terminator */

public:

	RTextFile()
	{
		m_pcBuffer = NULL;
	}

	TInt open(const char *a_pccFileName);

	void close();

	const char *GetLine();

	void Rewind();
};

#endif /* ! STDTEXTFILE_H */
