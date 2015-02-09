
#include "StdFuncs.h"
#include "StdTextFile.h"

/**
 * Opens a text file and prepares it for reading.
 * This function will open a file and will parse through it in order to prepare it for later
 * reading, using the RTextFile::GetLine() function.
 *
 * @date	Friday 23-Jul-2010 8:00 am
 * @param	a_pccFileName	Name of the text file to be opened
 * @return	KErrNone if the file was loaded and parsed successfully
 * @return	Otherwise any of the errors returned by Utils::LoadFile()
 */

TInt RTextFile::Open(const char *a_pccFileName)
{
	TInt RetVal;

	/* Load the entire file into memory */

	if ((RetVal = m_iSize = Utils::LoadFile(a_pccFileName, (unsigned char **) &m_pcBuffer)) >= 0)
	{
		/* Indicate success */

		RetVal = KErrNone;

		/* Now iterate through the file until EOF, CR or LF is found and break the file into lines */
		/* thate are separated by NULL terminators */

		m_pcBufferPtr = m_pcBuffer;
		m_pcBufferEnd = (m_pcBuffer + m_iSize);

		while (m_pcBufferPtr < m_pcBufferEnd)
		{
			while ((*m_pcBufferPtr != 0x0d) && (*m_pcBufferPtr != 0x0a))
			{
				++m_pcBufferPtr;
			}

			/* NULL terminate the line and point the current line pointer to the next line */

			*m_pcBufferPtr = '\0';
			++m_pcBufferPtr;

			if (*m_pcBufferPtr == 0x0a)
			{
				++m_pcBufferPtr;
			}
		}

		m_pcBufferPtr = m_pcBuffer;
	}
	else
	{
		Utils::Info("RTextFile::Open() => Unable to read in file \"%s\"\n", a_pccFileName);
	}

	return(RetVal);
}

/**
 * Closes the file and free any resources associated with it.
 * This function should be called when the user has finished with the class instance.  It will free all
 * resources associated with the class instance and the instance should no longer be used after this call.
 *
 * @date	Friday 23-Jul-2010 8:02 am
 */

void RTextFile::Close()
{
	delete [] m_pcBuffer;
	m_pcBuffer = m_pcBufferPtr = NULL;
}

/**
 * Gets the next line in the file.
 * Finds the next available line in the file, NULL terminates it and returns a pointer to it.
 *
 * @date	Friday 23-Jul-2010 8:05 am
 * @return	A pointer to the start of the next available line else NULL if there are no more lines
 */

const char *RTextFile::GetLine()
{
	char *RetVal;

	/* Ensure that we are not already at the EOF */

	if (m_pcBufferPtr < m_pcBufferEnd)
	{
		/* Return a pointer to the current line */

		RetVal = m_pcBufferPtr;

		/* Now iterate through the current line until EOF, CR or LF is found.  It is guaranteed that */
		/* the file is NULL terminated, even if it is a zero byte file, as Utils::LoadFile() will do */
		/* this for us */

		while (*m_pcBufferPtr != '\0')
		{
			++m_pcBufferPtr;
		}

		/* Skip over the previous NULL terminator and LF (if present) at the EOL */

		if ((m_pcBufferPtr < m_pcBufferEnd) && (*m_pcBufferPtr == '\0'))
		{
			++m_pcBufferPtr;
		}

		if ((m_pcBufferPtr < m_pcBufferEnd) && (*m_pcBufferPtr == '\n'))
		{
			++m_pcBufferPtr;
		}
	}
	else
	{
		RetVal = NULL;
	}

	return(RetVal);
}

/**
 * Rewinds the internal line pointer to the start of the file.
 * This function is useful when a file has been partially or completely read and the user wishes to reread
 * it from the beginning.  Calling this function will reset the internal line pointer to the start of the
 * file and will put the class instance back into the state it was in immediately after RTextFile::Open()
 * was called.
 *
 * @date	Wednesday 24-Dec-2014 11:55 am, 325 On George
 */

void RTextFile::Rewind()
{
	m_pcBufferPtr = m_pcBuffer;
}
