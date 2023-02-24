
#ifndef STDFILEREQUESTER_H
#define STDFILEREQUESTER_H

/** @file */

#define MAX_FILEREQUESTER_PATH 1024

/* A class to enable client code to display a file open or file save as requester in a platform */
/* independent manner */

class RFileRequester
{
private:

	char	*m_pcDirectoryName;						/**< Ptr to directory name extracted from fully qualified
														 filename passed into RFileRequester::GetFileName() */
	char	m_acFileName[MAX_FILEREQUESTER_PATH];	/**< Fully qualified filename of file selected by user */

public:

	RFileRequester()
	{
		m_pcDirectoryName = NULL;
		m_acFileName[0] = '\0';
	}

	void close();

	TInt GetFileName(const char *a_pccFileName, TBool a_bSaveAs);

	const char *FileName()
	{
		return(m_acFileName);
	}
};

#endif /* ! STDFILEREQUESTER_H */
