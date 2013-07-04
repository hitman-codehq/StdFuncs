
#ifndef STDTEXTFILE_H
#define STDTEXTFILE_H

/* This class will allow clients to read in a text file in its entirity and easily extract */
/* lines from that file, one at a time */

class RTextFile
{
private:

	char	*m_pcBuffer;	/* Ptr to buffer containing the text */
	char	*m_pcBufferPtr;	/* Ptr to current position in buffer */

public:

	RTextFile()
	{
		m_pcBuffer = NULL;
	}

	int Open(const char *a_pccFileName);

	void Close();

	char *GetLine();
};

#endif /* ! STDTEXTFILE_H */
