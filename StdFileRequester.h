
#ifndef STDFILEREQUESTER_H
#define STDFILEREQUESTER_H

#define MAX_FILEREQUESTER_PATH 1024

/* A class to enable client code to display a file open or file save as requester in a platform */
/* independent manner */

class RFileRequester
{
	char	m_acFileName[MAX_FILEREQUESTER_PATH];

public:

	RFileRequester()
	{
		m_acFileName[0] = '\0';
	}

	TInt GetFileName(TBool a_bOpen);

	const char *FileName()
	{
		return(m_acFileName);
	}
};

#endif /* ! STDFILEREQUESTER_H */
