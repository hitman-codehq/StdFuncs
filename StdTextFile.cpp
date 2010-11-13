
#include "StdFuncs.h"
#include "StdTextFile.h"

/* Written: Friday 23-Jul-2010 8:00 am */

int RTextFile::Open(const char *a_pccFileName)
{
	int RetVal;

	/* Load the entire file into memory */

	if ((RetVal = Utils::LoadFile(a_pccFileName, (unsigned char **) &m_pcBuffer)) == KErrNone)
	{
		m_pcBufferPtr = m_pcBuffer;
	}
	else
	{
		Utils::Info("RTextFile::Open() => Unable to read in file \"%s\"\n", a_pccFileName);
	}

	return(RetVal);
}

/* Written: Friday 23-Jul-2010 8:02 am */

void RTextFile::Close()
{
	delete [] m_pcBuffer;
	m_pcBuffer = m_pcBufferPtr = NULL;
}

/* Written: Friday 23-Jul-2010 8:05 am */
/* Finds the next available line in the file, NULL terminates it and returns a ptr to it */
/* @returns	A ptr to the start of the next available line else NULL if there are no more lines */

char *RTextFile::GetLine()
{
	char *RetVal;

	/* Get a ptr to the start of the current line, which is what will be returned */

	RetVal = m_pcBufferPtr;

	/* Now iterate through the current line until EOF, CR or LF is found */

	while ((*m_pcBufferPtr != '\0') && (*m_pcBufferPtr != 0x0d) && (*m_pcBufferPtr != 0x0a))
	{
		++m_pcBufferPtr;
	}

	/* If any valid characters were found then NULL terminate the current line and move the */
	/* current line ptr to the next line */

	if (m_pcBufferPtr > RetVal)
	{
		if (*m_pcBufferPtr != '\0')
		{
			/* NULL terminate the line and point the current line ptr to the next line */

			*m_pcBufferPtr = '\0';
			++m_pcBufferPtr;

			if (*m_pcBufferPtr == 0x0a)
			{
				++m_pcBufferPtr;
			}
		}
	}

	/* Otherwise indicate that we have reached the EOF */

	else
	{
		RetVal = NULL;
	}

	return(RetVal);
}
